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

#define DAC_CS BIT0				//CS is connected to P1.0

void SPI_DAC_INIT(); 
void SPI_transmit(); 

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	// SPI_DAC_INIT();

	P3DIR |= BIT7;
	P3OUT |= BIT7;

	PM5CTL0 &= ~LOCKLPM5;

	//uint32_t data_frame = 0x123456;

	while(1){
		// SPI_transmit(data_frame);
		// __bis_SR_register(LPM0_bits | GIE); // Enter Low Power Mode 0 with interrupts enabled
	}
	
	return 0;
}

void SPI_DAC_INIT(void){
	// Configure SPI pins
		P1SEL0 |=  BIT2 | BIT1;  //SIMO (P1.2), SCLK (P1.1)
		P1SEL1 &= ~(BIT2 | BIT1);

    // Configure CS pin
		P1DIR |= DAC_CS;   // Set CS pin (P1.0) as output
		P1OUT |= DAC_CS;   // Set CS pin high initially

    // Configure SPI
		UCB0CTLW0 |= UCSWRST;  // Put eUSCI_B in reset
		UCB0CTLW0 |= UCSYNC | UCCKPL | UCMSB | UCMST | UCSSEL__SMCLK;  // 3-pin, 8-bit SPI master
		UCB0BRW = 10;        // Set SPI clock to SMCLK / 1
		UCB0CTLW0 &= ~UCSWRST; // Release eUSCI_B from reset
}

void SPI_transmit(uint32_t data) {
    P1OUT &= ~DAC_CS;     // Select the DAC
    UCB0TXBUF = (data >> 16) & 0xFF; // Send MSB first
}

#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void){
	switch(__even_in_range(UCB0IV, USCI_SPI_UCTXIFG)){
		case USCI_NONE: break;             // Vector 0: No interrupts
        case USCI_SPI_UCTXIFG:             // Vector 2: TXIFG
            P1OUT |= DAC_CS;               // Deselect the DAC
            __bic_SR_register_on_exit(LPM0_bits); // Exit Low Power Mode 0
            break;
        default: break;
	}
}
