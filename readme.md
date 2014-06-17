
## Introduction
The motion capture suit prototype consists of 15 arduino micros with a IMU attached and a Raspberry Pi receiving the data over serial interface and forwarding it as OSC. 

## Firmware
The arduinos run the code in ./Arduino/compactImuClient

The Raspberry Pi is running an Archlinux distro. The c program in ./imu-udp-bridge-c runs on startup and connects to the sensors.  


## Configuration tools
You can use fabric with the included fabfile to configure a new ip address and update the raspberry pi firmware. You can install fabric using pip. 
    
    $ pip install fabric

### Configure ip

Insert your ip address below

    $ fab setip_all:192.168.0.1

### Upgrade firmware
	
This will get new code from git and run make.

	$ fab upgrade_all

### Reboot

	$ fab reboot_all

Check fabfile.py for addition commands.

## Calibration

The IMU's needs to be calibrated. TODO: what to run


## Other tools, legacy / WIP - not complete
./imuOscVisualizer is a somewhat incomplete openFrameworks tool for visualizing the OSC data. We primarily use Unity for the visualization now.

./imuTestVisualizer is an openframeWorks skeleton animation test connecting directly to the sensors. 

./Aruco WIP of a positioning system using the AR aruco library, a marker carpet and a camera on the knee.

./camcalib is for making opencv instrinsics for the camera







