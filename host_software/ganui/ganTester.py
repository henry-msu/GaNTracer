# This Python file uses the following encoding: utf-8

import pyftdi.spi
from time import sleep

class ganTester:
    def __init__(self, ftdiURL, cs, freq, mode):
        self.VdMin = 0
        self.VdMax = 0
        self.VdStep = 0
        self.tempReads = 0
        self.tempCalValue = 0
        self.spi = pyftdi.spi.SpiController()
        self.spi.configure(ftdiURL)
        self.slave = self.spi.get_port(cs=cs, freq=freq, mode=mode)
        self.gpio = self.spi.get_gpio()

    def readGPIOs(self): # read all gpio pins (for our setup it's only 3)
        return (self.gpio.read() & 0b1110000) >> 4 # get only the bits we want, and shift everything into place

    def readGPIO(self, num): # read single gpio pin, this is specific to our setup
        return ((self.gpio.read() & (1 << num+4)) >> num+4)

    def startTest(self):
        print("Starting test with parameters:")
        print(f"VdMin: {self.VdMin}")
        print(f"VdMax: {self.VdMax}")
        print(f"VdStep: {self.VdStep}")
        print(f"Temp reads: {self.tempReads}")
        print("")

        numBytes = 2*self.tempReads # how many bytes for the desired temperature readings
        triggerCollectionCMD = [0x0A, numBytes >> 8, numBytes & 0xFF] # command to send to MSP to tell it to start reading temperature
        self.slave.write(triggerCollectionCMD)

        while(self.readGPIO(0) == 0): # wait for msp to signal that it's finished
            continue

        temperatureRaw = list(self.slave.read(numBytes)); # read data back once MSP is done

        temperatureNums = []; # convert each byte into 16 bit numbers representing temperature
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

        calTempsC = [round(x - self.tempCalValue, 2) for x in tempsC]
        calTempsF = [round(x * (9/5) + 32, 2) for x in calTempsC]
