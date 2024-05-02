/*
 * common.h
 *
 *  Created on: Mar 27, 2024
 *      Author: henry
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <driverlib.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define CS_MCLK_DESIRED_FREQUENCY_IN_KHZ 20000 // Target frequency for MCLK in kHz
#define CS_MCLK_FLLREF_RATIO CS_MCLK_DESIRED_FREQUENCY_IN_KHZ/32.768 // MCLK/FLLRef Ratio
uint16_t status; // status of Oscillator fault flags

#define TEMPSENSOR_ADDRESS 0x4A // slave address of TEMPSENSOR
#define TEMPSENSOR_I2C EUSCI_B1_BASE // base address of eusci module being used for TEMPSENSOR

#define PIN_FINISHED_READING_TEMP GPIO_PIN0 // pin used to signal finished reading a batch of temperature

#define TB0_1MS 11 // ~11 TB0 ticks per 1ms, relies on clock source being 32768 kHz

void I2CMTXBytes(uint16_t dev, uint8_t** data_ptr, uint8_t* data, uint16_t n);
void I2CMRXBytes(uint16_t dev, uint8_t** data_ptr, uint8_t* data, uint16_t n);
void UARTtxByte(uint8_t data);
void UARTtxBytes(uint8_t* data, uint32_t n);
void delayms(uint16_t ms);
void initSPI(void);
void initUART(void);
void initI2C(void);
void initCS(void);
void initTB0(void);
void init(void);

#endif /* COMMON_H_ */
