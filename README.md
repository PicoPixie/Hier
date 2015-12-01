# Hier

Geofence w/SMS alerts - Mediatek LinkIt ONE

This project uses the onboard GPS and GPRS to publish it's current location over a PubNub channel.
A webscript subscribes to this PubNub channel and displays a Google Map with map markers visually indicating the 
current known location of :

(1) The LinkIt ONE(s) 
(2) The browser [if supporting geolocation]

Uses a hard-coded geofence value, if the difference between the two is greater than this geofence the webserver 
publishes a message across a "private" channel that only the receiving unit subscribes to, notifying the breach 
which triggers an SMS alert to a predefined cell number.

Limits SMS on repeated breach to at most once/5 minutes.  
