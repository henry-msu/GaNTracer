#!/usr/bin/env python3
from pyftdi.ftdi import Ftdi
from pyftdi.spi import *
from termcolor import colored, cprint
import hashlib
import random
import signal
import sys

def sigint_exit(sig, frame):
    if(exchanges == 0):
        print('\n\nNo exchanges attempted')
        sys.exit(0)
    percent = successfulexchanges/exchanges
    print(f'\n\n--------------------------------')
    print(f'            RESULTS')
    print(f'--------------------------------')
    print(f'Exchange size:        {data_size} bytes')
    print(f'Total exchanges:      {exchanges}')
    print(f'Successful exchanges: {successfulexchanges}')
    print(f'Percent success:      ', end='')
    if(percent == 1):
        cprint(f'{percent*100:.2f}%', "green")
    else:
        cprint(f'{percent*100:.2f}%', "red")
    sys.exit(0)

signal.signal(signal.SIGINT, sigint_exit) # catch SIGINT

spi = SpiController() # instantiate spi device
spi.configure('ftdi://ftdi:232h/1') # use first ft232h enumerated (THIS WILL CHANGE SOON)
slave = spi.get_port(cs=0, freq=300E3, mode=0) # Set up SPI master

data_size = 2048; # how much data to transmit/receive
exchanges = 0; # how many exchanges were attempted
successfulexchanges = 0; # how many exchanges were successful

while(1):
    data_out = random.choices(range(255),k=data_size); # generate some random 8 bit data

    input(f'Press enter to transmit {len(data_out)} bytes:')
    hashout = hash(tuple(data_out)) # calculate hash of data to transmit
    print(f'Transmitted data hash: {hashout}')
    # print('transmitted data: {}\n'.format(', '.join(hex(x) for x in data_out)))

    data_out.insert(0,0x00) # insert the "read" instruction to tell MSP to read data
    slave.write(data_out) # do data transmission

    # input('Press enter to begin data reception:')
    # for i in range(0, data_size - 1):
    input('Press enter to receive data back:')
    slave.write([0x01], start=True, stop=False) # begin read
    data_in = slave.read(data_size, start=False, stop=True); # read data
    #print('Received data: {}'.format(', '.join(hex(x) for x in data_in)))
    # input('Press enter to receive data back:')
    # data_in = slave.read(1, start=False, stop=True)
    # print('Received data: {}'.format(', '.join(hex(x) for x in data_in)))
    hashin = hash(tuple(data_in)) # calculate hash of received data
    print(f'Received data hash: {hashin}')

    exchanges += 1
    if(hashout == hashin):
        cprint('Hashes match, data exchange successful\n', "green")
        successfulexchanges += 1
    else:
        cprint('Hashes do not match, data transmit unsuccessful\n', "red")
