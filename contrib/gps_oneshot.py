#!/bin/env python

import gps

# Connect to gps daemon
gpsd = gps.gps(host='localhost', port='2947', mode=gps.WATCH_ENABLE | gps.WATCH_JSON | gps.WATCH_SCALED, verbose=0)
# Reads gps 10 times, if positioning found, stop reading.
for x in range(0, 9):
    gpsd.read()
    print gpsd.fix
    if ((gpsd.fix.latitude != 0.0) and (gpsd.fix.longitude != 0.0) and (gpsd.fix.altitude != "nan")):
        break
    gpsd.next()
print (gpsd.fix.latitude, gpsd.fix.longitude, gpsd.fix.altitude, gpsd.fix.time)
print gpsd
