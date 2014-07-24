# ROKOKO
This is the software repository for the ROKOKO project. A lot of it is still in development, nothing is guaranteed to work and stuff will break all the time. Also, the current design has some pretty weird design choices (like connecting 18 individual sensors with USB cables in USB hubs) -- these are of course not final, but were made in order to make our prototyping process as fast as possible.

## License
The license still has to be determined. For now, we reserve all rights to all original works in this repository (this will change once we've had that discussion). In the meantime you're welcome to download it, build it, tinker around with it for non-commercial use.

The ArUco library is released under the BSD license.

## Introduction
ROKOKO aims to create a wireless motion capture system that tracks the motion of the actor, their position on the stage as well as their facial expression. This is going to be used for children's theatre by the [company of the same name](http://rokoko.co).

The motion capture suit prototype consists of a number of Arduino micros with an IMU attached and a Raspberry Pi receiving the data over USB-serial interface and forwarding it as OSC. The position of the actor is determined by the [ArUco](http://www.uco.es/investiga/grupos/ava/node/26) augmented reality library. The stage is looking rather trippy:

![ArUco pattern](https://raw.githubusercontent.com/jchillerup/rokoko/master/aruco/rokoko/2a0board/2a0board.png)

... but that is because the ArUco library is capable of determining the position of a camera filming such a pattern.

## Motion capture

### Firmware
The Arduinos run the code in `./Arduino/compactImuClient`. We're building the code with an Arduino distribution that makes use of GCC version 4.8.1 in order to save space. At the time of writing, the main Arduino distribution produces a binary that is too large for the Arduino Micros (it's called `arduino-avr-toolchain-nightly-gcc-4.8.1-linux64.tgz`).

The Raspberry Pi is running an Archlinux distro. The c program in ./imu-udp-bridge-c runs on startup and connects to the sensors.  

### Recipient on the actor
The Arduinos are connected to the recipient (currently a Raspberry Pi) using USB and an awful lot of hubs. The program in `./imu-udp-bridge-c/` polls the Arduinos for data and relays it over OSC to the IP specified in `recipient.txt` on UDP port 14040.

The data is received by our Unity-based renderer and applied to the relevant bones.

### Calibration
The IMUs need to be calibrated to work properly.

To start the accelerometer calibration routine send an 'a' on the serial port. Then carefully rotate the sensor around itself to max out the `x`, `y` and `z` axes in 1g.

To calibrate the magnetometer send an 'm' on the serial port and repeat as above but be extra carefull that you are not close to electronic or magnetic disturbances in the room. Step away from computers, refrigerators, radiator, any big metal objects etc. 

To set the unique identifier of a sensor send a serial 'I' followed by an ID of sixteen (16) characters, no more, no less. Pad with spaces.


## Position on the stage
TODO

## Other tools, legacy / WIP - not complete
./imuOscVisualizer is a somewhat incomplete openFrameworks tool for visualizing the OSC data. We primarily use Unity for the visualization now.

./imuTestVisualizer is an openframeWorks skeleton animation test connecting directly to the sensors. 

./Aruco WIP of a positioning system using the AR aruco library, a marker carpet and a camera on the knee.

./camcalib is for making opencv instrinsics for the camera







