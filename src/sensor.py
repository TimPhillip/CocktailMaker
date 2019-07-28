from matplotlib import pyplot as plt
import numpy as np
import serial



s = serial.Serial(port= '/dev/tty.usbmodem14101', baudrate= 100000)

xdata = []
ydata = []

for i in range(1000):
    raw = s.readline()
    value = float(raw)
    if 1000 < value < 1300:
        xdata.append(i)
        ydata.append(value)

# add this if you don't want the window to disappear at the end
plt.plot(xdata, ydata)
plt.axhline(np.mean(ydata), c= 'red')
plt.show()




