from imu import IMU
import sys, time

if len(sys.argv) == 1:
    print("No devices defined")
    sys.exit(255)

i = IMU(sys.argv[1])
print("Connected to IMU")

print("Waiting for the IMU to wake up")
time.sleep(5)

print("Calibrating the accelerometer")
if (i.calibrate_accelerometer()):
    print(" - successful")

print("Calibrating the magnometer")
if (i.calibrate_magnometer()):
    print(" - successful")
    
name = input("Enter a new name: ").rstrip()

i.rename(name)

print("The IMU is now %s" % i.get_identifier())
