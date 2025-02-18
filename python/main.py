import os
import sys

dll_path = os.path.join(os.path.dirname(__file__), "src")
os.environ["PATH"] += os.pathsep + dll_path
sys.path.append(dll_path)

import serial
import time
from pylsl import StreamInlet, resolve_stream
import numpy as np

# inlet
streams = resolve_stream('type', 'EEG')
for stream in streams:
    if stream.name() == "armEMG":
        inlet = StreamInlet(stream)
        break

# REPLACE WITH YOUR PORT
ser = serial.Serial('COM10', 9600)

time.sleep(2)

while True:
    while True:
        latest_sample, timestamp = inlet.pull_sample(timeout=0.0)
        if latest_sample is None:
            break

    sample, timestamp = inlet.pull_sample(timeout=1.0)
    if sample:
        ser.write(str(sample[0]).encode() + b'\n')
        received = None
        if ser.in_waiting > 0:
            received = ser.readline().decode('utf-8', errors='ignore').strip()
        if received:
            print(f"Arduino says: {received}")