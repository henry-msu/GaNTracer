#!/usr/bin/env python3
import sys
import serial
import signal

ser = serial.Serial(sys.argv[1], sys.argv[2])

listoferrors = [];
errors = 0;
success = 0;
attempts = 0;
def signal_handler(sig, frame):
    ser.close()
    print(f"\nreadings attempted: {attempts}\n")
    print(f"readings below 20000 (probably an error at room temperature): {errors}\n")
    print(f"Success rate: {(success/attempts * 100)}\n")
    print(listoferrors);
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)


while(True):
    line = ser.readline().strip()
    sensor = int.from_bytes(line, "big")

    attempts = attempts + 1
    success = success + 1;
    if (sensor < 10000):
        listoferrors.append(sensor);
        success = success - 1;
        errors = errors + 1

    # conversion foruma taken from STS3x datasheet
    tempC = round(-45 + 175 * (sensor / ((2**16) - 1)), 2)
    tempF = round(-49 + 315 * (sensor / ((2**16) - 1)), 2)

    # print results
    print(f"Input: {sensor} ({sensor:#0{6}x})")
    print(f"Temp: {tempC:.2f} °C ({tempF:.2f} °F)\n")

ser.close()
# conversion foruma taken from STS3x datasheet
# tempC = round(-45 + 175 * (sensor / ((2**16) - 1)), 2)
# tempF = round(-49 + 315 * (sensor / ((2**16) - 1)), 2)

# print results
# print(f"Input: {sensor} ({sensor:#0{6}x})")
# print(f"Temp: {tempC} °C ({tempF} °F)")
