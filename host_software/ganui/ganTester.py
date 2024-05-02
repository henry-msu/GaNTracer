# This Python file uses the following encoding: utf-8

import pyftdi.spi
import time
import math
import random
import numpy as np

class ganTester:
    def __init__(self, ftdiURL, cs, freq, mode):
        self.VdMin = 0
        self.VdMax = 0
        self.VdStep = 0
        self.VgMin = 0
        self.VgMax = 0
        self.VgStep = 0
        self.tempReads = 0
        self.tempCalValue = 0
        self.spi = pyftdi.spi.SpiController()
        self.spi.configure(ftdiURL)
        self.slave = self.spi.get_port(cs=cs, freq=freq, mode=mode)
        self.gpio = self.spi.get_gpio()
        self.tempsC = [];
        self.tempsF = [];
        self.VgValues = [];
        self.VdValues = [];
        self.VdsMeasured = None;
        self.IdMeasured = None;

    def readGPIOs(self): # read all gpio pins (for our setup it's only 3)
        return (self.gpio.read() & 0b1110000) >> 4 # get only the bits we want, and shift everything into place

    def readGPIO(self, num): # read single gpio pin, this is specific to our setup
        return ((self.gpio.read() & (1 << num+4)) >> num+4)

    def startTest(self):
        print("Starting test with parameters:")
        print(f"VdMin: {self.VdMin}")
        print(f"VdMax: {self.VdMax}")
        print(f"VdStep: {self.VdStep}")
        print(f"VgMin: {self.VgMin}")
        print(f"VgMax: {self.VgMax}")
        print(f"VgStep: {self.VgStep}")
        print(f"Temp reads: {self.tempReads}")
        print("")

        # set up measurement values
        self.VgValues = np.round(np.arange(self.VgMin, self.VgMax+(self.VgStep/4), self.VgStep), 3) # generate list of Vgs values to test
        self.VdValues = np.round(np.arange(self.VdMin, self.VdMax+(self.VdStep/4), self.VdStep), 3) # generate list of Vds values to test
        self.VdsMeasured = np.empty((0,len(self.VdValues)))
        self.IdMeasured = np.empty((0,len(self.VdValues)))


        numBytes = 2*self.tempReads # how many bytes for the desired temperature readings
        triggerCollectionCMD = [0x0A, numBytes >> 8, numBytes & 0xFF] # command to send to MSP to tell it to start reading temperature
        self.slave.write(triggerCollectionCMD)

        while(self.readGPIO(0) == 0): # wait for msp to signal that it's finished
            continue
        print(f"test compltete at: {time.time_ns()} ns")
        temperatureRaw = list(self.slave.read(numBytes)); # read data back once MSP is done

        temperatureNums = []; # convert each byte into 16 bit numbers representing temperature
        for i in range(0, len(temperatureRaw) - 1, 2):
            tempNum = temperatureRaw[i] << 8
            i += 1
            tempNum += temperatureRaw[i]
            temperatureNums.append(tempNum)

        # convert temperature numbers into actual temperatures using formula
        # from STS3x-DIS datasheet
        unCalTempsC = [];
        unCalTempsF = [];
        for tempNum in temperatureNums:
            unCalTempsC.append(round(-45 + 175 * (tempNum/ ((2**16) - 1)), 2))
            unCalTempsF.append(round(-49 + 315 * (tempNum/ ((2**16) - 1)), 2))

        self.tempsC = [round(x - self.tempCalValue, 2) for x in unCalTempsC]
        self.tempsF= [round(x * (9/5) + 32, 2) for x in self.tempsC]

        # actual measurement data would go here
        for i in range(0, len(self.VgValues)):
            self.VdsMeasured = np.vstack((self.VdsMeasured, np.round(self.VdValues, 3)))
            self.IdMeasured = np.vstack((self.IdMeasured, np.round(self.VgValues[i]+self.VdValues*(i+1), 3)))

