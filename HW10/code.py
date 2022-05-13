from ulab import numpy as np # to get access to ulab numpy functions
import time
# Declare an array
a = np.zeros(1024)
# Fill in array with 3 sine waves
for x in range(1024):
    frequency1 = 3;
    frequency2 = 7;
    frequency3 = 20;
    a[x] = np.sin(2*np.pi*frequency1*x/1000) + np.sin(2*np.pi*frequency2*x/1000) + np.sin(2*np.pi*frequency3*x/1000)
fftArray = np.fft.fft(a)
fftArray = fftArray[0] # Get the real part of the fft
for x in range(1024/16): # Plot first half of data every 8 points
    print("("+str(fftArray[x*8])+",)") # print with plotting format
    time.sleep(.05) # delay in seconds

time.sleep(2) # delay in seconds

for x in range(1024/16): # Plot second half of data every 8 points
    print("("+str(fftArray[512 + x*8])+",)") # print with plotting format
    time.sleep(.05) # delay in seconds

