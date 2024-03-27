/*
 * Reading from temperature sensor, filling up a buffer, and sending it back to host device via spi
 * Henry Naguski
*/
#include <driverlib.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "common.h"


uint8_t* EUSCI_B1_TXDATA;
uint8_t* EUSCI_B1_RXDATA;

const uint8_t resetCMD[2] = {0x30, 0xA2};
const uint8_t OSCS_CMD[2] = {0x2C, 0x06}; // oneshot reading with clock stretching enabled
const uint8_t heaterOff[2] = {0x30, 0x66};

#define tempDataSize 1024 // how big temperature array is
uint16_t tempData[tempDataSize] = {0}; // Array to store temperature readings
uint16_t tempInd = 0; // index for tempData

void main (void) {
    WDT_A_hold(WDT_A_BASE);
    init();

    uint16_t RXnum = 2; // how many bytes to receive
    uint8_t* RXdata = malloc(RXnum * sizeof(uint8_t));
    uint8_t tempArr[4] = {0}; // array to store temperature as a string

    // Reset temperature sensor
    I2CMTXBytes(TEMPSENSOR_I2C, &EUSCI_B1_TXDATA, resetCMD, 2);

    while(1) {
        __delay_cycles(10000);
        I2CMTXBytes(TEMPSENSOR_I2C, &EUSCI_B1_TXDATA, OSCS_CMD, 2);
        // read temperature and ignore checksum (checksum is 3rd byte, only reading 2)
        I2CMRXBytes(TEMPSENSOR_I2C, &EUSCI_B1_RXDATA, RXdata, RXnum);
        __delay_cycles(10000);

        // store data in array
        if (tempInd >= tempDataSize) {
        	tempInd = 0;
        }
        tempData[tempInd++] = (uint16_t) ((RXdata[0] << 8) + RXdata[1]);

        // send result over UART
        tempArr[0] = RXdata[0];
        tempArr[1] = RXdata[1];
        tempArr[2] = '\r';
        tempArr[3] = '\n';
        UARTtxBytes(tempArr, 4);

        __no_operation();
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

// EUSCI_A0_ISR: Exits low power for UART
#pragma vector = EUSCI_A0_VECTOR
__interrupt void EUSCI_A0_ISR(void) {
    switch (__even_in_range(UCA0IV, UCIV__UCTXCPTIFG)) { // determine what interrupt was triggered
    case UCIV__UCTXIFG:
        LPM0_EXIT;
        break;
    default:
        break;
    }
} // end EUSCI_A0_ISR
