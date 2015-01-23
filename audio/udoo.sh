gst-launch-0.10 alsasrc ! audioconvert ! audioresample ! opusenc ! rtpopuspay ! udpsink host=127.0.0.1 port=14000 sync=true
