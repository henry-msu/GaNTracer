/*
 * Reading from temperature sensor
 * Henry Naguski
*/
#include <driverlib.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define CS_MCLK_DESIRED_FREQUENCY_IN_KHZ 20000 // Target frequency for MCLK in kHz
#define CS_MCLK_FLLREF_RATIO CS_MCLK_DESIRED_FREQUENCY_IN_KHZ/32.768 // MCLK/FLLRef Ratio

#define TEMPSENSOR_ADDRESS 0x4A // slave address of TEMPSENSOR
#define TEMPSENSOR_I2C EUSCI_B1_BASE // base address of eusci module being used for TEMPSENSOR

uint8_t* EUSCI_B1_TXDATA;
uint8_t* EUSCI_B1_RXDATA;

void I2CMTXBytes(uint16_t dev, uint8_t** data_ptr, uint8_t* data, uint16_t n);
void I2CMRXBytes(uint16_t dev, uint8_t** data_ptr, uint8_t* data, uint16_t n);

void init(void);

// Finally, "reliable" and "easy" data transmission
//   dev: base address of the module you want to use, something like "EUSCI_B1_BASE"
//   txcnt_ptr: address of transmit counter for chosen module, used in ISR to tell when transmission is finished
//   data_ptr: address of transmit data for chosen module, used in ISR
//   data: array of data to send
//   n: number of bytes to send
void I2CMTXBytes(uint16_t dev, uint8_t** data_ptr, uint8_t* data, uint16_t n) {
    *data_ptr = data;
    while (EUSCI_B_I2C_SENDING_STOP == EUSCI_B_I2C_masterIsStopSent(dev));
    HWREG16(dev + OFS_UCBxTBCNT) = n; // datasheet says to not modify this while module is active, but whatever
    ROM_EUSCI_B_I2C_masterSendMultiByteStart(dev, *(*data_ptr)++); // begin sending data, ISR takes care of the rest
    __bis_SR_register(CPUOFF + GIE); // enter LPM0 and wait
}

void I2CMRXBytes(uint16_t dev, uint8_t** data_ptr, uint8_t* data, uint16_t n) {
    *data_ptr = data;
    while (EUSCI_B_I2C_SENDING_STOP == EUSCI_B_I2C_masterIsStopSent(dev));
    HWREG16(dev + OFS_UCBxTBCNT) = n; // datasheet says to not modify this while module is active, but whatever
    EUSCI_B_I2C_masterReceiveStart(dev); // begin receiving data, ISR takes care of the rest
    __bis_SR_register(CPUOFF + GIE); // enter LPM0 and wait
    //HWREG16(dev + OFS_UCBxCTLW0) &= ~UCTXSTP; // gross workaround, for some reason MSP is not automatically clearing UCTXSTP like it should
}

// These two functions kinda suck, but they seem to work for 115200 baud...
// TODO: replace with driverlib, or don't since it's not necessary for final design
void UARTtxByte(uint8_t data) {
    while (UCA0IFG & UCTXIFG);
    UCA0TXBUF = data; // send byte
    do {
        LPM0; // enter low power mode and wait for byte to finish sending
    } while (UCA0IFG & UCTXIFG);
}

// UARTtxBytes: Transmit n bytes over UART
void UARTtxBytes(uint8_t* data, uint32_t n) {
    uint32_t i;
    while (UCA0IFG & UCTXIFG);
    for (i = 0; i < n; i++) {
        UCA0TXBUF = data[i];
        do {
            LPM0; // enter low power mode and wait for byte to finish sending
        } while (UCA0IFG & UCTXIFG);
    }
}


void init(void) {

    // If running at more than 8 MHz, need FRAM wait states
    FRAMCtl_configureWaitStateControl(FRAMCTL_ACCESS_TIME_CYCLES_2);

    //Set DCO FLL reference = REFO
    CS_initClockSignal(CS_FLLREF, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    //Set ACLK = REFO
    CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    CS_initFLLParam csparam = {0}; // Create struct variable to store proper software trim values
    // Set Ratio/Desired MCLK Frequency, initialize DCO, save trim values
    CS_initFLLCalculateTrim(CS_MCLK_DESIRED_FREQUENCY_IN_KHZ, CS_MCLK_FLLREF_RATIO, &csparam);
    CS_clearAllOscFlagsWithTimeout(1000); // Clear all OSC fault flags

    // Configure Pins for I2C
    // using eUSCI_B1, which is P4.6 and P4.7
    GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_P4,
        GPIO_PIN6 + GPIO_PIN7,
        GPIO_PRIMARY_MODULE_FUNCTION
    );

    // Configure I2C module
    EUSCI_B_I2C_initMasterParam I2Cparam = {
        .selectClockSource = EUSCI_B_I2C_CLOCKSOURCE_SMCLK,
        .i2cClk = CS_getSMCLK(),
        .dataRate = EUSCI_B_I2C_SET_DATA_RATE_400KBPS,
        .byteCounterThreshold = 2,
        .autoSTOPGeneration = EUSCI_B_I2C_SEND_STOP_AUTOMATICALLY_ON_BYTECOUNT_THRESHOLD
    };
    EUSCI_B_I2C_initMaster(TEMPSENSOR_I2C, &I2Cparam);

    EUSCI_B_I2C_setSlaveAddress(TEMPSENSOR_I2C, TEMPSENSOR_ADDRESS);

    EUSCI_B_I2C_enable(TEMPSENSOR_I2C);

    EUSCI_B_I2C_clearInterrupt(TEMPSENSOR_I2C,
        EUSCI_B_I2C_TRANSMIT_INTERRUPT0 +
        EUSCI_B_I2C_RECEIVE_INTERRUPT0 +
		EUSCI_B_I2C_BYTE_COUNTER_INTERRUPT
    );

    EUSCI_B_I2C_enableInterrupt(TEMPSENSOR_I2C,
        EUSCI_B_I2C_TRANSMIT_INTERRUPT0 +
        EUSCI_B_I2C_RECEIVE_INTERRUPT0 +
		EUSCI_B_I2C_BYTE_COUNTER_INTERRUPT
    );

     // Set up UART on eUSCI_A0
     // TODO: replace with driverlib
     // P1.6: UCA0RXD
     // P1.7: UCA0TXD
     // BRCLK = SMCLK = 1 MHz
     // 115200 baud, currently my functions break at lower baud rates
     static const uint8_t UCOS = 1;
     static const uint16_t BRx = 10;
     static const uint8_t BRFx = 13;
     static const uint8_t BRSx = 0xAD;

     UCA0CTLW0 |= UCSWRST;
     UCA0CTLW0 &= ~(UCSSEL | UCSYNC | UCMODE); // clear clock source, put in UART mode
     UCA0CTLW0 |= UCSSEL__SMCLK; // BRCLK = SMCLK
     UCA0BRW = BRx;
     UCA0MCTLW = (BRSx << 8) | (BRFx << 4) | UCOS;

     UCA0CTLW0 &= ~UCSWRST; // enable module
     UCA0IFG &= ~UCTXIFG;
     UCA0IE |= UCTXIE; // enable TX interrupt

     // Change P1.6 and P1.7 to UART mode
     P1SEL0 |= BIT6 | BIT7;
     P1SEL1 &= ~(BIT6 | BIT7);


    /*
     * Disable the GPIO power-on default high-impedance mode to activate
     * previously configured port settings
     */
    PMM_unlockLPM5();
}

void main (void) {
    WDT_A_hold(WDT_A_BASE);
    init();

    uint8_t resetCMD[2] = {0x30, 0xA2};
    uint8_t OSCS_CMD[2] = {0x2C, 0x06}; // oneshot reading with clock stretching enabled
    uint8_t heaterOff[2] = {0x30, 0x66};

    uint16_t RXnum = 2; // how many bytes to receive
    uint8_t* RXdata = malloc(RXnum * sizeof(uint8_t));
    uint8_t tempArr[4] = {0};

    // Reset temperature sensor
    I2CMTXBytes(TEMPSENSOR_I2C, &EUSCI_B1_TXDATA, resetCMD, 2);

    while(1) {
        __delay_cycles(10000);
        I2CMTXBytes(TEMPSENSOR_I2C, &EUSCI_B1_TXDATA, OSCS_CMD, 2);
        // read temperature and ignore checksum
        I2CMRXBytes(TEMPSENSOR_I2C, &EUSCI_B1_RXDATA, RXdata, RXnum);

        if(RXdata[1] == 0) {
        	__no_operation();
        }
        if(RXdata[0] == 0) {
        	__no_operation();
        }

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
