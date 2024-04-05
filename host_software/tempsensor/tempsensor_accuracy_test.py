#!/usr/bin/env python3

# Testing the calibrated sensor for accuracy

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
import csv

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
    calValue = float(sys.argv[2]) # calibration value
    testsDone = 0 # how many tests have been completed

    # how many rounds of tests to do
    while(1):
        try:
            tests = int(input(f'Enter how many rounds of accuracy testing to conduct: '))
        except ValueError:
            print("Enter an integer number of tests")
            continue
        else:
            break

    readings = 1; # only 1 reading per test
    numBytes = 2*readings # each temperature reading takes 2 bytes


    with open(filename, mode='w', newline='') as csvfile:

        # set up csv file
        csvwriter = csv.writer(csvfile)
        headers = ["rawData", "unCaltempC", "unCaltempF", "calTempC", "calTempF", "actualTempC", "tempErrorC"]
        csvwriter.writerow(headers)

        while(testsDone < tests):
            print(f"\nConducting test {testsDone + 1}")

            # get actual temperature from user
            while(1):
                try:
                    actualTempC = float(input(f'Enter the current temperature in celcius: '))
                except ValueError:
                    print("Enter the current temperature in degrees celcius")
                    continue
                else:
                    break

            # tell MSP to make temperature readings
            triggerCollectionCMD = [0x0A, numBytes >> 8, numBytes & 0xFF]
            #print("\ntriggerCollectionCMD: " + ', '.join(['0x{:02X}'.format(i) for i in triggerCollectionCMD]) + '\n')
            slave.write(triggerCollectionCMD)

            # Wait for MSP to signal it's finished reading
            while(readGPIO(0) == 0):
                continue

            # Once MSP is finished, read out it's data
            temperatureRaw = list(slave.read(numBytes));

            tempNum = 0
            # Convert 8 bit data read into 16 bit numbers representing the temperature
            for i in range(0, len(temperatureRaw) - 1, 2):
                tempNum = temperatureRaw[i] << 8
                i += 1
                tempNum += temperatureRaw[i]

            # convert temperature numbers into actual temperatures using formula
            # from STS3x-DIS datasheet
            tempC = round(-45 + 175 * (tempNum/ ((2**16) - 1)), 2)
            tempF = round(-49 + 315 * (tempNum/ ((2**16) - 1)), 2)

            # use calibration provided by user to calibrate temperature
            calTempC = round(tempC - calValue, 2)
            calTempF = round(calTempC * (9/5) + 32, 2)

            # calculate the difference between the calibrated temperature read by the sensor and the actual provided by the user
            tempErrorC = round(calTempC - actualTempC,2)

            # print results to screen
            print(f"Temperature reported by user: {actualTempC} °C")
            print(f"Calibrated measured temperature: {calTempC} °C ({calTempF} °F)")
            print(f"Difference between measured and actual: {tempErrorC} °C")

            # print results to file
            rows = [tempNum, tempC, tempF, calTempC, calTempF, actualTempC, tempErrorC] # convert each column to proper rows
            csvwriter.writerow(rows)

            testsDone += 1

        spi.close()

    print(f'Results recorded in "{filename}"')

if __name__ == '__main__':
    main()
