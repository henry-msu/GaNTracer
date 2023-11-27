//*****************************************************************************
//!   SPI slave talks to SPI master using 3-wire mode. Data is received
//!   from master and data from slave is then transmitted back to master.
//!   USCI RX ISR is used to handle communication, CPU normally in LPM4.
//!   Prior to initial data exchange, master pulses slaves RST for complete
//!   reset.
//!
//!   Use with eusci_spi_ex1_master code example.  If the slave is in
//!   debug mode, the reset signal from the master will conflict with slave's
//!   JTAG; to work around, use IAR's "Release JTAG on Go" on slave device.  If
//!   breakpoints are set in slave RX ISR, master must stopped also to avoid
//!   overrunning slave RXBUF.
//!
//!             Tested on MSP430FR2355
//!                 -----------------
//!            /|\ |                 |
//!             |  |                 |
//!    Master---+->|RST              |
//!                |                 |
//!                |             P1.2|-> Data Out (UCB0SIMO)
//!                |                 |
//!                |             P1.3|<- Data In (UCB0SOMI)
//!                |                 |
//!                |             P1.1|<- Serial Clock In (UCB0CLK)
//!
//! This example uses the following peripherals and I/O signals.  You must
//! review these and change as needed for your own board:
//! - SPI peripheral
//! - GPIO Port peripheral (for SPI pins)
//! - UCB0SIMO
//! - UCB0SOMI
//! - UCB0CLK
//!
//! This example uses the following interrupt handlers.  To use this example
//! in your own application you must add these interrupt handlers to your
//! vector table.
//! - USCI_B0_VECTOR
//!
//
//*****************************************************************************
#include "driverlib.h"

uint8_t transmitData = 0;
uint8_t receiveData = 0;

void main(void) {
    //Stop watchdog timer
    WDT_A_hold(WDT_A_BASE);

    // Configure SPI pins
    // Configure Pins for UCB0CLK, UCB0TXD/UCB0SIMO and UCB0RXD/UCB0SOMI
    /*
    * Select Port 1
    * Set Pin 1, Pin 2 and Pin 3 to input with function.
    */
    GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_P1,
        GPIO_PIN1 + GPIO_PIN2 + GPIO_PIN3,
        GPIO_PRIMARY_MODULE_FUNCTION
    );

    /*
     * Disable the GPIO power-on default high-impedance mode to activate
     * previously configured port settings
     */
    PMM_unlockLPM5();

    //Initialize slave to MSB first, inactive low clock polarity and 3 wire SPI
    EUSCI_B_SPI_initSlaveParam param = {0};
    param.msbFirst = EUSCI_B_SPI_MSB_FIRST;
    param.clockPhase = EUSCI_B_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT;
    param.clockPolarity = EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW;
    param.spiMode = EUSCI_B_SPI_3PIN;
    EUSCI_B_SPI_initSlave(EUSCI_B0_BASE, &param);

    EUSCI_B_SPI_enable(EUSCI_B0_BASE); //Enable SPI Module

    EUSCI_B_SPI_clearInterrupt(EUSCI_B0_BASE,
            EUSCI_B_SPI_RECEIVE_INTERRUPT
    );

    EUSCI_B_SPI_enableInterrupt(EUSCI_B0_BASE,
          EUSCI_B_SPI_RECEIVE_INTERRUPT // enable rx interrupt
    );

    __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0, enable interrupts
}

//******************************************************************************
//
// USCI_B0 interrupt vector service routine.
//
//******************************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_B0_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(USCI_B0_VECTOR)))
#endif
void USCI_B0_ISR (void)
{
    switch(__even_in_range(UCB0IV, USCI_SPI_UCTXIFG))
    {
        case USCI_SPI_UCRXIFG: // UCRXIFG
            while (!EUSCI_B_SPI_getInterruptStatus(EUSCI_B0_BASE, EUSCI_B_SPI_TRANSMIT_INTERRUPT)) {
                // do nothing while txbuf is full
            }
            //Receive data from master
            receiveData = EUSCI_B_SPI_receiveData(EUSCI_B0_BASE);
            transmitData = receiveData ^ 0xFF;
            // Transmit data back to master
            EUSCI_B_SPI_transmitData(EUSCI_B0_BASE, transmitData);

            break;
        default:
            break;
    }
}
