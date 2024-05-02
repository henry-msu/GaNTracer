/*
 * Reading from temperature sensor
 * Henry Naguski
*/
#include <driverlib.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "common.h"

uint8_t* EUSCI_B1_TXDATA;
uint8_t* EUSCI_B1_RXDATA;

#define tempDataSize 4096*2 // need two bytes for each temperature reading
#pragma PERSISTENT(tempData)
uint8_t tempData[tempDataSize] = {0};
uint16_t tempDataInd = 0;

uint8_t resetCMD[2] = {0x30, 0xA2};
uint8_t OSCS_HIGH_CMD[2] = {0x2C, 0x06}; // oneshot reading with clock stretching enabled, high repeatability
uint8_t OSCS_MED_CMD[2] = {0x2C, 0x0D}; // oneshot reading with clock stretching enabled, medium repeatability
uint8_t OSCS_LOW_CMD[2] = {0x2C, 0x10}; // oneshot reading with clock stretching enabled, low repeatability

uint8_t heaterOff[2] = {0x30, 0x66};

uint16_t RXnum = 2; // how many bytes to receive
uint8_t tempArr[4] = {0};

// SPI variables
uint8_t executing = 0; // whether or not we are doing something
uint8_t instruction = 0; // what instruction to execute
uint8_t tempReadNumCtr = 0; // for filling tempReadNum since it will be 2 bytes
uint16_t tempReadNum = 0; // how many temperature measurements to make * 2 since each reading is 2 bytes

void main (void) {
    init();

    uint8_t* RXdata = malloc(RXnum * sizeof(uint8_t));

    // Reset temperature sensor
    __delay_cycles(1000);
    I2CMTXBytes(TEMPSENSOR_I2C, &EUSCI_B1_TXDATA, resetCMD, 2);
    I2CMTXBytes(TEMPSENSOR_I2C, &EUSCI_B1_TXDATA, heaterOff, 2);

    while(1) {
    	LPM0; // enter LPM0 and wait until we have to do something

    	if(instruction == 0x0A) { // read temperature and notify host when finished
    		tempDataInd = 0; // reset temperature data index
    		while(tempDataInd < tempReadNum) {
    			delayms(1); // Let temp sensor cool down a bit between each reading, this is lame
				// tell temp sensor to make a measurement
				I2CMTXBytes(TEMPSENSOR_I2C, &EUSCI_B1_TXDATA, OSCS_HIGH_CMD, 2);
				// read temperature result (ignore checksum)
				I2CMRXBytes(TEMPSENSOR_I2C, &EUSCI_B1_RXDATA, RXdata, RXnum);
				__delay_cycles(500); // TODO: fix this, issue is the function returns too early, caused by either I2CMRXBytes or the ISR (or both)

				tempData[tempDataInd++] = RXdata[0];
				tempData[tempDataInd++] = RXdata[1];

    		}
    		tempDataInd = 0;
			EUSCI_A_SPI_clearInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);
			EUSCI_A_SPI_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);
			//delayms(500); // give a moment for sensor to cool down??? Without this the temperature creeps up slowly

    		GPIO_setOutputHighOnPin(GPIO_PORT_P2, PIN_FINISHED_READING_TEMP); // notify host that we're done reading, this will immediately begin data reception
			EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, tempData[tempDataInd++]); // place first byte into txbuf
    	}
    }
}

//******************************************************************************
// USCI_A0 interrupt vector service routine.
//******************************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(USCI_A0_VECTOR)))
#endif
void USCI_A0_ISR (void) {
	switch(__even_in_range(UCA0IV, USCI_SPI_UCTXIFG)) {
		case USCI_SPI_UCRXIFG: // UCRXIFG

			if(!executing) { // if not currently executing something, read in an instruction
				instruction = EUSCI_A_SPI_receiveData(EUSCI_A0_BASE);
				executing = 1; // now we're doing something
			}

			else if(instruction == 0x0A) { // trigger a measurement
				// figure out how many bits to read
				if (tempReadNumCtr++ == 0) {
					tempReadNum = EUSCI_A_SPI_receiveData(EUSCI_A0_BASE) << 8;
					break;
				}
				tempReadNum += EUSCI_A_SPI_receiveData(EUSCI_A0_BASE);
				tempReadNumCtr = 0;
				// disable receive interrupt while doing temperature readings, ignoring further commands
				EUSCI_A_SPI_disableInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_RECEIVE_INTERRUPT);
				LPM0_EXIT;
			}
			break;
		case USCI_SPI_UCTXIFG: // UCTXIFG
			if(instruction == 0x0A) {
				EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, tempData[tempDataInd++]);

				if(tempDataInd > tempReadNum) {
					// reset for next instruction
					tempDataInd = 0;
					tempReadNum = 0;
					instruction = 0;
					executing = 0;
					GPIO_setOutputLowOnPin(GPIO_PORT_P2, PIN_FINISHED_READING_TEMP);
					EUSCI_A_SPI_disableInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT);
					EUSCI_A_SPI_clearInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_RECEIVE_INTERRUPT);
					EUSCI_A_SPI_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_RECEIVE_INTERRUPT);
				}
			}
			break;
		default:
			break;
	}
}

//------------------------------------------------------------------------------
// The USCIAB1TX_ISR is structured such that it can be used to transmit any
// number of bytes by pre-loading TXByteCtr with the byte count. Also, TXData
// points to the next byte to transmit.
//------------------------------------------------------------------------------
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_B1_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(USCI_B1_VECTOR)))
#endif
void USCIB1_ISR(void) {
    switch(__even_in_range(UCB1IV, USCI_I2C_UCBIT9IFG)) {
        case USCI_NONE:             // No interrupts
            break;
        case USCI_I2C_UCALIFG:      // Arbitration lost
            break;
        case USCI_I2C_UCSTPIFG:
        	break;
        case USCI_I2C_UCNACKIFG:    // NACK received (master only)
            // resend start if NACK
            if (EUSCI_B_I2C_getMode(EUSCI_B1_BASE) == EUSCI_B_I2C_TRANSMIT_MODE) {
                EUSCI_B_I2C_masterSendStart(EUSCI_B1_BASE);
            }
            else {
                EUSCI_B_I2C_masterReceiveStart(EUSCI_B1_BASE);
            }
            break;

        case USCI_I2C_UCTXIFG0: // TXIFG0
            EUSCI_B_I2C_masterSendMultiByteNext(EUSCI_B1_BASE, *EUSCI_B1_TXDATA++);
            break;

        case USCI_I2C_UCRXIFG0: // RXIFG0, somehting in here is currently broken
      		*EUSCI_B1_RXDATA++ = EUSCI_B_I2C_masterReceiveSingle(EUSCI_B1_BASE);
            break;
        case USCI_I2C_UCBCNTIFG:
            __bic_SR_register_on_exit(CPUOFF); // Exit LPM0
            break;
        default:
            break;
    }
}

//------------------------------------------------------------------------------
// TB0 CCR0 Interrupt: to stop delayms
//------------------------------------------------------------------------------
#pragma vector = TIMER0_B0_VECTOR
__interrupt void TB0_CCR0(void){
    LPM0_EXIT;
    TB0CTL &= ~(MC__UPDOWN); // stop timer
    TB0CTL |= TBCLR;
    TB0CCTL0 &= ~(CCIE + CCIFG);
}

//------------------------------------------------------------------------------
// NMI vector service routine.
//------------------------------------------------------------------------------
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=UNMI_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(UNMI_VECTOR)))
#endif
void NMI_ISR(void) {
  do {
    // If it still can't clear the oscillator fault flags after the timeout,
    // trap and wait here.
    status = CS_clearAllOscFlagsWithTimeout(1000);
  } while(status != 0);
}
