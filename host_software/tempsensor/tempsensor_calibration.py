#!/usr/bin/env python3

# Allows the user to set the offset calibration of the temperature sensor

# note about GPIOs:
# The SPI module takes up the first 4 pins, so gpios start at ADBUS4.
# This is not compensated for automatically in pyftdi. To read GPIO0,
# you actually read ADBUS4, and so on. Additionally, our board is
# only using 3 gpio pins (ADBUS4, ADBUS5, and ADBUS6). The functions
# readGPIOs() and readGPIO(num) take all this into account and handle
# everything properly.

from pyftdi.ftdi import Ftdi
from pyftdi.spi import *
from termcolor import colored, cprint
from statistics import mean
import signal
import sys
import time
import datetime

def main():

    spi = SpiController() # instantiate spi device
    spi.configure('ftdi://ftdi:232h/1') # use first FT232H enumerated (THIS WILL CHANGE SOON)
    slave = spi.get_port(cs=0, freq=1.5E6, mode=0) # Set up SPI master
    gpio = spi.get_gpio() # get gpio port from FT232H

    def sigint_handler(sig, frame):
        print("\nSIGINT received, exiting forcefully")
        sys.exit(1)

    def readGPIOs(): # read all gpio pins (for our setup it's only 3)
        return (gpio.read() & 0b1110000) >> 4 # get only the bits we want, and shift everything into place

    def readGPIO(num): # read single gpio pin, this is specific to our setup
        return ((gpio.read() & (1 << num+4)) >> num+4)

    signal.signal(signal.SIGINT, sigint_handler) # catch SIGINT

    maxReadings = 4096# depends on MSP firmware
    filename = sys.argv[1] # results output file
    testsDone = 0 # how many tests have been completed
    offsetCals = [] # place to store all the offset values

    # how many rounds of calibration to do
    while(1):
        try:
            tests = int(input(f'Enter how many rounds of calibration to conduct: '))
        except ValueError:
            print("Enter an integer number of tests")
            continue
        else:
            break
    # read how many readings from user
    while(1):
        try:
            readings = int(input(f'Enter a number of temperature readings to take per test (max {maxReadings}): '))
        except ValueError:
            print("Enter an integer number of readings")
            continue
        if readings > maxReadings:
            print(f"You asked for too many readings, maximum is {maxReadings}")
        else:
            break
    numBytes = 2*readings # each temperature reading takes 2 bytes


    with open(filename, mode='w', newline='') as resultsfile:

        # set up calibration data file
        resultsfile.write(f"Temp sensor calibration for {datetime.datetime.now()}\n")

        while(testsDone < tests):
            print(f"Conducting test {testsDone + 1}")

            # get actual temperature from user
            while(1):
                try:
                    actualtemp = float(input(f'Enter the current temperature in celcius: '))
                except ValueError:
                    print("Enter the current temperature in degrees celcius")
                    continue
                else:
                    break

            print("Recording temperature from test device...")
            # tell MSP to make temperature readings
            triggerCollectionCMD = [0x0A, numBytes >> 8, numBytes & 0xFF]
            #print("\ntriggerCollectionCMD: " + ', '.join(['0x{:02X}'.format(i) for i in triggerCollectionCMD]) + '\n')
            slave.write(triggerCollectionCMD)

            # Wait for MSP to signal it's finished reading
            while(readGPIO(0) == 0):
                continue

            # Once MSP is finished, read out it's data
            temperatureRaw = list(slave.read(numBytes));

            # Convert 8 bit data read into 16 bit numbers representing the temperature
            temperatureNums = [];
            for i in range(0, len(temperatureRaw) - 1, 2):
                tempNum = temperatureRaw[i] << 8
                i += 1
                tempNum += temperatureRaw[i]
                temperatureNums.append(tempNum)

            # convert temperature numbers into actual temperatures using formula
            # from STS3x-DIS datasheet
            tempsC = [];
            tempsF = [];
            for tempNum in temperatureNums:
                tempsC.append(round(-45 + 175 * (tempNum/ ((2**16) - 1)), 2))
                tempsF.append(round(-49 + 315 * (tempNum/ ((2**16) - 1)), 2))

            # calculate the difference between the temperature read by the sensor and the actual provided by the user
            tempDiffsC = [x - actualtemp for x in tempsC]
            # average the difference to get the calibration
            offsetCalsingle = round(mean(tempDiffsC),2)
            offsetCals.append(offsetCalsingle)

            # print results to screen
            print(f"\nTemperature reported by user: {actualtemp}")
            print("Measured temperatures in C: " + ', '.join(['{:.2f}'.format(i) for i in tempsC]))
            print("Measured temperatures in F: " + ', '.join(['{:.2f}'.format(i) for i in tempsF]))
            print(f"Average difference between measured and actual: {offsetCalsingle}")

            testsDone += 1

        # calculate average offset, this is the value to use to calibrate the sensor
        avgOffset = round(mean(offsetCals),2)
        resultsfile.write(f"Average offset (calibration value): {avgOffset}\n")
        resultsfile.close()
        spi.close()

    print(f'\n--------------------------------')
    print(f'            RESULTS')
    print(f'--------------------------------')
    print(f'Tests conducted:               {testsDone}')
    print(f'Temperature readings per test: {readings}')
    print(f'Average temperature offset:    {avgOffset} °C')
    print(f'Calibration data written to "{filename}"')

if __name__ == '__main__':
    main()
