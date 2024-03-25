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

//----- Macro Definitions ----------------------------------------------------/
#define W2InRegN 		0b000
#define updateDACRegN 	0b000

#define AddrDacA		0b000
#define AddrDacB		0b001
#define AddrDacAll 		0b111
//----------------------------------------------------------------------------/

//----- Global Variable Declarations------------------------------------------/
char dataframe[];
unsigned int i, j;

typedef struct{
	uint8_t SPI1; 
	uint8_t	SPI2; 
	uint8_t SPI3; 
} FrameParts;
FrameParts SPI_Frames;
//------------------------------------------------------------------------------/

//----- Function Declarations --------------------------------------------------/
void SPI_Init(); 
void DAC_SetTx(uint8_t, uint8_t, uint16_t);
void Send2DAC(); 
//------------------------------------------------------------------------------/

/* MAIN PROGRAM */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

    //Initialize SPI 3 Wire
		SPI_Init(); 

	//Initialize Frame Parts Structure
		SPI_Frames.SPI1 = 0x00; 
		SPI_Frames.SPI2 = 0x00; 
		SPI_Frames.SPI3 = 0x00; 

	//DCSweep Ilim - Enable PWM controller
        P3DIR |= BIT7;
        P3OUT |= BIT7;

	while(1){
		DAC_SetTx(W2InRegN, AddrDacA, 0b00110101); 
		Send2DAC();
		for(j=0; j<1000; j++){}
	}

	return 0;
}

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
        P4DIR &= ~BIT1;
        P4REN |= BIT1;
        P4OUT |= BIT1;
        P4IES |= BIT1;

        P1SEL0 |= BIT0;
        P1SEL1 &= ~BIT0;

        P1SEL0 |= BIT1;
        P1SEL1 &= ~BIT1;

        P1SEL0 |= BIT2;
        P1SEL1 &= ~BIT2;

        PM5CTL0 &= ~LOCKLPM5;				 //Unlock LPM5

        UCB0CTLW0 &= ~UCSWRST;              // Release eUSCI_B0 for operation
}

void DAC_SetTx(uint8_t command, uint8_t address,  uint16_t data){
	//---Data Frame:-------------------------------------------------------------
	//| 23 | 22 | 21 | 20 | 19 | 18 | 17 | 16 | 15 | 14 | 13 | 12 | 11 | 10 | 09 | 08 | 07 | 06 | 05 | 04 | 03 | 02 | 01 | 01
	//----------------------------------------------------------------
	//| -  | -  | C2 | C1 | C0 | A2 | A1 | A0 | D11| D10| D9 | D8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | -  | -  | -  | -  |
	//---------------------------------------------------------------------------

	//Compile the command, address, and data  into a single variable.
		uint32_t dc_mask = 0b00 << 23 | 0b0000 << 0;
		uint32_t TXData = (command << 21) | (address << 18) | (data & 0b111111111111) << 3 | dc_mask;
		
	// Extract the 3, 2-byte items from the TXData variable
		SPI_Frames.SPI1 = (TXData >> 16) & 0xFFFF;
		SPI_Frames.SPI2 = (TXData >> 8) & 0xFF;
		SPI_Frames.SPI3 = TXData & 0xFF;
}

void Send2DAC(void){
    // Pointer to the beginning of the SPI_Frames structure
    uint8_t *ptr = (uint8_t *)&SPI_Frames;
    
    // Iterate through each byte of the structure
    for (i = 0; i < sizeof(SPI_Frames); i++){
        // Send the current byte through SPI
        UCB0TXBUF = *(ptr + i);
        
        // Wait for transmission to complete
        while (!(UCB0IFG & UCTXIFG));
    }
}

// #pragma vector = PORT4_VECTOR
// __interrupt void ISR_PORT4_S1(void){
// //	P1OUT &= ~BIT0;
// //	position = 0;
// //	UCB0TXBUF = [position];
// 	P4IFG &= ~BIT1; 
// }


// #pragma vector = USCI_B0_VECTOR
// __interrupt void USCI_B0_ISR(void){
// //	position++;
// //	if (position < sizeof(dataframe)){
// //		UCB0TXBUF = packet[position];
// //	}
// //	else{
// 		UCB0IFG &= ~UCTXIFG; 
// //	}
// }
