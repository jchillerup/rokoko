gst-launch-1.0 v4l2src device=/dev/video1 ! 'image/jpeg,width=640,height=480,framerate=15/1' ! filesink buffer-size=0 location=/dev/stdout | ./rokoko_position_streamer /dev/stdin  ../../rokoko/2a0board/2a0board.yml ../../rokoko/camera.yml 120 10.10.10.108 camera_right

#./rokoko_position_streamer live:1 ../../rokoko/2a0board/2a0board.yml ../../rokoko/camera.yml 120
