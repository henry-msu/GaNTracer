#include "driverlib.h"

uint8_t transmitData = 0;
uint8_t receiveData = 0;

void initSPI() {
    // Stop watchdog timer
    WDT_A_hold(WDT_A_BASE);

    // Configure SPI pins
    // Configure Pins for UCA0CLK, UCA0TXD/UCA0SIMO, UCA0RXD/UCA0SOMI, and UCA0STE
    GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_P1,
        GPIO_PIN1 + GPIO_PIN2 + GPIO_PIN3 + GPIO_PIN4,
        GPIO_PRIMARY_MODULE_FUNCTION
    );

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PMM_unlockLPM5();

    // Initialize slave to MSB first, inactive low clock polarity, and 4-wire SPI
    EUSCI_A_SPI_initSlaveParam param = {0};
    param.msbFirst = EUSCI_A_SPI_MSB_FIRST;
    param.clockPhase = EUSCI_A_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT;
    param.clockPolarity = EUSCI_A_SPI_CLOCKPOLARITY_INACTIVITY_LOW;
    param.spiMode = EUSCI_A_SPI_4PIN_UCxSTE_ACTIVE_HIGH;
    EUSCI_A_SPI_initSlave(EUSCI_A0_BASE, &param);

    // Enable 4-pin functionality
    EUSCI_A_SPI_select4PinFunctionality(EUSCI_A0_BASE, EUSCI_A_SPI_ENABLE_SIGNAL_FOR_4WIRE_SLAVE);

    EUSCI_A_SPI_enable(EUSCI_A0_BASE); // Enable SPI Module

    EUSCI_A_SPI_clearInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_RECEIVE_INTERRUPT);
    EUSCI_A_SPI_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_SPI_RECEIVE_INTERRUPT);
}

void writeSPI(uint8_t data) {
    // Wait until the transmit buffer is ready
    while (!EUSCI_A_SPI_getInterruptStatus(EUSCI_A0_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT));

    // Send data to master
    EUSCI_A_SPI_transmitData(EUSCI_A0_BASE, data);
}

uint8_t readSPI() {
    // Wait until the receive buffer is full
    while (!EUSCI_A_SPI_getInterruptStatus(EUSCI_A0_BASE, EUSCI_A_SPI_RECEIVE_INTERRUPT));

    // Receive data from master
    return EUSCI_A_SPI_receiveData(EUSCI_A0_BASE);
}

void configureRegisters() {
    // Your register setup using provided settings
    // ...

    // For example, to write to a register:
    // writeSPI(registerAddress);
    // writeSPI(registerValue);
}

void main(void) {
    initSPI();

    // Additional initialization if needed
    configureRegisters();

    __bis_SR_register(LPM0_bits + GIE); // Enter LPM0, enable interrupts
}

//******************************************************************************
//
// USCI_A0 interrupt vector service routine.
//
//******************************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(USCI_A0_VECTOR)))
#endif
void USCI_A0_ISR(void) {
    switch (__even_in_range(UCA0IV, USCI_SPI_UCTXIFG)) {
    case USCI_SPI_UCRXIFG: // UCRXIFG
        while (!EUSCI_A_SPI_getInterruptStatus(EUSCI_A0_BASE, EUSCI_A_SPI_TRANSMIT_INTERRUPT)) {
            // do nothing while txbuf is full
        }

        // Receive data from master
        receiveData = readSPI();

        // Process received data if needed
        // ...

        // Example: Transmit modified data back to master
        transmitData = receiveData ^ 0xFF;
        writeSPI(transmitData);

        break;

    default:
        break;
    }
}
