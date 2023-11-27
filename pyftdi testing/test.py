#!/usr/bin/env python3
import os
from pyftdi.ftdi import Ftdi
from pyftdi.spi import *

spi = SpiController() # instantiate spi device
spi.configure('ftdi://ftdi:232h/1') # use first ft232h enumerated (THIS WILL CHANGE SOON)
slave = spi.get_port(cs=0, freq=400E3, mode=0) # Set up SPI master

while(1):
    input('press enter to transmit\n')
    write_buf = [0xAA]
    print(f"writing: {hex(int.from_bytes(write_buf))}")
    read_buf = slave.exchange(write_buf, duplex=True)
    read = int.from_bytes(read_buf)
    print(f"read : {hex(read)}")

