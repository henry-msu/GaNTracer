/*
 * FT232H (or any SPI master) to MSP430 slave using 4-wire, CS active low mode
 *
 *  FT232H                   MSP430FR2355
 * --------|               |----------------|
 *       DO|->Data Out---->|P1.2 (UCB0SIMO) |
 *         |               |                |
 *       DI|->Data In----->|P1.3 (UCB0SOMI) |
 *         |               |                |
 *       SK|->Clock------->|P1.1 (UCB0CLK)  |
 *         |               |                |
 *       CS|->Chip select->|P1.0 (UCB0STE)  |
 * --------|               |                |
 *                         |     (MCLK) P3.0|--> MCLK output
 *                         |----------------|
 *
 * This example uses the following peripherals and I/O signals:
 * - CS module (to set MCLK frequency to 24 MHz
 * - EUSCI_B0 module (SPI mode)
 * - GPIO Port module (for SPI pins)
 *   - UCB0STE
 *   - UCB0CLK
 *   - UCB0SIMO
 *   - UCB0SOMI
 *   - MCLK
 *
 * This example uses the following interrupt handlers:
 * - USCI_B0_VECTOR
 * - UNMI_VECTOR
 *
 */
#include "driverlib.h"
#include "Board.h"

#define CS_MCLK_DESIRED_FREQUENCY_IN_KHZ 24000 // Target frequency for MCLK in kHz
#define CS_MCLK_FLLREF_RATIO CS_MCLK_DESIRED_FREQUENCY_IN_KHZ/32.768 // MCLK/FLLRef Ratio
#define DATA_SIZE 2048 // how many bytes to send/receive

uint32_t clockValue = 0; // current Clock values
uint16_t status; // status of Oscillator fault flags

uint8_t receiveData[DATA_SIZE] = {0}; // SPI RX memory
uint16_t i = 0; // basic iterator
uint8_t instruction = 0; // buffer to store what SPI instruction is being executed
uint8_t executing = 0; // status of SPI, are we waiting for a new instruction or executing one

void main(void) {
    WDT_A_hold(WDT_A_BASE); // Stop watchdog timer

    /*
     * Configure pins for UCB0STE, UCB0CLK, UCB0SIMO and UCB0SOMI
     * Set P1.0:3 to input with their primary function.
     */
    GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_P1,
        GPIO_PIN0 + GPIO_PIN1 + GPIO_PIN2 + GPIO_PIN3,
        GPIO_PRIMARY_MODULE_FUNCTION
    );

    // MCLK output to GPIO
    GPIO_setAsPeripheralModuleFunctionOutputPin(
        GPIO_PORT_MCLK,
        GPIO_PIN_MCLK,
        GPIO_FUNCTION_MCLK
        );

    /*
     * Disable the GPIO power-on default high-impedance mode to activate
     * previously configured port settings
     */
    PMM_unlockLPM5();

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

    /*
     * Initialize EUSCI_B0
     * SPI slave
     * MSB first
     * Active high clock polarity
     * 4 wire SPI
     * CS active low
     */
    EUSCI_B_SPI_initSlaveParam spiparam = {0};
    spiparam.msbFirst = EUSCI_B_SPI_MSB_FIRST;
    spiparam.clockPhase = EUSCI_B_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT;
    spiparam.clockPolarity = EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW;
    spiparam.spiMode = EUSCI_B_SPI_4PIN_UCxSTE_ACTIVE_LOW;
    EUSCI_B_SPI_initSlave(EUSCI_B0_BASE, &spiparam);
    EUSCI_B_SPI_enable(EUSCI_B0_BASE); // Enable SPI module

    EUSCI_B_SPI_clearInterrupt(EUSCI_B0_BASE, EUSCI_B_SPI_RECEIVE_INTERRUPT);
    EUSCI_B_SPI_enableInterrupt(EUSCI_B0_BASE, EUSCI_B_SPI_RECEIVE_INTERRUPT); // Enable RX interrupt

    SFR_enableInterrupt(SFR_OSCILLATOR_FAULT_INTERRUPT); // Enable oscillator fault interrupt

    // MAIN LOOP
    while(1) {
        __bis_SR_register(LPM0_bits + GIE); // Enter LPM0, enable interrupts
    }
}

//******************************************************************************
// USCI_B0 interrupt vector service routine.
//******************************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_B0_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(USCI_B0_VECTOR)))
#endif
void USCI_B0_ISR (void) {
    switch(__even_in_range(UCB0IV, USCI_SPI_UCTXIFG)) {
        case USCI_SPI_UCRXIFG: // UCRXIFG
            if(!executing) { // if not currently executing something, read in an instruction
               instruction = EUSCI_B_SPI_receiveData(EUSCI_B0_BASE);
               i = 0;
               executing = 1;
               if(instruction == 0x01) { // write data back to master
                   i = 0;
                   EUSCI_B_SPI_disableInterrupt(EUSCI_B0_BASE, EUSCI_B_SPI_RECEIVE_INTERRUPT); // disable RX interrupt
                   EUSCI_B_SPI_clearInterrupt(EUSCI_B0_BASE, EUSCI_B_SPI_TRANSMIT_INTERRUPT);
                   EUSCI_B_SPI_enableInterrupt(EUSCI_B0_BASE, EUSCI_B_SPI_TRANSMIT_INTERRUPT); // enable TX interrupt
                   // place first byte in transmit buffer, need to load it during this instruction or else the transmission will be late by 1 byte
                   EUSCI_B_SPI_transmitData(EUSCI_B0_BASE, receiveData[i++]);
               }
            }
            else if(instruction == 0x00) { // read bytes from master
                receiveData[i++] = EUSCI_B_SPI_receiveData(EUSCI_B0_BASE);
                if(i >= DATA_SIZE) {
                    i = 0;
                    executing = 0;
                    instruction = 0;
                }
            }
            break;
        case USCI_SPI_UCTXIFG: // UCTXIFG
            if(i <= DATA_SIZE - 1) {
                EUSCI_B_SPI_transmitData(EUSCI_B0_BASE, receiveData[i++]);
            }
            else if(i == DATA_SIZE) {
            	i++;
            }
            else if(i > DATA_SIZE) {
                i = 0;
                executing = 0;
                instruction = 0;
                EUSCI_B_SPI_disableInterrupt(EUSCI_B0_BASE, EUSCI_B_SPI_TRANSMIT_INTERRUPT); // disable TX interrupt
                EUSCI_B_SPI_clearInterrupt(EUSCI_B0_BASE, EUSCI_B_SPI_RECEIVE_INTERRUPT);
                EUSCI_B_SPI_enableInterrupt(EUSCI_B0_BASE, EUSCI_B_SPI_RECEIVE_INTERRUPT); // enable RX interrupt
            }
        default:
            break;
    }
}

//******************************************************************************
// NMI vector service routine.
//******************************************************************************
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
