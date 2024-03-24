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

#define DAC_CS BIT0 // Define DAC chip select pin as BIT0 (P1.0)

//Define DAC Commands
#define Com_Write2InRegN            0x00
#define Com_UpdateDacReg            0x01
#define Com_Write2InRegN_SW_LDAC    0x02
#define Com_WriteUpdateDacRegN      0x03
#define Com_PowerDownDac            0x04
#define Com_ResetDac                0x05
#define Com_LDAC_SetReg             0x06
#define Com_DAC_intRef              0x07

//Define DAC Address
#define DAC_A_addr                  0x00
#define DAC_B_addr                  0x01
#define DAC_All_addr                0x07

// #define DATA_LENGTH 10

void SPI_Init(); 
void DAC_Tx(); 

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	SPI_Init(); 

	//DCSweep Ilim - Enable PWM controller
	P3DIR |= BIT7;
	P3OUT |= BIT7;

	//Unlock LPM5
	PM5CTL0 &= ~LOCKLPM5;

	//uint8_t data_to_send[DATA_LENGTH] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};

	int i;

	while(1){
		 // Send each byte in the data array
    	for (i = 0; i < DATA_LENGTH; i++) {
        	DAC_Tx(command, address, data)
    	}
	__delay_cycles(1000);
	}
	return 0;
}

// SPI-Initialize the SPI module for communication with the DAC
void SPI_Init(void){
	// Configure GPIOs for SPI mode
    	P1SEL0 |= BIT1 | BIT2 | BIT3 | BIT0; // Set pins for SPI: P1.1 (CLK), P1.2 (MOSI), P1.3 (MISO), P1.0 (CS)
    	P1SEL1 &= ~(BIT1 | BIT2 | BIT3 | BIT0);

    // Configure eUSCI_B0 for SPI
		UCB0CTLW0 |= UCSWRST;               // Put eUSCI_B0 in reset
		UCB0CTLW0 |= UCSYNC | UCCKPH | UCMSB | UCMST | UCSSEL__SMCLK;  // 4-wire SPI, Clock phase = 1, MSB first, Master mode, SMCLK as clock source
		UCB0BRW = 0x0A;                     // Set SPI baud rate (frequencies)
		UCB0CTLW0 &= ~UCSWRST;              // Release eUSCI_B0 for operation
}

void DAC_Tx(uint16_t command, uint16_t address,  uint16_t data){
	//---Data Frame:-------------------------------------------------------------
	//| 23 | 22 | 21 | 20 | 19 | 18 | 17 | 16 | 15 | 14 | 13 | 12 | 11 | 10 | 09 | 08 | 07 | 06 | 05 | 04 | 03 | 02 | 01 | 01
	//----------------------------------------------------------------
	//| -  | -  | C2 | C1 | C0 | A2 | A1 | A0 | D11| D10| D9 | D8 | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | -  | -  | -  | -  |
	//---------------------------------------------------------------------------

	//Create a data frame that contains command, address, and data in proper spots
	uint32_t data_frame = ((command & 0x07) << 20 | (address & 0x07) << 18 | (data & 0x0FFF) << 4);

	//Send 3 8bit SPI transmissions to the DAC containing the reduced data frame bits from DB23 - DB0
	// Transmit the data frame
    P1OUT &= ~DAC_CS;     // Select the DAC
    UCB0TXBUF = (data_frame >> 16) & 0xFF; // Send MSB first
    while (!(UCB0IFG & UCTXIFG)); // Wait for TX buffer to be ready
    UCB0TXBUF = (data_frame >> 8) & 0xFF;
    while (!(UCB0IFG & UCTXIFG)); // Wait for TX buffer to be ready
    UCB0TXBUF = data_frame & 0xFF;
    while (!(UCB0IFG & UCTXIFG)); // Wait for TX buffer to be ready
    P1OUT |= DAC_CS; // Deselect the DAC
	
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
