from imu import IMU

i = IMU('/dev/ttyACM3')

print("Connected to IMU")

name = input("Enter a new name: ").rstrip()

i.rename(name)

print("The IMU is now %s" % i.get_identifier())
