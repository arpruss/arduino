## Build ##

This is designed for NodeMCU v2/v3 which is based on an ESP8266-12E.
The only other thing you need to buy are 10 LEDs and 10 resistors.

Connect an LED with an appropriate current-limiting resistor to 
GPIOs 1, 3, 15, 13, 12, 14, 2, 4, 5, 16, with the flat side of the LED
to the pin. The first four are the hours and the last six are the minutes.

This lets the pins sink the current; the ESP8266 can handle
more sink than source current; if you prefer to source the current, 
comment out #define CONNECTION_SINK in clock.h.

For configuration, you need a button hooked up to GPIO 0, pressed = low.
(The NodeMCU v3 has that button labeled "Flash".)


## Operation ##

Setup:

* Power on.
* During the in-and-out animation (you'll see what I mean), press the GPIO 0 button
* Display will show the device's IP address as follows. The two high bits show the current 
octet number (0, 1, 2, 3) and the other eight bits show the value of the octet. It's probably
192.168.4.1 in case you don't want to do binary-to-decimal conversion.
* Connect to the 'ESP-CLOCK' SSID.
* Point a browser to the device IP.
* Configure Wifi credentials and time/ntp attributes.
* After submitting, the clock connects to the given Wifi network and starts.

During normal operation:

* Press the button to display the device IP address (same format as before)
* Browse to that address to view the status and change config.

## Extras ##

You can also use this device as a web-controlled 10 LED panel. To do that, use:

    http://Device.ip.address/io?op=OPERATION&value=VALUE

The possible OPERATION values are:
* clock: switch back to clock mode
* set: set all and only the LEDs indicated by the VALUE (0-1023, 0-0x3FF, 0-0b1111111111)
* or: active the LEDs indicated by the VALUE, which is now a mask (dec/hex/bin)
* andnot: deactivate the LEDs indicated by the VALUE, which is a mask (dec/hex/bin)
* animate: this activates an animation sequence set in the VALUE. The animation sequence is given
    by a sequence of the form ReturnToClockWhenDone:FrameTime:NumberOfRepeats:Data
    
    - ReturnToClockWhenDone is 0 to stay in LED mode and 1 to go back to clock mode. 
    
    - FrameTime is the time for each frame in decimal seconds. 
    
    - NumberOfRepeats says how many times the sequence is supposed to repeat. Set to -1 to go forever. 
    
    - Data is a colon-separated sequence of dec/hex/bin values.

    For instance 0:0.25:3:0x3FF:0 flashes all the LEDs three times, and stays in LED mode.

The set, or, andnot and animate (without the return-to-clock option) commands switch to LED mode.
In that mode, you can also slowly poll the GPIO 0 button--its value is returned by the device's
webserver.

## Copyright ##

The design and code is Copyright 2015 Ben Buxton (bbuxton@gmail.com) and Alexander Pruss (arpruss@gmail.com).

Licenced under GPLv3.

