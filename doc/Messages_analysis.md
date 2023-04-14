# Messages analysis

You might want to analyze the packets sent by a module connected to the hub.
The initialization sequence is cryptic and a parser is very useful to know
the capabilities of a Powered Up module to be able to replicate it.

The `parse_message()` function of the `device_messages_parser` module is there for that.

Let's take the example of the initialization sequence of the Tilt Sensor module
module (see [source](https://github.com/ysard/MyOwnBricks/blob/master/src/TiltSensor.cpp)).

It can be reduced to a succession of bytes:

```python
>>> message = "\x40\x22\x9D\x49\x03\x02\xB7\x52\x00\xC2\x01\x00\x6E\x5F\x00\x00\x00\x10\x00\x00\x00\x10\xA0\x9B\x00\x4C\x50\x46\x32\x2D\x43\x41\x4C\x6F\x9B\x01\x00\x00\x34\xC2\x00\x00\x34\x42\xE5\x9B\x02\x00\x00\xC8\xC2\x00\x00\xC8\x42\xE6\x9B\x03\x00\x00\x34\xC2\x00\x00\x34\x42\xE7\x93\x04\x43\x41\x4C\x00\x26\x8B\x05\x10\x00\x61\x93\x80\x03\x00\x03\x00\xEC\xA2\x00\x4C\x50\x46\x32\x2D\x43\x52\x41\x53\x48\x00\x00\x00\x00\x00\x00\x53\x9A\x01\x00\x00\x00\x00\x00\x00\xC8\x42\xEE\x9A\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xED\x9A\x03\x00\x00\x00\x00\x00\x00\xC8\x42\xEC\x92\x04\x43\x4E\x54\x00\x30\x8A\x05\x10\x00\x60\x92\x80\x03\x00\x03\x00\xED\xA1\x00\x4C\x50\x46\x32\x2D\x54\x49\x4C\x54\x00\x00\x00\x00\x00\x00\x00\x1E\x99\x01\x00\x00\x00\x00\x00\x00\x20\x41\x06\x99\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xEE\x99\x03\x00\x00\x00\x00\x00\x00\x20\x41\x04\x91\x04\x44\x49\x52\x00\x35\x89\x05\x04\x00\x77\x91\x80\x01\x00\x02\x00\xED\xA0\x00\x4C\x50\x46\x32\x2D\x41\x4E\x47\x4C\x45\x00\x00\x00\x00\x00\x00\x5B\x98\x01\x00\x00\x34\xC2\x00\x00\x34\x42\xE6\x98\x02\x00\x00\xC8\xC2\x00\x00\xC8\x42\xE5\x98\x03\x00\x00\x34\xC2\x00\x00\x34\x42\xE4\x90\x04\x44\x45\x47\x00\x2D\x88\x05\x10\x00\x62\x90\x80\x02\x00\x03\x00\xEE"
```

Let's see what we can learn from this:

```python
>>> from my_own_bricks.device_messages_parser import parse_message
>>> parse_message(message)
device header: 0x40 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_TYPE tot size 3
        LUMP_CMD_TYPE Type ID: 0x22
device header: 0x49 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_MODES tot size 4
        LUMP_CMD_MODES modes: 4, views: 3
device header: 0x52 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_SPEED tot size 6
        LUMP_CMD_SPEED Speed: 115200
device header: 0x5f => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_VERSION tot size 10
        LUMP_CMD_VERSION fw-version: 1.0.0.0, hw-version: 1.0.0.0
device header: 0x9b => type LUMP_MSG_TYPE_INFO mode 3/11 tot size 11
        MODE 3 INFO_NAME 'LPF2-CAL'
device header: 0x9b => type LUMP_MSG_TYPE_INFO mode 3/11 tot size 11
        MODE 3 INFO_RAW -45.0 to 45.0
device header: 0x9b => type LUMP_MSG_TYPE_INFO mode 3/11 tot size 11
        MODE 3 INFO_PCT -100.0% to 100.0%
device header: 0x9b => type LUMP_MSG_TYPE_INFO mode 3/11 tot size 11
        MODE 3 INFO_SI -45.0 to 45.0
device header: 0x93 => type LUMP_MSG_TYPE_INFO mode 3/11 tot size 7
        MODE 3 INFO_UNITS 'CAL'
device header: 0x8b => type LUMP_MSG_TYPE_INFO mode 3/11 tot size 5
        MODE 3 INFO_MAPPING input_flags: Absolute, output_flags: None
device header: 0x93 => type LUMP_MSG_TYPE_INFO mode 3/11 tot size 7
        MODE 3 INFO_FORMAT 3 int8, each 3 chars, 0 decimals
device header: 0xa2 => type LUMP_MSG_TYPE_INFO mode 2/10 tot size 19
        MODE 2 INFO_NAME 'LPF2-CRASH'
device header: 0x9a => type LUMP_MSG_TYPE_INFO mode 2/10 tot size 11
        MODE 2 INFO_RAW 0.0 to 100.0
device header: 0x9a => type LUMP_MSG_TYPE_INFO mode 2/10 tot size 11
        MODE 2 INFO_PCT 0.0% to 100.0%
device header: 0x9a => type LUMP_MSG_TYPE_INFO mode 2/10 tot size 11
        MODE 2 INFO_SI 0.0 to 100.0
device header: 0x92 => type LUMP_MSG_TYPE_INFO mode 2/10 tot size 7
        MODE 2 INFO_UNITS 'CNT'
device header: 0x8a => type LUMP_MSG_TYPE_INFO mode 2/10 tot size 5
        MODE 2 INFO_MAPPING input_flags: Absolute, output_flags: None
device header: 0x92 => type LUMP_MSG_TYPE_INFO mode 2/10 tot size 7
        MODE 2 INFO_FORMAT 3 int8, each 3 chars, 0 decimals
device header: 0xa1 => type LUMP_MSG_TYPE_INFO mode 1/9 tot size 19
        MODE 1 INFO_NAME 'LPF2-TILT'
device header: 0x99 => type LUMP_MSG_TYPE_INFO mode 1/9 tot size 11
        MODE 1 INFO_RAW 0.0 to 10.0
device header: 0x99 => type LUMP_MSG_TYPE_INFO mode 1/9 tot size 11
        MODE 1 INFO_PCT 0.0% to 100.0%
device header: 0x99 => type LUMP_MSG_TYPE_INFO mode 1/9 tot size 11
        MODE 1 INFO_SI 0.0 to 10.0
device header: 0x91 => type LUMP_MSG_TYPE_INFO mode 1/9 tot size 7
        MODE 1 INFO_UNITS 'DIR'
device header: 0x89 => type LUMP_MSG_TYPE_INFO mode 1/9 tot size 5
        MODE 1 INFO_MAPPING input_flags: Discrete, output_flags: None
device header: 0x91 => type LUMP_MSG_TYPE_INFO mode 1/9 tot size 7
        MODE 1 INFO_FORMAT 1 int8, each 2 chars, 0 decimals
device header: 0xa0 => type LUMP_MSG_TYPE_INFO mode 0/8 tot size 19
        MODE 0 INFO_NAME 'LPF2-ANGLE'
device header: 0x98 => type LUMP_MSG_TYPE_INFO mode 0/8 tot size 11
        MODE 0 INFO_RAW -45.0 to 45.0
device header: 0x98 => type LUMP_MSG_TYPE_INFO mode 0/8 tot size 11
        MODE 0 INFO_PCT -100.0% to 100.0%
device header: 0x98 => type LUMP_MSG_TYPE_INFO mode 0/8 tot size 11
        MODE 0 INFO_SI -45.0 to 45.0
device header: 0x90 => type LUMP_MSG_TYPE_INFO mode 0/8 tot size 7
        MODE 0 INFO_UNITS 'DEG'
device header: 0x88 => type LUMP_MSG_TYPE_INFO mode 0/8 tot size 5
        MODE 0 INFO_MAPPING input_flags: Absolute, output_flags: None
device header: 0x90 => type LUMP_MSG_TYPE_INFO mode 0/8 tot size 7
        MODE 0 INFO_FORMAT 2 int8, each 3 chars, 0 decimals
```

All headers are analyzed, and the description of each mode (0 to 3) is detailed for each packet.
