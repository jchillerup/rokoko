# robroker
This is a message broker written in nanomsg. It's meant to provide a pub/sub interface to all of the data flowing from a ROKOKO suit to the downstream systems that use the data. Used in conjunction with Avahi it turns the suit into a plug and play device.

The code often refers to upstream and downstream. We define upstream as the source of the new, fresh data, i.e. the sensors, and downstream as the layers of the system that uses the sensor data for something, i.e. Unity3d (in most cases).