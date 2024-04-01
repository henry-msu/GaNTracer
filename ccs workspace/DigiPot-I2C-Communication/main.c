#include <msp430.h> 

int Data_Cnt = 0; 
char Packet[] = {0x03, 0x33, 0x44, 0x55};


int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	UCB1CTLW0 |= UCSWRST; 

	UCB1CTLW0 |= (UCSSEL_3 | UCMODE_3 | UCMST | UCTR); 
	UCB1BRW = 10; 
	UCB1I2CSA = 0x58;
	UCB1CTLW1 |= UCASTP_2; 
	UCB1TBCNT = sizeof(Packet); 

	P4SEL1 &= ~BIT6;
	P4SEL0 |= BIT6; 

	P4SEL1 &= ~BIT7;
	P4SEL0 |= BIT7; 

	PM5CTL0 &= ~LOCKLPM5; 

	UCB1CTLW0 &= ~UCSWRST; 

	UCB1IE |= UCTXIE0; 
	__enable_interrupt(); 

	int i; 
	while(1){
		UCB1CTLW0 |= UCTXSTT; 
		for(i=0; i<100; i++){}
	}
	return 0;
}

#pragma vector=EUSCI_B1_VECTOR
__interrupt void EUSCI_B1_ISR(void){
	if (Data_Cnt == (sizeof(Packet)-1)){
		UCB1TXBUF = Packet[Data_Cnt]; 
		Data_Cnt = 0; 
	} else {
		UCB1TXBUF = Packet[Data_Cnt]; 
		Data_Cnt++;
	}
}
