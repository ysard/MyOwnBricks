# Build messages

You might want to create your own devices with original specifications or simulate
an official device. The problem is that the initialization sequence sequence is hard
to guess and without it your device has no chance to work.


The `forge_packets()` function in the `device_messages_builder` module of the
Python library is there for this.

Let's take the example of the unknown initialization sequence of the "Spike Force Sensor"
that is approximated by reverse engineering.
See [sources](https://github.com/ysard/MyOwnBricks/blob/spike_force/src/ForceSensor.cpp).

According to the [technical specifications](https://education.lego.com/v3/assets/blt293eea581807678a/blt23df304b05e587b2/5f8801ba721f8178f2e5e626/techspecs_technicforcesensor.pdf),
from the known initialization sequences of the other sensors, and from the code of
the PyBricks project, we can assume the following description of the modes.


There are 6 modes:

    PBIO_IODEV_MODE_PUP_FORCE_SENSOR__FORCE = 0,  // read 1x int8_t
    PBIO_IODEV_MODE_PUP_FORCE_SENSOR__TOUCHED = 1,// read 1x int8_t
    PBIO_IODEV_MODE_PUP_FORCE_SENSOR__TAPPED = 2, // read 1x int8_t
    PBIO_IODEV_MODE_PUP_FORCE_SENSOR__FRAW  = 4,  // read 1x int16_t
    PBIO_IODEV_MODE_PUP_FORCE_SENSOR__CALIB = 6,  // ??   8x int16_t

The 3rd mode is probably what the doc calls "Force-filter sensing (high speed "peak" sensing)".
It is currently not supported.

The 5th mode is probably a DEBUG mode, not supported either.

Notes:

- For absolute values, I add "Func mapping 2.0+" because advanced and recent sensors like
"color & distance sensor" (see `PROX` mode) have this flag.
Note however that this same sensor has only the `Absolute` flag for `AMBI` and `REFLT` modes.



```python
>>> data = [
    ## LUMP_MSG_TYPE_CMD messages
    (
        ("LUMP_MSG_TYPE_CMD", "LUMP_CMD_TYPE", None),
        0x3F,
    ),
    # The values `modes2` and `views2` are mandatory for PoweredUp devices.
    # Nevertheless the value `view2` is uncertain.
    (
        ("LUMP_MSG_TYPE_CMD", "LUMP_CMD_MODES", None),
        {"modes": 6, "views": 6, "modes2": 6, "views2": 6},
    ),
    (
        ("LUMP_MSG_TYPE_CMD", "LUMP_CMD_SPEED", None),
        115200,
    ),
    (
        ("LUMP_MSG_TYPE_CMD", "LUMP_CMD_VERSION", None),
        {"fw-version": (1, 0, 0, 0), "hw-version": (1, 0, 0, 0)},
    ),

    ## LUMP_MSG_TYPE_INFO messages
    ## Mode CALIB:
    # Range: 0 - 65535 as for Color Sensor / Color & Distance Sensor
    # Note that Tilt Sensor has: -45 / 45 °C
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_NAME", 6),
        "CALIB",
    ),
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_RAW", 6),
        (0, 65535),
    ),
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_PCT", 6),
        (0, 100),
    ),
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_SI", 6),
        (0, 65535),
    ),
    # N/A for Color & Distance Sensor, NULL for Color Sensor
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_UNITS", 6),
        "N/A",
    ),
    # Absolute for Color & Distance Sensor, none for Color Sensor
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_MAPPING", 6),
        {"input_flags": [], "output_flags": []},
    ),
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_FORMAT", 6),
        (8, 1, 4, 0),
    ),

    ## Mode FRAW
    # Values from 0 to 1000.
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_NAME", 4),
        "FRAW",
    ),
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_RAW", 4),
        (0, 1000),
    ),
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_PCT", 4),
        (0, 100),
    ),
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_SI", 4),
        (0, 1000),
    ),
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_UNITS", 4),
        "RAW",
    ),
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_MAPPING", 4),
        {"input_flags": ["Absolute", "Func mapping 2.0+"], "output_flags": []},
    ),
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_FORMAT", 4),
        (1, 1, 4, 0),
    ),

    ## Mode TAPPED
    # Sensor data output: 0-3:
    # - Single tap
    # - Quick tap
    # - Press and hold
    # So, its Discrete values
    # Symbol IDX is supposed
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_NAME", 2),
        "TAPPED",
    ),
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_RAW", 2),
        (0, 3),
    ),
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_PCT", 2),
        (0, 100),
    ),
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_SI", 2),
        (0, 3),
    ),
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_UNITS", 2),
        "IDX",
    ),
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_MAPPING", 2),
        {"input_flags": ["Discrete"], "output_flags": []},
    ),
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_FORMAT", 2),
        (1, 0, 3, 0),
    ),

    ## Mode TOUCHED
    # Sensor data output: 1=activated or 0=not activated
    # So, its Discrete values
    # Symbol IDX is supposed
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_NAME", 1),
        "TOUCHED",
    ),
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_RAW", 1),
        (0, 1),
    ),
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_PCT", 1),
        (0, 100),
    ),
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_SI", 1),
        (0, 1),
    ),
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_UNITS", 1),
        "IDX",
    ),
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_MAPPING", 1),
        {"input_flags": ["Discrete"], "output_flags": []},
    ),
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_FORMAT", 1),
        (1, 0, 3, 0),
    ),

    ## FORCE
    # Limited to a fixed max output value: 10 newtons
    # output resolution: 0.1 newton steps
    # So it should be values from 0 to 100, divided by 10
    # to have a resolution of 0.1.
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_NAME", 0),
        "FORCE",
    ),
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_RAW", 0),
        (0, 100),
    ),
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_PCT", 0),
        (0, 100),
    ),
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_SI", 0),
        (0, 100),
    ),
    # PCT or RAW, or force unit symbol...
    # => FOR supposed
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_UNITS", 0),
        "FOR",
    ),
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_MAPPING", 0),
        {"input_flags": ["Absolute", "Func mapping 2.0+"], "output_flags": []},
    ),
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_FORMAT", 0),
        (1, 0, 3, 0),
    ),

    ## COMBOS
    # Modes 0, 1, 4
    (
        ("LUMP_MSG_TYPE_INFO", "INFO_MODE_COMBOS", 0),
        (0, 1, 4),
    ),
]
```

Let's put that data in the builder function:

```python
>>> from my_own_bricks.device_messages_builder import forge_packets
>>> from my_own_bricks.messages import get_hex_msg
>>>
>>> code = list()
>>> for msg in forge_packets(data):
...     hex_bytes = get_hex_msg(msg)
...     size = len(msg)
...     text = f"SerialTTL.write(\"{hex_bytes}\", {size});"
...     code.append(text)
>>>
>>> print("\n".join(code))
SerialTTL.write("\x40\x3F\x80", 3);
SerialTTL.write("\x51\x05\x05\x05\x05\xAE", 6);
SerialTTL.write("\x52\x00\xC2\x01\x00\x6E", 6);
SerialTTL.write("\x5F\x00\x00\x00\x10\x00\x00\x00\x10\xA0", 10);
SerialTTL.write("\x9E\x00\x43\x41\x4C\x49\x42\x00\x00\x00\x24", 11);
SerialTTL.write("\x9E\x01\x00\x00\x00\x00\x00\xFF\x7F\x47\xA7", 11);
SerialTTL.write("\x9E\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xE9", 11);
SerialTTL.write("\x9E\x03\x00\x00\x00\x00\x00\xFF\x7F\x47\xA5", 11);
SerialTTL.write("\x96\x04\x4E\x2F\x41\x00\x4D", 7);
SerialTTL.write("\x8E\x05\x00\x00\x74", 5);
SerialTTL.write("\x96\x80\x08\x01\x04\x00\xE4", 7);
SerialTTL.write("\x94\x00\x46\x52\x41\x57\x69", 7);
SerialTTL.write("\x9C\x01\x00\x00\x00\x00\x00\x00\x7A\x44\x5C", 11);
SerialTTL.write("\x9C\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xEB", 11);
SerialTTL.write("\x9C\x03\x00\x00\x00\x00\x00\x00\x7A\x44\x5E", 11);
SerialTTL.write("\x94\x04\x52\x41\x57\x00\x2B", 7);
SerialTTL.write("\x8C\x05\x50\x00\x26", 5);
SerialTTL.write("\x94\x80\x01\x01\x04\x00\xEF", 7);
SerialTTL.write("\x9A\x00\x54\x41\x50\x50\x45\x44\x00\x00\x71", 11);
SerialTTL.write("\x9A\x01\x00\x00\x00\x00\x00\x00\x40\x40\x64", 11);
SerialTTL.write("\x9A\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xED", 11);
SerialTTL.write("\x9A\x03\x00\x00\x00\x00\x00\x00\x40\x40\x66", 11);
SerialTTL.write("\x92\x04\x49\x44\x58\x00\x3C", 7);
SerialTTL.write("\x8A\x05\x04\x00\x74", 5);
SerialTTL.write("\x92\x80\x01\x00\x03\x00\xEF", 7);
SerialTTL.write("\x99\x00\x54\x4F\x55\x43\x48\x45\x44\x00\x22", 11);
SerialTTL.write("\x99\x01\x00\x00\x00\x00\x00\x00\x80\x3F\xD8", 11);
SerialTTL.write("\x99\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xEE", 11);
SerialTTL.write("\x99\x03\x00\x00\x00\x00\x00\x00\x80\x3F\xDA", 11);
SerialTTL.write("\x91\x04\x49\x44\x58\x00\x3F", 7);
SerialTTL.write("\x89\x05\x04\x00\x77", 5);
SerialTTL.write("\x91\x80\x01\x00\x03\x00\xEC", 7);
SerialTTL.write("\x98\x00\x46\x4F\x52\x43\x45\x00\x00\x00\x3A", 11);
SerialTTL.write("\x98\x01\x00\x00\x00\x00\x00\x00\xC8\x42\xEC", 11);
SerialTTL.write("\x98\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xEF", 11);
SerialTTL.write("\x98\x03\x00\x00\x00\x00\x00\x00\xC8\x42\xEE", 11);
SerialTTL.write("\x90\x04\x46\x4F\x52\x00\x30", 7);
SerialTTL.write("\x88\x05\x50\x00\x22", 5);
SerialTTL.write("\x90\x80\x01\x00\x03\x00\xED", 7);
SerialTTL.write("\x88\x06\x13\x00\x62", 5);
```

We have our init sequence ready to be copied in C++ source code!
