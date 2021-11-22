#!/usr/bin/env python

import matplotlib.pyplot as plt
import pandas as pd

df = pd.read_csv('wireshark_capture.csv')
print(df.head())

prev_time = df['Time'].min()
bytes_transfered = 0
throughputs = []
times = []

for i in range(len(df)):
    if (df['Time'][i] - prev_time > 2):
        times.append(df['Time'][i])
        throughputs.append(bytes_transfered/2)
        bytes_transfered = df['TCP Segment Len'][i]
        prev_time = df['Time'][i]
    else:
        bytes_transfered += df['TCP Segment Len'][i]

if prev_time != df['Time'][len(df) - 1]:
    throughputs.append(bytes_transfered/(df['Time'][len(df) - 1] - prev_time))
    times.append(df['Time'][len(df) - 1])

print(times)

plt.title('plot of Throughput vs time')
plt.step(times, throughputs)
plt.xlabel('Time in seconds')
plt.ylabel('Throughput')
plt.show()

