/*---------------------------------------------------------------------------
* EELE 489R 
* 14GaNCT - Lance Gonzalez
* Attempting to configure SPI code for the DAC communication 
*
* Ports: 	P1.0 - CS 	-> 	SYNC(dac)
* 			P1.1 - SClK -> 	SCLK
* 			P1.2 - SIMO -> 	DIN 
*			P3.5 - LDAC 
* 			P3.6 - CLR
*----------------------------------------------------------------------------*/

#include <msp430.h> 
#include <stdint.h>

//----- Macro Definitions ------------------------------------------------------------------------/
#define Wr2_DAC_A		0b00000000
#define Wr2_DAC_B		0b00000001
#define Up_DAC_A		0b00001000
#define Up_DAC_B		0b00001001
#define Wr2_DAC_ALL		0b00000111
#define DAC_SW_RST		0b00101000
#define LDAC_REG_Select	0b00110000
#define DAC_INT_REF		0b00111000
#define DAC_PW_UP		0b00100000

#define UpdateDAC		P3OUT &= ~BIT5; //Set LDAC low to transfer data from in reg to dac reg
#define LatchDAC		P3OUT |= BIT5;  //Set LDAC high to hold value in input register
#define ClearDAC 		P3OUT &= ~BIT6;	//Set CLR low to rest all registers to zero value
//------------------------------------------------------------------------------------------------/

//----- Global Variable Declarations--------------------------------------------------------------/
volatile uint8_t SPI_Frame[] = {0x00, 0x00, 0x00};
volatile int position, i, j;
volatile int DataNow = 0b0000111111111111;

//----- Function Declarations --------------------------------------------------------------------/
void SPI_Init(); 
void DAC_SetTx(uint8_t, uint16_t);
void Send2DAC(); 
void delay(unsigned int);


//----- MAIN PROGRAM -----------------------------------------------------------------------------/
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

    //Initialize SPI 3 Wire
		SPI_Init(); 

//        P4DIR &= ~BIT1;
//        P4REN |= BIT1;
//        P4OUT |= BIT1;
////        P4IES |= BIT1;
//
//        P4IE  |= BIT1;

		__enable_interrupt(); 
		
	//DCSweep Ilim - Enable PWM controller
        P3DIR |= BIT7;
        P3OUT |= BIT7;

	//DAC additional Pin initialization
		P3DIR |= BIT5;			//P3.5 - LDAC
		P3OUT |= BIT5; 

		P3DIR |= BIT6;
		P3OUT |= BIT6; 			//P3.6 - CLR

		UpdateDAC;
		DAC_SetTx(DAC_SW_RST, 0b01);  	//Software reset of the DAC module
		Send2DAC();
		delay(1);

		DAC_SetTx(DAC_INT_REF, 0b00);  //Enable Internal Reference of DAC
		Send2DAC();
		delay(1);

		DAC_SetTx(LDAC_REG_Select, 0b11);  	//Ignore LDAC functionality
		Send2DAC();
		delay(1);

		DAC_SetTx(DAC_PW_UP, 0b000011);  	//Power Up Channels?
		Send2DAC();
		delay(1);


	while(1){

		DAC_SetTx(Wr2_DAC_A, DataNow);	//Write to all DAC channels
		Send2DAC();
		delay(1);

		DAC_SetTx(Up_DAC_A, 0b00);
		Send2DAC(); 

		DAC_SetTx(Wr2_DAC_B, DataNow);
		Send2DAC(); 
		delay(1);

		DAC_SetTx(Up_DAC_B, 0b00);
		Send2DAC(); 

		delay(10);

	}

	return 0;
}

//----- Function Definitions ---------------------------------------------------------------------/
//SPI-Initialize the SPI module for communication with the DAC
void SPI_Init(void){

    // Configure eUSCI_B0 for SPI
        UCB0CTLW0 |= UCSWRST;               // Put eUSCI_B0 in reset
        UCB0CTLW0 |= (UCSSEL__SMCLK | UCSYNC | UCMSB | UCMST);           //1MHz clock, Synchronus, 
        UCB0BRW = 10;                     	// Set SPI baud rate ( 10kHz )
		UCB0CTLW0 |= UCMODE1; 
		UCB0CTLW0 &= ~UCMODE0; 
		UCB0CTLW0 |= UCSTEM; 

    // Configure GPIOs for SPI mode
        P1SEL0 |= BIT0;
        P1SEL1 &= ~BIT0;
        P1OUT |= BIT0;

        P1SEL0 |= BIT1;
        P1SEL1 &= ~BIT1;

        P1SEL0 |= BIT2;
        P1SEL1 &= ~BIT2;	

        PM5CTL0 &= ~LOCKLPM5;				 //Unlock LPM5

        UCB0CTLW0 &= ~UCSWRST;              // Release eUSCI_B0 for operation

		//IRQ enable
		UCB0IFG &= ~UCTXIFG;
		UCB0IE |= UCTXIE; 
}

//Configure the necessary data frames for SPI transmission
void DAC_SetTx(uint8_t commandAddr,  uint16_t data){
	//---Data Frame:-------------------------------------------------------------
	//| 23 | 22 | 21 | 20 | 19 | 18 | 17 | 16 | 15 | 14 | 13 | 12 | 11 | 10 | 09 | 08 | 07 | 06 | 05 | 04 | 03 | 02 | 01 | 01
	//----------------------------------------------------------------
	//| -  | -  | C2 | C1 | C0 | A2 | A1 | A0 | D11| D10| D9 | D8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | -  | -  | -  | -  |
	//---------------------------------------------------------------------------

	//Place 4 don't cares after 12 bit data  to make it a valid 16bit word
		uint16_t TXData =  ((data & 0xFFF) << 4);
		
	// Extract the 3, 2-byte items from the TXData variable
		SPI_Frame[0] = commandAddr;
		SPI_Frame[1] = (TXData >> 8) & 0xFF;						//Send Upper Byte
		SPI_Frame[2] = TXData & 0xFF;								//Send Lower Byte
}

//Send first byte to DAC over SPI - ISR handles other 2 bytes
void Send2DAC(void){
	// P3OUT |= BIT6;                           // Set clr  pin high to enable DAC registers

    position = 0; 
	UCB0TXBUF = SPI_Frame[position];

	LPM0; 
}

//delay somewhat tuned so that the parameter is the number of ms you want to delay
void delay(unsigned int ms){
    unsigned int i;
    for (i = 0; i < ms; i++){
        __delay_cycles(1100); // Assuming 1MHz clock
    }
}

//----- Interrupt Service Routines ---------------------------------------------------------------/
//#pragma vector = PORT4_VECTOR
//__interrupt void ISR_Port4_S1(void){
//	DataNow += 100;
//	P4IFG &= ~BIT1; 	//Clear interrupt
//}

//Iterate through each frame in SPI array, load into TXBUF, wait for TXBUF ready status
#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void){
	position++;
	if (position < sizeof(SPI_Frame)){
		UCB0TXBUF = SPI_Frame[position];
	}
	else{ 
 		LPM0_EXIT;
		UCB0IFG &= ~UCTXIFG; 
	}
}
