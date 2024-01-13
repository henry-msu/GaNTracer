#!/usr/bin/env python3
from pyftdi.ftdi import Ftdi
from pyftdi.spi import *
from termcolor import colored, cprint
import time
import hashlib
import random
import signal
import sys

def print_results():
    if(exchanges == 0):
        print('\n\nNo exchanges attempted')
        sys.exit(0)
    percent = successfulexchanges/exchanges
    print(f'\n--------------------------------')
    print(f'            RESULTS')
    print(f'--------------------------------')
    print(f'Exchange size:         {data_size} bytes')
    print(f'Total exchanges:       {exchanges}')
    print(f'Successful exchanges:  {successfulexchanges}')
    print(f'Percent success:       ', end='')
    if(percent == 1):
        cprint(f'{percent*100:.2f}%', "green")
    else:
        cprint(f'{percent*100:.2f}%', "red")
    print(f'Total data exchanged:  {2*data_size*exchanges} bytes')
    print(f'Total time taken:      {total_time:.4f} s')
    print(f'Data rate (kilobytes): {((2*data_size*exchanges)/total_time)/1000:.4f} kB/s')
    print(f'Data rate (kilobits):  {((2*data_size*exchanges)/total_time):.4f} kb/s')
    sys.exit(0)

signal.signal(signal.SIGINT, print_results) # catch SIGINT

spi = SpiController() # instantiate spi device
spi.configure('ftdi://ftdi:232h/1') # use first ft232h enumerated (THIS WILL CHANGE SOON)
slave = spi.get_port(cs=0, freq=1.5E6, mode=0) # Set up SPI master

successfulexchanges = 0; # how many exchanges were successful
total_time = 0; # time for each exchange
data_size = 2048; # how much data to transmit/receive in bytes. This is hardcoded in the MSP firmware so cannot ask user
tic = 0;
toc = 0;

# read how many exchanges from user
while(1):
    try:
        exchanges = int(input('Enter a number of exchanges to attempt: '))
        break
    except:
        print("Enter a NUMBER of exchanges")

# do the transmission
for i in range(0, exchanges):
    data_out = random.choices(range(255),k=data_size); # generate some random 8 bit data

    hashout = hash(tuple(data_out)) # calculate hash of data to transmit
    print(f'Transmitted data hash: {hashout}')

    data_out.insert(0,0x00) # insert the "read" instruction to tell MSP to read data
    tic = time.perf_counter()
    slave.write(data_out) # do data transmission

    slave.write([0x01], start=True, stop=False) # begin read
    data_in = slave.read(data_size, start=False, stop=True); # read data
    toc = time.perf_counter()
    total_time += toc-tic
    hashin = hash(tuple(data_in)) # calculate hash of received data
    print(f'Received data hash:    {hashin}\n')

    if(hashout == hashin):
        successfulexchanges += 1
    else:
        cprint('Hashes do not match, data transmit unsuccessful', "red")

print_results()
