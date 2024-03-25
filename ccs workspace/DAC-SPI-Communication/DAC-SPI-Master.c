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

char packet[] = {0xF0, 0xF0, 0xF0, 0x40};
unsigned int position, i;


int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

    //Initialize SPI 3 Wire
		SPI_Init(); 

	//DCSweep Ilim - Enable PWM controller
        P3DIR |= BIT7;
        P3OUT |= BIT7;

	//IRQs enable
		P4IFG &= ~BIT1; 
		P4IE |= BIT1; 

		UCB0IFG &= ~UCTXIFG;
		UCB0IE |= UCTXIE; 

		__enable_interrupt(); 

	while(1){}
	return 0;
}

//SPI-Initialize the SPI module for communication with the DAC
void SPI_Init(void){

    // Configure eUSCI_B0 for SPI
        UCB0CTLW0 |= UCSWRST;               // Put eUSCI_B0 in reset
        UCB0CTLW0 |= (UCSSEL__SMCLK | UCSYNC | UCMSB | UCMST);           //1MHz clock, Synchronus, 
        UCB0BRW = 10;                     	// Set SPI baud rate ( 10kHz )

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

// void DAC_Tx(uint16_t command, uint16_t address,  uint16_t data){
// 	//---Data Frame:-------------------------------------------------------------
// 	//| 23 | 22 | 21 | 20 | 19 | 18 | 17 | 16 | 15 | 14 | 13 | 12 | 11 | 10 | 09 | 08 | 07 | 06 | 05 | 04 | 03 | 02 | 01 | 01
// 	//----------------------------------------------------------------
// 	//| -  | -  | C2 | C1 | C0 | A2 | A1 | A0 | D11| D10| D9 | D8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | -  | -  | -  | -  |
// 	//---------------------------------------------------------------------------

// 	//Create a data frame that contains command, address, and data in proper spots
// 	uint32_t data_frame = ((command & 0x07) << 20 | (address & 0x07) << 18 | (data & 0x0FFF) << 4);

// 	//Send 3 8bit SPI transmissions to the DAC containing the reduced data frame bits from DB23 - DB0
// 	// Transmit the data frame
//     P1OUT &= ~DAC_CS;     // Select the DAC
//     UCB0TXBUF = (data_frame >> 16) & 0xFF; // Send MSB first
//     while (!(UCB0IFG & UCTXIFG)); // Wait for TX buffer to be ready
//     UCB0TXBUF = (data_frame >> 8) & 0xFF;
//     while (!(UCB0IFG & UCTXIFG)); // Wait for TX buffer to be ready
//     UCB0TXBUF = data_frame & 0xFF;
//     while (!(UCB0IFG & UCTXIFG)); // Wait for TX buffer to be ready
//     P1OUT |= DAC_CS; // Deselect the DAC
	
// }

#pragma vector = PORT4_VECTOR
__interrupt void ISR_PORT4_S1(void){
	position = 0; 
	UCB0TXBUF = packet[position];
	P4IFG &= ~BIT1; 
}


#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void){
	position++; 
	if (position < sizeof(packet)){
		UCB0TXBUF = packet[position];
	}
	else{
		UCB0IFG &= ~UCTXIFG; 
	}
}
