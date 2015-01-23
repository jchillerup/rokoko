gst-launch-1.0 v4l2src device=/dev/video1 ! 'image/jpeg,width=640,height=480,framerate=15/1' ! filesink buffer-size=0 location=/dev/stdout | ./rokoko_position_streamer /dev/stdin  ../../rokoko/5x2hboard/5x2hboard.yml ../../rokoko/calibration-logitech-webcam.yml 120 192.168.0.114 rokoko-udoo1/camera

#./rokoko_position_streamer live:1 ../../rokoko/2a0board/2a0board.yml ../../rokoko/camera.yml 120
