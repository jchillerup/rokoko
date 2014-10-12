import os, sys, glob, time

recipient = ""
hostname = ""
active = set()

def get_new_devices():
    nodes = set(glob.glob('/dev/ttyACM*'))
    to_enable = nodes.difference(active)
    return set(to_enable)

def run(devices):
    commandline = "screen -d -m ./main %s /%s %s" % (recipient, hostname, ' '.join(devices))
    print(commandline)
    os.system(commandline)

if __name__ == '__main__':
    recipient = sys.argv[1]
    hostname = sys.argv[2]
    
    while True:
        time.sleep(1)

        # Find any new devices
        to_enable = get_new_devices()
        
        if len(to_enable) > 0:
            print("Found new devices, waiting for a bit them all to come up")
            time.sleep(3)

            # Check if there might be even more new devices in the pipeline
            to_enable = get_new_devices()
            
            # Run the thing
            run(to_enable)

            # Update the list of active devices
            for node in to_enable:
                active.add(node)
        else:
            print("Nothing to do %s" % time.time(), end="\r")
