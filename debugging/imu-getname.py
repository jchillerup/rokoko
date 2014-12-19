from imu import IMU

i = IMU('/dev/ttyACM3')

print("Connected to IMU")
print("Identifier %s" % i.get_identifier())
