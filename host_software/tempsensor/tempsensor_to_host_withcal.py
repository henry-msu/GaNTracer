#!/usr/bin/env python3

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
import signal
import sys
import csv
import time
import datetime

def sigint_handler(sig, frame):
    print()
    sys.exit(0)

def readGPIOs(): # read all gpio pins (for our setup it's only 3)
    return (gpio.read() & 0b1110000) >> 4 # get only the bits we want, and shift everything into place

def readGPIO(num): # read single gpio pin, this is specific to our setup
    return ((gpio.read() & (1 << num+4)) >> num+4)

maxReadings = 4096# depends on MSP firmware
testsDone = 0;
signal.signal(signal.SIGINT, sigint_handler) # catch SIGINT

filename = sys.argv[1] # results output file

calValue = float(sys.argv[2]) # calibration value

tic = 0;
toc = 0;
processingtime = 0; # how long the FT232H and MSP have spent doing things

# how many tests of x readings to do
while(1):
    try:
        tests = int(input(f'Enter a number of tests to conduct: '))
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

spi = SpiController() # instantiate spi device
spi.configure('ftdi://ftdi:232h/1') # use first FT232H enumerated (THIS WILL CHANGE SOON)
slave = spi.get_port(cs=0, freq=1.5E6, mode=0) # Set up SPI master
gpio = spi.get_gpio() # get gpio port from FT232H

with open(filename, mode='w', newline='') as csvfile:

    # set up csv file
    csvwriter = csv.writer(csvfile)
    headers = ["rawData", "unCaltempC", "unCaltempF", "calTempC", "calTempF"]
    csvwriter.writerow(headers)

    while(testsDone < tests):
        print(f"Conducting test {testsDone + 1} of {tests}. Processing time is ~{str(datetime.timedelta(seconds = processingtime))} (~{processingtime:.04f}s)")

        # tell MSP to make temperature readings
        triggerCollectionCMD = [0x0A, numBytes >> 8, numBytes & 0xFF]
        #print("\ntriggerCollectionCMD: " + ', '.join(['0x{:02X}'.format(i) for i in triggerCollectionCMD]) + '\n')
        tic = time.perf_counter()
        slave.write(triggerCollectionCMD)

        # Wait for MSP to signal it's finished reading
        while(readGPIO(0) == 0):
            continue

        # Once MSP is finished, read out it's data
        temperatureRaw = list(slave.read(numBytes));
        toc = time.perf_counter()
        processingtime += toc-tic

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

        calTempsC = [round(x - calValue, 2) for x in tempsC]
        calTempsF = [round(x * (9/5) + 32, 2) for x in calTempsC]

        # print results to file
        rows = zip(temperatureNums, tempsC, tempsF, calTempsC, calTempsF) # convert each column to proper rows
        csvwriter.writerows(rows)

        # print results to screen
        # print("\nRaw temperature data: " + ', '.join(['0x{:04X}'.format(i) for i in temperatureNums]))
        # print("\nTemperatures in C: " + ', '.join(['{:.2f}'.format(i) for i in tempsC]))
        # print("\nTemperatures in F: " + ', '.join(['{:.2f}'.format(i) for i in tempsF]))

        testsDone += 1

spi.close()

print(f'\n--------------------------------')
print(f'            RESULTS')
print(f'--------------------------------')
print(f'Tests conducted:                           {tests}')
print(f'Temperature readings per test:             {readings}')
print(f'Total number of temperature readings:      {tests*readings}')
print(f'Bytes of data read from MSP:               {numBytes*tests}')
print(f'Approximate time taken for MSP and FT232H: {processingtime:.3f} s')
print(f'Temperature readings written to "{filename}"')
