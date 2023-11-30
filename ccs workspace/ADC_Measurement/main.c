#include <msp430.h>
// Define SPI Pin Assignments
#define SPI_CS   BIT4  // P1.4 as Chip Select (CS/STE)
#define SPI_SIMO BIT6  // P1.6 as Slave In Master Out (SIMO)
#define SPI_SOMI BIT7  // P1.7 as Slave Out Master In (SOMI)
#define SPI_CLK  BIT5  // P1.5 as Serial Clock (SCLK)
unsigned char spiReceivedData; // Define globally if used outside ISR
//static unsigned char spiDataReceivedFlag; // Define globally if used outside ISR
unsigned char TxData; // Stores data to be transmitted
unsigned int position;

//unsigned int position;

void initSPI() {
    // Initialize SPI peripheral settings
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;
    // put eUSCI_A0 into software reset
    UCA0CTLW0 |= UCSWRST; // UCSWRST = 1 to put eUSCI_A0 into SW reset
    // configure eUSCI_A0
    UCA0CTLW0 |= UCSSEL__SMCLK; // eUSCI clock is SMCLK = 1MHz
    UCA0BRW = 10;              // Prescaler = 10 to give SCLK = 100kHz
    UCA0CTLW0 |= UCSYNC;
    UCA0CTLW0 |= UCMST;         // Master Select

    UCA0CTLW0 |= UCMODE1;
    UCA0CTLW0 &= ~UCMODE0;      // STL active low (so chip can communicate back)
    //UCA0CTLW0 |= UCMODE0;          // STL Active High
    UCA0CTLW0 |= UCSTEM;        // UCSTEM is used to configure the STE as an output enable for the slave
    UCA0CTLW0 &= ~UC7BIT;       // Character length 8bit
    UCA0CTLW0 |= UCMSB;         // MSB First Select (0=LSB First, 1 = MSB First)
    UCA0CTLW0 |= UCCKPL;        // Clock Inactive state High
    // Set up SPI pins
    //P1DIR |= SPI_CS;       // Set CS as output
    //P1OUT |= SPI_CS;       // Set CS high
    P1SEL1 &= ~SPI_CS;
    P1SEL1 &= ~SPI_SIMO;
    P1SEL1 &= ~SPI_SOMI;
    P1SEL1 &= ~SPI_CLK;  // Assign SPI pins to USCI_A0
    P1SEL0 |= SPI_CS | SPI_SIMO | SPI_SOMI | SPI_CLK; // Assign SPI pins to USCI_A0

    PM5CTL0 &= ~LOCKLPM5;                     // Disable the I/O LPM (low power mode)
    UCA0CTLW0 &= ~UCSWRST;                    // **Initialize USCI state machine**
    // Enable IRQ
    UCA0IFG &= ~UCRXIFG;                      // Clear RXFLG  Flag
    UCA0IE |= UCRXIE;                         // Enable RXIFG IRQS
    UCA0IFG &= ~UCTXIFG;                      // Clear RXFLG  Flag
    UCA0IE |= UCTXIE;                         // Enable RXIFG IRQS

    __enable_interrupt();                    // Enable Maskable IRQs
}

//void selectADC() {
//    P1OUT &= ~SPI_CS;  // Set CS low to select the ADC
//}
//
//void deselectADC() {
//    P1OUT |= SPI_CS;   // Set CS high to deselect the ADC
//}
//--------------------------------------------------------------
//-------SPI Write----------------------------------------------
//--------------------------------------------------------------
void spiWrite(char data) {
    // Wait for any previous SPI transmission to complete
//    while ((UCA0IFG & UCTXIFG) == 0);
    //int i;
    // Load the data into the SPI transmit buffer
    UCA0TXBUF = data;
    //for(i=0; i<10000; i=i+1){}
    // Wait for the transmission to complete
//    while((UCA0IFG & UCTXIFG) == 0);                       //wait for stop flag
//    UCB0IFG &= ~UCSTPIFG;                               // clear flag
    UCA0IFG &= ~UCTXIFG;
    return;
}

//-------------------------------------------------------------
//-------SPI Read----------------------------------------------
//--------------------------------------------------------------
unsigned char spiRead() {
    // Send a dummy byte to read from SPI
//    UCA0TXBUF = 0xFF;

    // Wait for the receive buffer to be ready
//    while ((UCA0IFG & UCRXIFG) == 0);
//    spiReceivedData = UCA0RXBUF;
    UCA0IFG |= UCRXIFG;
    // Read and return the received data
    return ;
}

//unsigned char readADC(unsigned char address) {
//    unsigned char data;
//
//    // Send read command along with the address
//    spiWrite(address);
//
//    // Read the data from the ADC
//    data = spiRead();
//
//    return data;
//}
//--------------------------------------------------------------
//-------ADC CONFIG----------------------------------------------
//--------------------------------------------------------------
void configureADC() {
    // Send configuration commands to MCP3462R
    // Set external voltage reference, 16-bit resolution, gain of 1
    // Register address for configuring the external reference, resolution, and gain
    char SPIAddress = 0b11000000; // ADC address (0x3)
    // Configuration settings
    char Reg_Address = 0b00000100; // Value for Config0 [0x1]
    char RWSetting = 0b00000010; // Value for Writing (0x02)

    char configValue;// 0b11000110

    char config0 = 0b01101100; // values from One Note (11.27.2023 ADC Config)
    char config1 = 0b00000000;
    char config2 = 0b10001011;
    char config3 = 0b11000000;
    char IRQ = 0b01111011;
    char MUX = 0b00000001;  // channel 0 and 1 (reg address 0x6)
    // Combine the settings for ADC Addres
    configValue = SPIAddress | Reg_Address | RWSetting;

    // Write the configuration value to the ADC's configuration register
    spiWrite(configValue); //
    spiWrite(config0);
    spiWrite(config1);
    spiWrite(config2);
    spiWrite(config3);
    spiWrite(IRQ);
    spiWrite(MUX);
}
//--------------------------------------------------------------
//-------Main----------------------------------------------
//--------------------------------------------------------------
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    // Initialize SPI and configure the ADC
    initSPI();
    configureADC();

    // Main loop
    while(1) {
        // Assuming these are the correct commands or addresses for reading
        // Replace these with actual values from the MCP3462R datasheet

//        unsigned char channel1Address = 0x01; // Address for Channel 0-1
//        unsigned char channel2Address = 0x23; // Address for Channel 2-3

        // Read data from Channel 1
//        unsigned char dataChannel1 = readADC(channel1Address);

        // Read data from Channel 2
//        unsigned char dataChannel2 = readADC(channel2Address);

        // Process the read data
        // ...

        // Add a delay or some condition to control the frequency of ADC reads
        // ...
        spiRead();
    }
}
//I hate data sheets!!
//MSP430 chapter 23
#pragma vector=EUSCI_A0_VECTOR
__interrupt void ISR_EUSCI_A0(void) {
    //__interrupt void EUSCI_B0_I2C_ISR(void)
        switch (UCA0IV)
        {
        case 0x02:  //RX Flag
            spiReceivedData = UCA0RXBUF; // Read RX Buff
            UCA0IFG &= ~UCRXIFG;
            //UCA0IFG &= ~UCTXIFG;
            break;

        case 0x04:
//            position++;
//            if(position < sizeof(TxData)){
//                UCA0TXBUFF = TxData[position];
//            }
//            else{
            //while ((UCA0IFG & UCRXIFG) == 0);
            //UCA0IFG &= ~UCTXIFG;
           // __delay_cycles(40);
            //UCA0IFG &= ~UCRXIFG;
//            }
            break;
        default:
            break;
        }

}


