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


// void SPI_DAC_INIT(); 
// void SPI_transmit(); 

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	

    // Configure SPI
		UCB0CTLW0 |= UCSWRST;  		// Put eUSCI_B in reset
		
		// Configure SPI pins
			P1SEL0 |=  BIT2 | BIT1;  	//SIMO (P1.2), SCLK (P1.1)
			P1SEL1 &= ~(BIT2 | BIT1);
			P1DIR |= BIT0;   			// Set CS pin (P1.0) as output
			P1OUT |= BIT0;   			// Set CS pin high initially

		UCB0CTLW0 |= UCSYNC | UCMSB | UCMST | UCSSEL__SMCLK;  // 3-pin, 8-bit SPI master
		UCB0BRW = 10;           	// Set SPI clock to SMCLK / 1
		UCB0CTLW0 &= ~UCSWRST;  	// Release eUSCI_B from reset

	//DCSweep Ilim - Enable PWM controller
	P3DIR |= BIT7;
	P3OUT |= BIT7;

	//Unlock LPM5
	PM5CTL0 &= ~LOCKLPM5;

	int i;

	while(1){
		UCB0TXBUF = 0xB2;
		__delay_cycles(1000); 
	}
	return 0;
}

// #pragma vector = USCI_B0_VECTOR
// __interrupt void USCI_B0_ISR(void){
// 	switch(__even_in_range(UCB0IV, USCI_SPI_UCTXIFG)){
// 		case USCI_NONE: break;             // Vector 0: No interrupts
//         case USCI_SPI_UCTXIFG:             // Vector 2: TXIFG
//             P1OUT |= DAC_CS;               // Deselect the DAC
//             break;
//         default: break;
// 	}
// }
