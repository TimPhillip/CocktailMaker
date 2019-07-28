import numpy as np
import os
import re
from matplotlib import pyplot as plt

def list_measurement_files():
    files = os.listdir("/Volumes/TIMS USB/")

    regex = re.compile("WaveData(?P<number>[0-9]*)\.csv")
    files = filter(regex.match, files)

    matches = map(regex.match, files)
    matches = map( lambda m: (os.path.join("/Volumes/TIMS USB/" ,m.string), m.groupdict()['number']), matches )

    return matches

def data_point_generator(files):

    voltage_measurement = None
    frequency_measurement = None

    for file, index in files:
        index = int(index)

        if index % 2 == 0 and (voltage_measurement is None):
            voltage_measurement = measure_voltage(file)

        if index % 2 == 1 and (frequency_measurement is None):
            frequency_measurement = measure_frequency(file)

        if not (voltage_measurement is None or frequency_measurement is None):
            yield (voltage_measurement, frequency_measurement)
            voltage_measurement = None
            frequency_measurement = None

def measure_voltage(file):
    data = np.loadtxt(file, delimiter= ',', skiprows= 3)
    return np.mean(data, axis= 0)[-1]

def measure_frequency(file):
    data = np.loadtxt(file, delimiter=',', skiprows=3)

    high = data[:,1] > 1.0

    low = data[:,1] <= 1.0
    diff = np.asarray(high[:-1] , np.int32) - np.asarray(high[1:] , np.int32)

    tRise = np.extract( diff < 0.0 , data[:,0])
    tFall = np.extract( diff > 0.0, data[:,0])

    try:
        if data[0,1] < 1.0:
            return 1. / (2*(tFall[0] - tRise[0]))
        else:
            return 1. / (2*(tRise[0] - tFall[0]))
    except:
        return np.nan

if __name__ == "__main__":
    X = list(data_point_generator(list_measurement_files()))
    X = np.asarray(X)

    plt.scatter(X[:,1], X[:,0], marker= 'x')
    plt.xlabel('f [Hz]')
    plt.ylabel('U [V]')
    plt.show()