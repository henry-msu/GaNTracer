#include <msp430.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Define SPI Pin Assignments
#define SPI_CS   BIT4  // P1.4 as Chip Select (CS/STE)
#define SPI_SIMO BIT6  // P1.6 as Slave In Master Out (SIMO)
#define SPI_SOMI BIT7  // P1.7 as Slave Out Master In (SOMI)
#define SPI_CLK  BIT5  // P1.5 as Serial Clock (SCLK)
unsigned char spiReceivedData; // Define globally if used outside ISR
//static unsigned char spiDataReceivedFlag; // Define globally if used outside ISR
char Settings[] = {
        0b01000110, // CMD 0x46
        0b01110011, // Config0 0x7F (0x01)
        0b00000000, // Config1 0xC0
        0b10001011, // Config2 0x8B
        0b10000010,//, 0b10100010// Config3 0xC0
        0b01110111, // IRQ 0x7F
        0b00000001};//0b00100011};  // MUX 0x01 - channel 0 and 1 (reg address 0x6)
//        0b00100000, // SCAN reg
//        0b00100011, // address
//        0b00101000};// end scan reg

#define NUM_SAMPLES 20
// Global variables
unsigned int samples1[NUM_SAMPLES];
unsigned int samples2[NUM_SAMPLES];
unsigned int currentIndex = 0;

unsigned int position;
char ChanMux[2] = {0b01011010,0b00000000}; // 0x5B       // holds the 2 address we need to write to
unsigned int Muxed = 1; // Keeps track if channel mux was changed 1(false)
unsigned int i;
char channel1Address = 0x01;//0x08;// single-ended CH0 //0x01; // Address for Channel 0-1
char channel2Address = 0x23;//0x28;// single-ended CH2 //0x23; // Address for Channel 2-3
// UART Global Variables
#define max_UART_Length 64
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
    UCA0BRW = 26;              // Prescaler = 10 to give SCLK = 100kHz
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
    P1DIR |= BIT2;           // Set IRQ as output
    P1OUT &= BIT2;           // Set IRQ high
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
void selectIQR() {
    P1OUT |= BIT2;  // Set CS low to select the ADC
}

void deselectIQR() {
    P1OUT &= ~BIT2;   // Set CS high to deselect the ADC
}

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
    //UCA0IFG |= UCRXIFG;
    spiReceivedData = UCA0RXBUF; // Read RX Buff
    // Read and return the received data
    return spiReceivedData;
}
//-----4byte read --------------
//unsigned int readADC() {
//    for (i = 0; i < 100; i++){}
//    //selectIQR();
//    unsigned int byte1,byte2; // assume MSBs, Then LSBs
//    //char packet[2] = {0b01011011,address};
//    // Send read command along with the address
//    spiWrite(0b01000011); // 0x41 (write to read from 0x00 adc reg)
//    while (!(UCTXIFG));
//    for(i = 0; i < 12; i++){
//       spiWrite(0xFF);
//       while (!(UCTXIFG));
//
//    }
//    //spiRead();
//    spiRead();       // some sort of timing issue
//    spiRead();
//    byte1 = spiRead();     // read from 0x00 but the we can only read 2bytes
//    //while (!(UCA0IFG));
//    byte2 = spiRead();     // read from 0x00 trying to get 2nd 2bytes
//    //while (UCA0IFG & UCTXIFG);
//    int data = (byte1 << 8)| byte2;
//   // deselectIQR();
//    return data;
//}

//-----2byte read ---------------
unsigned int readADC() {
    for (i = 0; i < 100; i++){}
    //selectIQR();
    unsigned int byte1,byte2; // assume MSBs, Then LSBs
    //char packet[2] = {0b01011011,address};
    // Send read command along with the address
    spiWrite(0b01000011); // 0x41 (write to read from 0x00 adc reg)
    while (!(UCTXIFG));
    for(i = 0; i < 4; i++){
       spiWrite(0xFF);
       while (!(UCTXIFG));
       spiRead();       // some sort of timing issue
    }
    spiRead();
    while (!(UCA0IFG & UCRXIFG));
    byte1 = spiRead();     // read from 0x00 but the we can only read 2bytes
    while (!(UCA0IFG & UCRXIFG));
    byte2 = spiRead();     // read from 0x00 trying to get 2nd 2bytes
    //while (UCA0IFG & UCTXIFG);
    int data = (byte1 << 8)| byte2;
   // deselectIQR();
    return data;
}
//-----------------------------------------------------------------------
//--------------------------------------------------------------------------------------
void regcheck(){
    for (i = 0; i < 100; i++){}
        spiWrite(0b01000111); // read config regs 0x1starting address
        while (!(UCA0IFG & UCTXIFG));
//        spiWrite(0xFF); // dummy write so we can read
//         // Read the data from the ADC
////        spiRead();     // read from 0x00 but the we can only read 2bytes
////        while (!(UCA0IFG & UCRXIFG));
//               for (i = 0; i < 10; i++){}
//        spiWrite(0xFF); // dummy write so we can read
        //--- reads 4 more values
        for(i = 0; i < 35; i++){
            //while (!(UCRXIFG));
            spiWrite(0xFF);
        }
    }


void unlock(){      // unlock the register protection.
    //selectIQR();
    spiWrite(0x76);  // 0x76 0b01110110 write to 0xD
    spiWrite(0xA5); // 0xA5 the "password"
//    while (!(UCA0IFG & UCTXIFG));
//    while (!(UCA0IFG & UCRXIFG))
    for (i = 0; i < 100; i++){}
    //de//selectIQR();
    return;
}

void lock(){
    spiWrite(0x76);  // 0x76 0b01110110 write to 0xD
    spiWrite(0x00); // 0xA5 the "password"
    //    while (!(UCA0IFG & UCTXIFG));
    //    while (!(UCA0IFG & UCRXIFG))
        for (i = 0; i < 100; i++){}
}

void changechannel(char address) {
    //selectIQR();
    for (i = 0; i < 100; i++){}
    ChanMux[1] = address;
    // Send read command along with the address
    Muxed = 0;
    // Read the data from the ADC
    UCA0TXBUF = ChanMux[0];
    //while(Muxed < 1) {};
//    //de//selectIQR();
    return;
}

void offset(char value1, char value2){    // needs to be two’s complement
    spiWrite(0b01100110);  //  write to 0x9
    spiWrite(value1); // should be 0xFF
    while (!(UCTXIFG));
    spiWrite(value2); // offset value
    //    while (!(UCA0IFG & UCRXIFG))
        for (i = 0; i < 100; i++){}
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
    //selectIQR();
    position = 0;
    UCA0TXBUF = Settings[position];
    while(position < sizeof(Settings)-1) {};
    //de//selectIQR();
    return;
}
//---------------------------------------------------------
// Sends the Data for easier viewing
//-------------------------------------------------------
void Setup_UART(void){
    // Take eUSCI_B1 out of software reset with UCSWRST = 0
    UCA1CTLW0 |= UCSWRST;
    // UART Communication to PC, Setup UART A1 (Tx)
    UCA1CTLW0 |= UCSWRST;
    UCA1CTLW0 |= UCSSEL__SMCLK; // Using SM clock 1MHz
    //UCA1BRW = 6;              //For 9600
    UCA1BRW = 1;                //For 38400
    //UCA1MCTLW |= 0x2081;        //For 9600
    UCA1MCTLW |= 0x00A1;
    P4SEL1 &= ~BIT3;
    P4SEL0 |= BIT3;
    UCA1CTLW0 &= ~UCSWRST; // Take UART A1 out of SW
    UCA1IFG &= ~UCTXIFG;                      // Clear RXFLG  Flag
    UCA1IE |= UCTXIE; // Enable USCI_A1 TX interrupt
}

void Send_UART(int value) {
    while (!(UCA1IFG & UCTXIFG)); // Wait for TX buffer to be ready for high byte
    UCA1TXBUF = (value >> 8) & 0xFF; // Send high byte
//    for (i = 0; i < 100; i++){}
    while (!(UCA1IFG & UCTXIFG)); // Wait for TX buffer to be ready for low byte
    UCA1TXBUF = value & 0xFF; // Send low byte
//    for (i = 0; i < 100; i++){}
    while (!(UCA1IFG & UCTXIFG)); // Wait for TX buffer to be ready for newline
    UCA1TXBUF = '\r'; // Send carriage return

    while (!(UCA1IFG & UCTXIFG)); // Wait for TX buffer to be ready for newline
    UCA1TXBUF = '\n'; // Send newline
//    for (i = 0; i < 100; i++){}
}
//void Send_UART(unsigned int value) {
//    char numStr[7]; // Enough for 5 digits plus '\r\n\0'
//    sprintf(numStr, "%u\r\n", value); // Convert to string and append CRLF
//
//    for (i = 0; numStr[i] != '\0'; i++) {
//        while (!(UCA1IFG & UCTXIFG)); // Wait for TX buffer to be ready
//        UCA1TXBUF = numStr[i]; // Send character
//    }
//}

//void Send_UART(int value){
////    UCA1IE |= UCTXCPTIE;                                             //Enable the TX IRQ
////    UCA1IFG &= ~UCTXCPTIFG;                                          //Clear the TX IFG
//    UCA1TXBUF = value;
//}
//void Send_UART_Message(int Size_UART_Message){
//    UART_Position_Counter = 0;
//    UART_Message_Length = Size_UART_Message;
//    UCA1IE |= UCTXCPTIE;                                             //Enable the TX IRQ
//    UCA1IFG &= ~UCTXCPTIFG;                                          //Clear the TX IFG
//    UCA1TXBUF = UART_Message_Global[UART_Position_Counter];          //Put first value into the tx buffer
//
//}

// Function to update the samples array with a new value
void updateSamples(unsigned int newValue1,signed int newValue2) {
    samples1[currentIndex] = newValue1;
    samples2[currentIndex] = newValue2;
    currentIndex = (currentIndex + 1) % NUM_SAMPLES;
    return;
}

// Function to calculate the average of the samples array
unsigned int calculateAverage1() {
    unsigned long sum = 0;
    int d;
    for (d = 0; d < NUM_SAMPLES; d++) {
        sum += samples1[d];
    }
    return (unsigned int)(sum / NUM_SAMPLES);
}
// Function to calculate the average of the samples array
unsigned int calculateAverage2() {
    unsigned long sum = 0;
    int d;
    for (d = 0; d < NUM_SAMPLES; d++) {
        sum += samples2[d];
    }
    return (unsigned int)(sum / NUM_SAMPLES);
}
//--------------------------------------------------------------
//-------Main----------------------------------------------
//--------------------------------------------------------------
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
    // Initialize SPI and configure the ADC
    //while(1){
    initSPI();
    spiWrite(0b01111000); // resets registers (works)
    for (i = 0; i < 10; i++){}
    unlock();
    for (i = 0; i < 10; i++){}
    configureADC();
    //Setup_UART();
    for (i = 0; i < 10; i++){}
    //lock();
   //}
    //offset(0b11111111, 0b11100000);  //-8 : 1111 1111 1111 1000
    // Main loop
    while(1) {
//        for (i = 0; i < 100; i++){}
//        spiWrite(0b01101010); // write to interupts
//        spiWrite(0b01111111); // resets the interupts values
//        for (i = 0; i < 100; i++){}


//        regcheck();
////        for (i = 0; i < 100; i++){}
////        spiWrite(0b01101000);

        //        for (i = 0; i < 10; i++){}y
        // Read data from Channel 0-1
        changechannel(channel1Address);
        for (i = 0; i < 25; i++){}
        spiWrite(0b01101000); // start converstion 0x68
        for (i = 0; i < 10; i++){}
        unsigned int dataChannel1 = readADC();
        // Read data from Channel 2-3
        //        for (i = 0; i < 10; i++){}
        for (i = 0; i < 100; i++){}
        changechannel(channel2Address);
        for (i = 0; i < 25; i++){}
        spiWrite(0b01101000); // start converstion 0x68
        for (i = 0; i < 10; i++){}
        unsigned int dataChannel2 = readADC();
        // Process the read data
        // ...
        // Add a delay or some condition to control the frequency of ADC reads
        // ...
        for (i = 0; i < 200; i++){}
//------------------------------------------------
//Average the readings--------------------------------
//------------------------------------------------
        // Update the samples array with the new value
        updateSamples(dataChannel1,dataChannel2);
        // Calculate the average of the samples array
        unsigned int averagedCH0 = calculateAverage1();
        unsigned int averagedCH2 = calculateAverage2();

        for (i = 0; i < 200; i++){}
        Send_UART(averagedCH2);
        //UCA1TXBUF = (char)dataChannel1;
        //UCA1TXBUF = 'T';
//        while(1){
//            spiWrite(0b01111000);
//            for (i = 0; i < 100; i++){}
//            regcheck();
//        }

    }
}
//I hate data sheets!!
//MSP430 chapter 23
//#pragma vector=EUSCI_A0_VECTOR
//__interrupt void ISR_EUSCI_A0(void) {
//    //__interrupt void EUSCI_B0_I2C_ISR(void)
//        switch (UCA0IV)
//        {
//        case 0x02:  //RX Flag
//            //spiReceivedData = UCA0RXBUF; // Read RX Buff
//            UCA0IFG &= ~UCRXIFG;
//            //UCA0IFG &= ~UCTXIFG;
//            break;
//
//        case 0x04:  // TX Flag
//            if (position < sizeof(Settings)-1)
//            {
//                position++;
//                UCA0TXBUF = Settings[position];
//                UCA0IFG &= ~UCTXIFG;
//            }
//            else if (Muxed == 0)
//            {
//                UCA0TXBUF = ChanMux[1];
//                Muxed = 1;
//                //de//selectIQR();
//                UCA0IFG &= ~UCTXIFG;
//            }
//            else
//            {
//                UCA0IFG &= ~UCTXIFG;
//            }
//            break;
////            while ((UCA0IFG & UCRXIFG) == 0);
////            UCA0IFG &= ~UCTXIFG;
////            UCA0IFG &= ~UCTXIFG;
//           // __delay_cycles_cycles(40);
////            }
//
//        default:
//            UCA0IFG &= ~UCTXIFG;
//            UCA0IFG &= ~UCRXIFG;
//            break;
//        }
//}
#pragma vector=EUSCI_A0_VECTOR
__interrupt void ISR_EUSCI_A0(void) {
    switch (UCA0IV) {
        case 0x02:  // RX Flag
            spiReceivedData = UCA0RXBUF;  // Read RX Buffer and store in global variable
            break;

        case 0x04:  // TX Flag
            if (position < sizeof(Settings) - 1) {
                UCA0TXBUF = Settings[++position];  // Load next byte into TX buffer
            } else if (!Muxed) {
                UCA0TXBUF = ChanMux[1];  // Change the multiplexer channel if needed
                Muxed = 1;
            }
            break;

        default:
            // No other cases handled
            break;
    }
}


#pragma vector=EUSCI_A1_VECTOR
__interrupt void ISR_EUSCI_A1(void) {
    if (UCA1IFG & UCTXIFG) {
        // Add code here to load the next byte into UCA1TXBUF if you have more data to send
        // Example: UCA1TXBUF = next_byte_to_send;
        // Make sure to track the end of your data to avoid buffer overruns

        UCA1IFG &= ~UCTXIFG;  // Clear TX interrupt flag
    }
}


//#pragma vector=EUSCI_A1_VECTOR
//__interrupt void ISR_EUSCI_A1(void) {
////    if(position == sizeof(message)){
////        UCA1IE &= UCTXCPTIE;
////    }
////    else{
////        UCA1TXBUF = measage[position];
////    }
//    UCA1IFG &= ~UCTXCPTIFG;
//    //UCA1IE &= ~UCTXCPTIE;
//}


