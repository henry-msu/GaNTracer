/*
 * common.c
 *
 * Useful functions and variables for this project
 *
 *  Created on: Mar 27, 2024
 *      Author: henry
 */

#include "common.h"

// Finally, "reliable" and "easy" data transmission
//   dev: base address of the module you want to use, something like "EUSCI_B1_BASE"
//   data_ptr: address of transmit data for chosen module, used in ISR
//   data: array of data to send
//   n: number of bytes to send
void I2CMTXBytes(uint16_t dev, uint8_t** data_ptr, uint8_t* data, uint16_t n) {
    *data_ptr = data;
    while (EUSCI_B_I2C_SENDING_STOP == EUSCI_B_I2C_masterIsStopSent(dev));
    HWREG16(dev + OFS_UCBxTBCNT) = n; // datasheet says to not modify this while module is active, but whatever
    EUSCI_B_I2C_masterSendMultiByteStart(dev, *(*data_ptr)++); // begin sending data, ISR takes care of the rest
    __bis_SR_register(CPUOFF + GIE); // enter LPM0 and wait
}

void I2CMRXBytes(uint16_t dev, uint8_t** data_ptr, uint8_t* data, uint16_t n) {
    *data_ptr = data;
    while (EUSCI_B_I2C_SENDING_STOP == EUSCI_B_I2C_masterIsStopSent(dev));
    HWREG16(dev + OFS_UCBxTBCNT) = n; // datasheet says to not modify this while module is active, but whatever
    EUSCI_B_I2C_masterReceiveStart(dev); // begin receiving data, ISR takes care of the rest
    __bis_SR_register(CPUOFF + GIE); // enter LPM0 and wait
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
