/*
 * Reading from STS3x-DIS I2C temperature sensor
 * Henry Naguski
*/
#include "eusci_b_i2c.h"
#include <driverlib.h>
#include <stdlib.h>
#include <stdint.h>

#define TEMPSENSOR_ADDRESS 0x4A // slave address of temperature sensor
#define TEMPSENSOR_I2C EUSCI_B1_BASE // base address of eusci module being used for temperature sensor
#define RXCOUNT 2 // Will always read 2 bytes of data from sensor, as we don't care about the CRC

uint16_t EUSCI_B1_TXCNT = 0;
uint8_t* EUSCI_B1_TXDATA;

uint16_t EUSCI_B1_RXCNT = 0;
uint8_t* EUSCI_B1_RXDATA;

void I2CTXBytes(uint16_t dev, uint16_t* txcnt_ptr, uint8_t** data_ptr, uint8_t* data, uint16_t n);
void I2CRXBytes(uint16_t dev, uint16_t* rxcnt_ptr, uint8_t** data_ptr, uint8_t* data, uint16_t n);

void init(void);

// Finally, "reliable" and "easy" data transmission
//   dev: base address of the module you want to use, something like "EUSCI_B1_BASE"
//   txcnt_ptr: address of transmit counter for chosen module, used in ISR to tell when transmission is finished
//   data_ptr: address of transmit data for chosen module, used in ISR
//   data: array of data to send
//   n: number of bytes to send
void I2CTXBytes(uint16_t dev, uint16_t* txcnt_ptr, uint8_t** data_ptr, uint8_t* data, uint16_t n) {
    *txcnt_ptr = n - 1; // Fill proper txcnt register
    *data_ptr = data;
    while (EUSCI_B_I2C_SENDING_STOP == EUSCI_B_I2C_masterIsStopSent(dev));
    ROM_EUSCI_B_I2C_masterSendMultiByteStart(dev, *(*data_ptr)++); // begin sending data, ISR takes care of the rest
    __bis_SR_register(CPUOFF + GIE); // enter LPM0 and wait
}

void I2CRXBytes(uint16_t dev, uint16_t* rxcnt_ptr, uint8_t** data_ptr, uint8_t* data, uint16_t n) {
    *rxcnt_ptr = n - 1; // Fill proper rxcnt register
    *data_ptr = data;
    while (EUSCI_B_I2C_SENDING_STOP == EUSCI_B_I2C_masterIsStopSent(dev));
    EUSCI_B_I2C_masterReceiveStart(dev); // begin receiving data, ISR takes care of the rest
    __bis_SR_register(CPUOFF + GIE); // enter LPM0 and wait
}

void init(void) {
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
        .byteCounterThreshold = 0,
        .autoSTOPGeneration = EUSCI_B_I2C_NO_AUTO_STOP
    };
    EUSCI_B_I2C_initMaster(TEMPSENSOR_I2C, &I2Cparam);

    EUSCI_B_I2C_setSlaveAddress(TEMPSENSOR_I2C, TEMPSENSOR_ADDRESS);

    EUSCI_B_I2C_enable(TEMPSENSOR_I2C);

    EUSCI_B_I2C_clearInterrupt(TEMPSENSOR_I2C,
        EUSCI_B_I2C_TRANSMIT_INTERRUPT0 +
        EUSCI_B_I2C_RECEIVE_INTERRUPT0 +
        EUSCI_B_I2C_NAK_INTERRUPT
    );

    EUSCI_B_I2C_enableInterrupt(TEMPSENSOR_I2C,
        EUSCI_B_I2C_TRANSMIT_INTERRUPT0 +
        EUSCI_B_I2C_RECEIVE_INTERRUPT0 +
        EUSCI_B_I2C_NAK_INTERRUPT
    );

    /*
     * Disable the GPIO power-on default high-impedance mode to activate
     * previously configured port settings
     */
    PMM_unlockLPM5();
}

void main (void) {
    WDT_A_hold(WDT_A_BASE);
    init();

    uint16_t TXnum = 2; // how many bytes to send
    uint8_t TXdata[2] = {0x24, 0x00};
    uint8_t resetCMD[2] = {0x30, 0xA2};
    uint8_t heaterOff[2] = {0x30, 0x66};

    uint16_t RXnum = 2; // how many bytes to receive
    uint8_t* RXdata = malloc(RXnum * sizeof(uint8_t));

    // Reset temperature sensor
    I2CTXBytes(TEMPSENSOR_I2C, &EUSCI_B1_TXCNT, &EUSCI_B1_TXDATA, resetCMD, TXnum);
    I2CTXBytes(TEMPSENSOR_I2C, &EUSCI_B1_TXCNT, &EUSCI_B1_TXDATA, heaterOff, TXnum); // I think the heater is off by default, but can't be too sure

    while(1) {
        I2CTXBytes(TEMPSENSOR_I2C, &EUSCI_B1_TXCNT, &EUSCI_B1_TXDATA, TXdata, TXnum);
        __delay_cycles(100000);
        I2CRXBytes(TEMPSENSOR_I2C, &EUSCI_B1_RXCNT, &EUSCI_B1_RXDATA, RXdata, RXnum);
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
        case USCI_I2C_UCNACKIFG:    // NACK received (master only)
            // resend start if NACK
            if (EUSCI_B_I2C_getMode(EUSCI_B1_BASE) == EUSCI_B_I2C_TRANSMIT_MODE) {
                EUSCI_B_I2C_masterSendStart(EUSCI_B1_BASE);
            }
            else {
                EUSCI_B_I2C_masterReceiveStart(EUSCI_B1_BASE);
            }
            break;

        case USCI_I2C_UCTXIFG0:     // TXIFG0
            // Check TX byte counter
            if (EUSCI_B1_TXCNT) {
                EUSCI_B_I2C_masterSendMultiByteNext(EUSCI_B1_BASE, *EUSCI_B1_TXDATA++);
                // Decrement TX byte counter
                EUSCI_B1_TXCNT--;
            }
            else {
                EUSCI_B_I2C_masterSendMultiByteStop(EUSCI_B1_BASE);
                __bic_SR_register_on_exit(CPUOFF); // Exit LPM0
            }
            break;

        case USCI_I2C_UCRXIFG0:     // RXIFG0, somehting in here is currently broken
		    if (EUSCI_B1_RXCNT) {
                *EUSCI_B1_RXDATA++ = EUSCI_B_I2C_masterReceiveMultiByteNext(EUSCI_B1_BASE);
                EUSCI_B1_RXCNT--;
            }
            else {
                EUSCI_B_I2C_masterReceiveMultiByteStop(EUSCI_B1_BASE);
	            __bic_SR_register_on_exit(CPUOFF); // Exit LPM0
		    }
            break;
        default:
            break;
    }
}
