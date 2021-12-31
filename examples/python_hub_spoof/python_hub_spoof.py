#!/usr/bin/env python3
"""Concept proof of hub spoofing to easily debug the code implementing
a sensor on the microcontroller.

Here, the tested sensor is the Color & Distance one.

The code uses the `my_own_bricks` module.
"""
# Custom imports
from my_own_bricks.high_level_communication import get_device_messages
from my_own_bricks.commons import COLOR_MAP

# Reverse color map (names as keys, codes as values)
R_COLOR_MAP = {v: k for k, v in COLOR_MAP.items()}


def main():
    """Send & receive data from sensor (Color & Distance Sensor here)"""
    messages = get_device_messages()
    msg = next(messages)  # Prime the coroutine
    while True:
        if msg:
            # Get and parse data yelled from device
            parse_response(*msg)

        # Send command to coroutine
        # This code is blocking until a response is received or the device is
        # ready to accept a new query. (See doc)
        # Set RGB LED color (mode 5, color blue (code 3))
        msg = messages.send((5, R_COLOR_MAP["BLUE"]))
        # Get RGB data
        # msg = messages.send(6)


def parse_response(header, response):
    """Parse responses for read-only modes 0,1,3,4,6,8

    .. note:: Write modes don't return any data.
    """
    if header == 0xC0:  # Mode 0
        print("\t\tLED:", COLOR_MAP.get(response[0], "UKN"))
    if header == 0xC1:  # Mode 1
        print("\t\tPROX:", response[0])
    if header == 0xC2:  # Mode 2
        print("\t\tCOUNT:", response[0])
    if header == 0xC3:  # Mode 3
        print("\t\tREFLT:", response[0])
    if header == 0xC4:  # Mode 4
        print("\t\tAMBI:", response[0])
    if header == 0xDE:  # Mode 6
        red = (response[1] << 8) | response[0]
        green = (response[3] << 8) | response[2]
        blue = (response[5] << 8) | response[4]
        print("R:", red, "G:", green, "B:", blue)
    if header == 0xD0:  # Mode 8
        print("\t\tCOLOR:", COLOR_MAP[response[0]])
        print("\t\tPROX:", response[1])
        print("\t\tLED:", COLOR_MAP.get(response[2], "UKN"))
        print("\t\tREFLT:", response[3])


if __name__ == "__main__":
    maint()
