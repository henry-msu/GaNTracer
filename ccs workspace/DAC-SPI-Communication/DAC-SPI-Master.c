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
#define Wr2_DAC_A		0b000000
#define Wr2_DAC_B		0b000001
#define Wr2_DAC_ALL		0b000111

//------------------------------------------------------------------------------------------------/

//----- Global Variable Declarations--------------------------------------------------------------/
volatile uint8_t SPI_Frame[] = {0x00, 0x00, 0x00};
volatile int position, i, j;

//----- Function Declarations --------------------------------------------------------------------/
void SPI_Init(); 
void DAC_SetTx(uint8_t, uint16_t);
void Send2DAC(); 
void UpdateDAC(); 
void ClearDAC(); 


//----- MAIN PROGRAM -----------------------------------------------------------------------------/
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

    //Initialize SPI 3 Wire
		SPI_Init(); 
		__enable_interrupt(); 
		
	//DCSweep Ilim - Enable PWM controller
        P3DIR |= BIT7;
        P3OUT |= BIT7;

	//DAC additional Pin initialization
		P3DIR |= BIT5;			//P3.5 - LDAC
		P3OUT |= BIT5; 

		P3DIR |= BIT6;
		P3OUT |= BIT6; 			//P3.6 - CLR

	while(1){
		DAC_SetTx(Wr2_DAC_ALL, 0b001101011111);
		Send2DAC();
		UpdateDAC(); 
		// for(j=0; j<1000; j++){}
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
	P3OUT |= BIT6;                           // Set clr  pin high to enable DAC registers

    position = 0; 
	UCB0TXBUF = SPI_Frame[position];
}

//Update DAC registers by setting LDAC low and allowing registers to take value of input registers
void UpdateDAC(void){
	P3OUT &= ~BIT5;                        // Set LDAC low -> updates DACregister
	delay(30);
	P3OUT |= BIT5;                          // Set LDAC high -> finish
}

//Set CLR - P3.6 Low to write zero scale to all input and DAC registers
void ClearDAC(void){
	P3OUT &= ~BIT6;                            //CLR is set low
}

//delay somewhat tuned so that the parameter is the number of ms you want to delay
void delay(unsigned int ms){
    unsigned int i;
    for (i = 0; i < ms; i++){
        __delay_cycles(1000); // Assuming 1MHz clock
    }
}

//----- Interrupt Service Routines ---------------------------------------------------------------/
#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void){
	position++;
	if (position < sizeof(SPI_Frame)){
		UCB0TXBUF = SPI_Frame[position];
	}
	else{ 
 		UCB0IFG &= ~UCTXIFG; 
	}
}
