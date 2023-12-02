#include <msp430.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Define SPI Pin Assignments
#define SPI_CS   BIT4  // P1.4 as Chip Select (CS/STE)
#define SPI_SIMO BIT6  // P1.6 as Slave In Master Out (SIMO)
#define SPI_SOMI BIT7  // P1.7 as Slave Out Master In (SOMI)
#define SPI_CLK  BIT5  // P1.5 as Serial Clock (SCLK)
unsigned char spiReceivedData; // Define globally if used outside ISR
//static unsigned char spiDataReceivedFlag; // Define globally if used outside ISR
unsigned char TxData;  // Stores data to be transmitted
char Settings[] = {
        0b01000110, // CMD 0x46
        0b01101111, // Config0 0x6F
        0b00000000, // Config1 0x00
        0b10001011, // Config2 0x8B
        0b11000000, // Config3 0xC0
        0b01111011, // IRQ 0x7B
        0b00000001};  // MUX 0x01 - channel 0 and 1 (reg address 0x6)
unsigned int position = 0;
char ChanMux[2] = {0b01011011,0b00000000}; // 0x5B       // holds the 2 address we need to write to
unsigned int Muxed = 1; // Keeps track if channel mux was changed 1(false)
unsigned int i;
// UART Global Variables
#define max_UART_Length 128
char UART_Message_Global[max_UART_Length];
char *UART_Message_ptr = UART_Message_Global;
unsigned int UART_Position_Counter;
int UART_Message_Length;
char Temp_Raw_ASCII[5];
char *Temp_Raw_ASCII_ptr = Temp_Raw_ASCII;

void initSPI() {
    // Initialize SPI peripheral settings
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;
    // put eUSCI_A0 into software reset
    UCA0CTLW0 |= UCSWRST; // UCSWRST = 1 to put eUSCI_A0 into SW reset
    // configure eUSCI_A0
    UCA0CTLW0 |= UCSSEL__SMCLK; // eUSCI clock is SMCLK = 1MHz
    UCA0BRW = 20;              // Prescaler = 10 to give SCLK = 100kHz
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
    UCA0TXBUF = data;
    //UCA0IFG &= ~UCTXIFG;
    return;
}

//-------------------------------------------------------------
//-------SPI Read----------------------------------------------
//--------------------------------------------------------------
unsigned char spiRead() {
    UCA0IFG |= UCRXIFG;
    //spiReceivedData = UCA0RXBUF; // Read RX Buff
    // Read and return the received data
    return spiReceivedData;
}

unsigned int readADC() {
    for (i = 0; i < 100; i++){}

    unsigned char byte1,byte2; // assume MSBs, Then LSBs
    //char packet[2] = {0b01011011,address};
    // Send read command along with the address
    spiWrite(0b01000001); // 0x41 (write to read from 0x00 adc reg)
    while ((UCA0IFG & UCTXIFG) == 0);
    spiWrite(0xFF); // dummy write so we can read
    // Read the data from the ADC
    byte1 = spiRead();     // read from 0x00 but the we can only read 2bytes
    //while ((UCA0IFG & UCRXIFG) == 0);
//    for (i = 0; i < 100; i++){}
    spiWrite(0xFF); // dummy write so we can read
    // Read the data from the ADC
    byte2 = spiRead();     // read from 0x00 trying to get 2nd 2bytes
    int data = byte1;
    data = data << 8;
    data = data | byte2;
    return data;
}


void changechannel(char address) {
    for (i = 0; i < 100; i++){}
    ChanMux[1] = address;
    // Send read command along with the address
    Muxed = 0;
    // Read the data from the ADC
    UCA0TXBUF = ChanMux[0];

    return;
}
//--------------------------------------------------------------
//-------ADC CONFIG----------------------------------------------
//--------------------------------------------------------------
void configureADC() {
//    // Send configuration commands to MCP3462R
//    // Set external voltage reference, 16-bit resolution, gain of 1
//    // Register address for configuring the external reference, resolution, and gain
//    char SPIAddress = 0b01000000; // ADC address (0x1)
//    // Configuration settings
//    char Reg_Address = 0b00000100; // Value for Config0 [0x1]
//    char RWSetting = 0b00000010; // Value for Writing (0x02)
//
//    char configValue;// 0b01000110
//    // Combine the settings for ADC Address
//    configValue = SPIAddress | Reg_Address | RWSetting;
//    char config0 = 0b01101100; // values from One Note (11.27.2023 ADC Config)
//    char config1 = 0b00000000;
//    char config2 = 0b10001011;
//    char config3 = 0b11000000;
//    char IRQ = 0b01111011;
//    char MUX = 0b00000001;  // channel 0 and 1 (registor address 0x6)
//    UCA0TXBUF = Settings[position];
    //UCA0IFG |= UCTXIFG;
    UCA0TXBUF = Settings[position];
   // if (sizeof(Settings) > 0) {
//            UCA0TXBUF = Settings[0];  // Start with the first byte
            //position = 1;             // Next byte to send
        //}
    return;
}



void Setup_UART(void){
    // Take eUSCI_B1 out of software reset with UCSWRST = 0
    UCA1CTLW0 &= ~UCSWRST;

    // UART Communication to PC, Setup UART A0 (Tx)

    UCA1CTLW0 |= UCSWRST;

    UCA1CTLW0 |= UCSSEL__SMCLK; // Using SM clock

    //UCA1BRW = 6;              //For 9600
    UCA1BRW = 1;                //For 38400

    //UCA1MCTLW |= 0x2081;        //For 9600
    UCA1MCTLW |= 0x00A1;

//    P1SEL1 &= ~BIT7; // Use pin 1.7 for UART TX to Bluetooth
//    P1SEL0 |= BIT7;
//
//
//    P1SEL1 &= ~BIT6; // Use pin1.6 for UART RX on Bluetooth
//    P1SEL0 |= BIT6;


    UCA1CTLW0 &= ~UCSWRST; // Take UART A0 out of SW Reset
}


//void Send_UART_Message(int Size_UART_Message){
//    UART_Position_Counter = 0;
//    UART_Message_Length = Size_UART_Message;
//    UCA1IE |= UCTXCPTIE;                                             //Enable the TX IRQ
//    UCA1IFG &= ~UCTXCPTIFG;                                          //Clear the TX IFG
//    UCA1TXBUF = UART_Message_Global[UART_Position_Counter];          //Put first value into the tx buffer
//
//}

//--------------------------------------------------------------
//-------Main----------------------------------------------
//--------------------------------------------------------------
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    // Initialize SPI and configure the ADC
    initSPI();
    configureADC();
    //Setup_UART();
    for (i = 0; i < 10; i++){}

    unsigned char channel1Address = 0x01; // Address for Channel 0-1
    unsigned char channel2Address = 0x23; // Address for Channel 2-3
    // Main loop

    while(1) {
        // Read data from Channel 0-1
        for (i = 0; i < 10; i++){}
        changechannel(channel1Address);
        for (i = 0; i < 10; i++){}
        unsigned int dataChannel1 = readADC();
        for (i = 0; i < 100; i++){}
        // Read data from Channel 2-3
        changechannel(channel2Address);
        for (i = 0; i < 10; i++){}
        unsigned int dataChannel2 = readADC();

        // Process the read data
        // ...
        // Add a delay or some condition to control the frequency of ADC reads
        // ...
        //spiRead();

        //UCA1TXBUF = 'T';
        for (i = 0; i < 10000; i++){}
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

        case 0x04:  // TX Flag
            if (position < sizeof(Settings)-1)
            {
                position++;
                UCA0TXBUF = Settings[position];
                UCA0IFG &= ~UCTXIFG;
            }
            else if (Muxed == 0)
            {
                UCA0TXBUF = ChanMux[1];
                Muxed = 1;
                UCA0IFG &= ~UCTXIFG;
            }
            else
            {
                UCA0IFG &= ~UCTXIFG;
            }

//            while ((UCA0IFG & UCRXIFG) == 0);
//            UCA0IFG &= ~UCTXIFG;
//            UCA0IFG &= ~UCTXIFG;
           // __delay_cycles_cycles(40);
//            }
            break;
        default:
            break;
        }
}

//#pragma vector=EUSCI_A1_VECTOR
//__interrupt void ISR_EUSCI_A1(void) {
//    if(position == sizeof(message)){
//        UCA1IE &= UCTXCPTIE;
//    }
//    else{
//        UCA1TXBUF = measage[position];
//    }
//    UCA1IFG &= ~UCTXCPTIFG;
//}

