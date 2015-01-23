export AUDIO_CAPS="application/x-rtp,media=(string)audio,clock-rate=(int)48000,encoding-name=(string)X-GST-OPUS-DRAFT-SPITTKA-00"
gst-launch-0.10 udpsrc port=14000 caps=$AUDIO_CAPS ! gstrtpjitterbuffer latency=25 ! rtpopusdepay ! opusdec plc=true ! alsasink
