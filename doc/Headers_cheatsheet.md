# Headers cheatsheet

## Compute a header for a payload

Here is a method for calculating a header that will be sent to the hub with the Python binding.

```python
>>> from my_own_bricks.header_checksum import (
...     get_hub_header,
...     parse_device_header,
...     get_device_header,
...     get_cheksum,
... )
>>> from my_own_bricks.header_checksum import lump_msg_type_t, lump_cmd_t
>>> header = get_device_header(lump_msg_type_t["LUMP_MSG_TYPE_CMD"], lump_cmd_t["LUMP_CMD_WRITE"], 10)
>>> print(hex(header))
'0x5c'
>>> # Let's check it
>>> parse_device_header(0x5c)
device header: 0x5c => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_WRITE tot size 10
('LUMP_MSG_TYPE_CMD', 4, 10)
```

This function is able to automatically compute a padding for a payload with a
smaller size than that allowed (See the allowed list in the following section).

```python
>>> # Let's take a data packet sent by a device on the mode 6, with 8 bytes of payload
>>> header = get_device_header(lump_msg_type_t["LUMP_MSG_TYPE_DATA"], 6, 8)
>>> assert header == 0xde
>>> # A size of 8 is not allowed and is extended to next allowed size: 10
>>> header = get_device_header(lump_msg_type_t["LUMP_MSG_TYPE_DATA"], 6, 10)
>>> assert header == 0xde
>>> # Let's check it
>>> ret = parse_device_header(header)
>>> assert ret == ("LUMP_MSG_TYPE_DATA", 6, 10)
```

## List of all existing headers

Here is the list of all the possible headers (encoded on 1 byte from 0 to 255)
and their meanings (type of message, command, mode and total size of packet).

For type `LUMP_MSG_TYPE_DATA`, modes meanings for Boost Color & Distance Sensor
are displayed for information purposes.

Notes:

- with LUMP_MSG_TYPE_DATA msg, cmd message is useless,
it's assimilated to the mode

- with LUMP_MSG_TYPE_INFO msg, mode is obtained with INFO_MODE_PLUS_8
flag which is set in the next byte (not the header).
cmd message is also useless it's the info_type data that is useful,
also in the next byte.

This output is obtained via the following code:

```python
from my_own_bricks.header_checksum import get_all_possible_device_headers
get_all_possible_device_headers()
```

```
device header: 0x0 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_TYPE tot size 1
device header: 0x1 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_MODES tot size 1
device header: 0x2 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_SPEED tot size 1
device header: 0x3 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_SELECT tot size 1
device header: 0x4 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_WRITE tot size 1
device header: 0x5 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_UNK1 tot size 1
device header: 0x6 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_EXT_MODE tot size 1
device header: 0x7 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_VERSION tot size 1
device header: 0x8 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_TYPE tot size 1
device header: 0x9 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_MODES tot size 1
device header: 0xa => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_SPEED tot size 1
device header: 0xb => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_SELECT tot size 1
device header: 0xc => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_WRITE tot size 1
device header: 0xd => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_UNK1 tot size 1
device header: 0xe => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_EXT_MODE tot size 1
device header: 0xf => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_VERSION tot size 1
device header: 0x10 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_TYPE tot size 1
device header: 0x11 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_MODES tot size 1
device header: 0x12 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_SPEED tot size 1
device header: 0x13 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_SELECT tot size 1
device header: 0x14 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_WRITE tot size 1
device header: 0x15 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_UNK1 tot size 1
device header: 0x16 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_EXT_MODE tot size 1
device header: 0x17 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_VERSION tot size 1
device header: 0x18 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_TYPE tot size 1
device header: 0x19 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_MODES tot size 1
device header: 0x1a => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_SPEED tot size 1
device header: 0x1b => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_SELECT tot size 1
device header: 0x1c => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_WRITE tot size 1
device header: 0x1d => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_UNK1 tot size 1
device header: 0x1e => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_EXT_MODE tot size 1
device header: 0x1f => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_VERSION tot size 1
device header: 0x20 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_TYPE tot size 1
device header: 0x21 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_MODES tot size 1
device header: 0x22 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_SPEED tot size 1
device header: 0x23 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_SELECT tot size 1
device header: 0x24 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_WRITE tot size 1
device header: 0x25 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_UNK1 tot size 1
device header: 0x26 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_EXT_MODE tot size 1
device header: 0x27 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_VERSION tot size 1
device header: 0x28 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_TYPE tot size 1
device header: 0x29 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_MODES tot size 1
device header: 0x2a => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_SPEED tot size 1
device header: 0x2b => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_SELECT tot size 1
device header: 0x2c => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_WRITE tot size 1
device header: 0x2d => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_UNK1 tot size 1
device header: 0x2e => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_EXT_MODE tot size 1
device header: 0x2f => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_VERSION tot size 1
device header: 0x30 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_TYPE tot size 1
device header: 0x31 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_MODES tot size 1
device header: 0x32 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_SPEED tot size 1
device header: 0x33 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_SELECT tot size 1
device header: 0x34 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_WRITE tot size 1
device header: 0x35 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_UNK1 tot size 1
device header: 0x36 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_EXT_MODE tot size 1
device header: 0x37 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_VERSION tot size 1
device header: 0x38 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_TYPE tot size 1
device header: 0x39 => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_MODES tot size 1
device header: 0x3a => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_SPEED tot size 1
device header: 0x3b => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_SELECT tot size 1
device header: 0x3c => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_WRITE tot size 1
device header: 0x3d => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_UNK1 tot size 1
device header: 0x3e => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_EXT_MODE tot size 1
device header: 0x3f => type LUMP_MSG_TYPE_SYS cmd LUMP_CMD_VERSION tot size 1
device header: 0x40 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_TYPE tot size 3
device header: 0x41 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_MODES tot size 3
device header: 0x42 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_SPEED tot size 3
device header: 0x43 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_SELECT tot size 3
device header: 0x44 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_WRITE tot size 3
device header: 0x45 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_UNK1 tot size 3
device header: 0x46 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_EXT_MODE tot size 3
device header: 0x47 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_VERSION tot size 3
device header: 0x48 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_TYPE tot size 4
device header: 0x49 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_MODES tot size 4
device header: 0x4a => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_SPEED tot size 4
device header: 0x4b => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_SELECT tot size 4
device header: 0x4c => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_WRITE tot size 4
device header: 0x4d => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_UNK1 tot size 4
device header: 0x4e => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_EXT_MODE tot size 4
device header: 0x4f => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_VERSION tot size 4
device header: 0x50 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_TYPE tot size 6
device header: 0x51 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_MODES tot size 6
device header: 0x52 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_SPEED tot size 6
device header: 0x53 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_SELECT tot size 6
device header: 0x54 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_WRITE tot size 6
device header: 0x55 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_UNK1 tot size 6
device header: 0x56 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_EXT_MODE tot size 6
device header: 0x57 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_VERSION tot size 6
device header: 0x58 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_TYPE tot size 10
device header: 0x59 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_MODES tot size 10
device header: 0x5a => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_SPEED tot size 10
device header: 0x5b => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_SELECT tot size 10
device header: 0x5c => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_WRITE tot size 10
device header: 0x5d => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_UNK1 tot size 10
device header: 0x5e => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_EXT_MODE tot size 10
device header: 0x5f => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_VERSION tot size 10
device header: 0x60 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_TYPE tot size 18
device header: 0x61 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_MODES tot size 18
device header: 0x62 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_SPEED tot size 18
device header: 0x63 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_SELECT tot size 18
device header: 0x64 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_WRITE tot size 18
device header: 0x65 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_UNK1 tot size 18
device header: 0x66 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_EXT_MODE tot size 18
device header: 0x67 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_VERSION tot size 18
device header: 0x68 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_TYPE tot size 34
device header: 0x69 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_MODES tot size 34
device header: 0x6a => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_SPEED tot size 34
device header: 0x6b => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_SELECT tot size 34
device header: 0x6c => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_WRITE tot size 34
device header: 0x6d => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_UNK1 tot size 34
device header: 0x6e => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_EXT_MODE tot size 34
device header: 0x6f => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_VERSION tot size 34
device header: 0x70 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_TYPE tot size 66
device header: 0x71 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_MODES tot size 66
device header: 0x72 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_SPEED tot size 66
device header: 0x73 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_SELECT tot size 66
device header: 0x74 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_WRITE tot size 66
device header: 0x75 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_UNK1 tot size 66
device header: 0x76 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_EXT_MODE tot size 66
device header: 0x77 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_VERSION tot size 66
device header: 0x78 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_TYPE tot size 130
device header: 0x79 => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_MODES tot size 130
device header: 0x7a => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_SPEED tot size 130
device header: 0x7b => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_SELECT tot size 130
device header: 0x7c => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_WRITE tot size 130
device header: 0x7d => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_UNK1 tot size 130
device header: 0x7e => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_EXT_MODE tot size 130
device header: 0x7f => type LUMP_MSG_TYPE_CMD cmd LUMP_CMD_VERSION tot size 130
device header: 0x80 => type LUMP_MSG_TYPE_INFO mode 0/8 tot size 4
device header: 0x81 => type LUMP_MSG_TYPE_INFO mode 1/9 tot size 4
device header: 0x82 => type LUMP_MSG_TYPE_INFO mode 2/10 tot size 4
device header: 0x83 => type LUMP_MSG_TYPE_INFO mode 3/11 tot size 4
device header: 0x84 => type LUMP_MSG_TYPE_INFO mode 4/12 tot size 4
device header: 0x85 => type LUMP_MSG_TYPE_INFO mode 5/13 tot size 4
device header: 0x86 => type LUMP_MSG_TYPE_INFO mode 6/14 tot size 4
device header: 0x87 => type LUMP_MSG_TYPE_INFO mode 7/15 tot size 4
device header: 0x88 => type LUMP_MSG_TYPE_INFO mode 0/8 tot size 5
device header: 0x89 => type LUMP_MSG_TYPE_INFO mode 1/9 tot size 5
device header: 0x8a => type LUMP_MSG_TYPE_INFO mode 2/10 tot size 5
device header: 0x8b => type LUMP_MSG_TYPE_INFO mode 3/11 tot size 5
device header: 0x8c => type LUMP_MSG_TYPE_INFO mode 4/12 tot size 5
device header: 0x8d => type LUMP_MSG_TYPE_INFO mode 5/13 tot size 5
device header: 0x8e => type LUMP_MSG_TYPE_INFO mode 6/14 tot size 5
device header: 0x8f => type LUMP_MSG_TYPE_INFO mode 7/15 tot size 5
device header: 0x90 => type LUMP_MSG_TYPE_INFO mode 0/8 tot size 7
device header: 0x91 => type LUMP_MSG_TYPE_INFO mode 1/9 tot size 7
device header: 0x92 => type LUMP_MSG_TYPE_INFO mode 2/10 tot size 7
device header: 0x93 => type LUMP_MSG_TYPE_INFO mode 3/11 tot size 7
device header: 0x94 => type LUMP_MSG_TYPE_INFO mode 4/12 tot size 7
device header: 0x95 => type LUMP_MSG_TYPE_INFO mode 5/13 tot size 7
device header: 0x96 => type LUMP_MSG_TYPE_INFO mode 6/14 tot size 7
device header: 0x97 => type LUMP_MSG_TYPE_INFO mode 7/15 tot size 7
device header: 0x98 => type LUMP_MSG_TYPE_INFO mode 0/8 tot size 11
device header: 0x99 => type LUMP_MSG_TYPE_INFO mode 1/9 tot size 11
device header: 0x9a => type LUMP_MSG_TYPE_INFO mode 2/10 tot size 11
device header: 0x9b => type LUMP_MSG_TYPE_INFO mode 3/11 tot size 11
device header: 0x9c => type LUMP_MSG_TYPE_INFO mode 4/12 tot size 11
device header: 0x9d => type LUMP_MSG_TYPE_INFO mode 5/13 tot size 11
device header: 0x9e => type LUMP_MSG_TYPE_INFO mode 6/14 tot size 11
device header: 0x9f => type LUMP_MSG_TYPE_INFO mode 7/15 tot size 11
device header: 0xa0 => type LUMP_MSG_TYPE_INFO mode 0/8 tot size 19
device header: 0xa1 => type LUMP_MSG_TYPE_INFO mode 1/9 tot size 19
device header: 0xa2 => type LUMP_MSG_TYPE_INFO mode 2/10 tot size 19
device header: 0xa3 => type LUMP_MSG_TYPE_INFO mode 3/11 tot size 19
device header: 0xa4 => type LUMP_MSG_TYPE_INFO mode 4/12 tot size 19
device header: 0xa5 => type LUMP_MSG_TYPE_INFO mode 5/13 tot size 19
device header: 0xa6 => type LUMP_MSG_TYPE_INFO mode 6/14 tot size 19
device header: 0xa7 => type LUMP_MSG_TYPE_INFO mode 7/15 tot size 19
device header: 0xa8 => type LUMP_MSG_TYPE_INFO mode 0/8 tot size 35
device header: 0xa9 => type LUMP_MSG_TYPE_INFO mode 1/9 tot size 35
device header: 0xaa => type LUMP_MSG_TYPE_INFO mode 2/10 tot size 35
device header: 0xab => type LUMP_MSG_TYPE_INFO mode 3/11 tot size 35
device header: 0xac => type LUMP_MSG_TYPE_INFO mode 4/12 tot size 35
device header: 0xad => type LUMP_MSG_TYPE_INFO mode 5/13 tot size 35
device header: 0xae => type LUMP_MSG_TYPE_INFO mode 6/14 tot size 35
device header: 0xaf => type LUMP_MSG_TYPE_INFO mode 7/15 tot size 35
device header: 0xb0 => type LUMP_MSG_TYPE_INFO mode 0/8 tot size 67
device header: 0xb1 => type LUMP_MSG_TYPE_INFO mode 1/9 tot size 67
device header: 0xb2 => type LUMP_MSG_TYPE_INFO mode 2/10 tot size 67
device header: 0xb3 => type LUMP_MSG_TYPE_INFO mode 3/11 tot size 67
device header: 0xb4 => type LUMP_MSG_TYPE_INFO mode 4/12 tot size 67
device header: 0xb5 => type LUMP_MSG_TYPE_INFO mode 5/13 tot size 67
device header: 0xb6 => type LUMP_MSG_TYPE_INFO mode 6/14 tot size 67
device header: 0xb7 => type LUMP_MSG_TYPE_INFO mode 7/15 tot size 67
device header: 0xb8 => type LUMP_MSG_TYPE_INFO mode 0/8 tot size 131
device header: 0xb9 => type LUMP_MSG_TYPE_INFO mode 1/9 tot size 131
device header: 0xba => type LUMP_MSG_TYPE_INFO mode 2/10 tot size 131
device header: 0xbb => type LUMP_MSG_TYPE_INFO mode 3/11 tot size 131
device header: 0xbc => type LUMP_MSG_TYPE_INFO mode 4/12 tot size 131
device header: 0xbd => type LUMP_MSG_TYPE_INFO mode 5/13 tot size 131
device header: 0xbe => type LUMP_MSG_TYPE_INFO mode 6/14 tot size 131
device header: 0xbf => type LUMP_MSG_TYPE_INFO mode 7/15 tot size 131
device header: 0xc0 => type LUMP_MSG_TYPE_DATA mode 0 (COLOR/SPEC1) tot size 3
device header: 0xc1 => type LUMP_MSG_TYPE_DATA mode 1 (PROX/DEBUG) tot size 3
device header: 0xc2 => type LUMP_MSG_TYPE_DATA mode 2 (COUNT/CALIB) tot size 3
device header: 0xc3 => type LUMP_MSG_TYPE_DATA mode 3 (REFLT) tot size 3
device header: 0xc4 => type LUMP_MSG_TYPE_DATA mode 4 (AMBI ) tot size 3
device header: 0xc5 => type LUMP_MSG_TYPE_DATA mode 5 (COL_O) tot size 3
device header: 0xc6 => type LUMP_MSG_TYPE_DATA mode 6 (RGB_I) tot size 3
device header: 0xc7 => type LUMP_MSG_TYPE_DATA mode 7 (IR_TX) tot size 3
device header: 0xc8 => type LUMP_MSG_TYPE_DATA mode 0 (COLOR/SPEC1) tot size 4
device header: 0xc9 => type LUMP_MSG_TYPE_DATA mode 1 (PROX/DEBUG) tot size 4
device header: 0xca => type LUMP_MSG_TYPE_DATA mode 2 (COUNT/CALIB) tot size 4
device header: 0xcb => type LUMP_MSG_TYPE_DATA mode 3 (REFLT) tot size 4
device header: 0xcc => type LUMP_MSG_TYPE_DATA mode 4 (AMBI ) tot size 4
device header: 0xcd => type LUMP_MSG_TYPE_DATA mode 5 (COL_O) tot size 4
device header: 0xce => type LUMP_MSG_TYPE_DATA mode 6 (RGB_I) tot size 4
device header: 0xcf => type LUMP_MSG_TYPE_DATA mode 7 (IR_TX) tot size 4
device header: 0xd0 => type LUMP_MSG_TYPE_DATA mode 0 (COLOR/SPEC1) tot size 6
device header: 0xd1 => type LUMP_MSG_TYPE_DATA mode 1 (PROX/DEBUG) tot size 6
device header: 0xd2 => type LUMP_MSG_TYPE_DATA mode 2 (COUNT/CALIB) tot size 6
device header: 0xd3 => type LUMP_MSG_TYPE_DATA mode 3 (REFLT) tot size 6
device header: 0xd4 => type LUMP_MSG_TYPE_DATA mode 4 (AMBI ) tot size 6
device header: 0xd5 => type LUMP_MSG_TYPE_DATA mode 5 (COL_O) tot size 6
device header: 0xd6 => type LUMP_MSG_TYPE_DATA mode 6 (RGB_I) tot size 6
device header: 0xd7 => type LUMP_MSG_TYPE_DATA mode 7 (IR_TX) tot size 6
device header: 0xd8 => type LUMP_MSG_TYPE_DATA mode 0 (COLOR/SPEC1) tot size 10
device header: 0xd9 => type LUMP_MSG_TYPE_DATA mode 1 (PROX/DEBUG) tot size 10
device header: 0xda => type LUMP_MSG_TYPE_DATA mode 2 (COUNT/CALIB) tot size 10
device header: 0xdb => type LUMP_MSG_TYPE_DATA mode 3 (REFLT) tot size 10
device header: 0xdc => type LUMP_MSG_TYPE_DATA mode 4 (AMBI ) tot size 10
device header: 0xdd => type LUMP_MSG_TYPE_DATA mode 5 (COL_O) tot size 10
device header: 0xde => type LUMP_MSG_TYPE_DATA mode 6 (RGB_I) tot size 10
device header: 0xdf => type LUMP_MSG_TYPE_DATA mode 7 (IR_TX) tot size 10
device header: 0xe0 => type LUMP_MSG_TYPE_DATA mode 0 (COLOR/SPEC1) tot size 18
device header: 0xe1 => type LUMP_MSG_TYPE_DATA mode 1 (PROX/DEBUG) tot size 18
device header: 0xe2 => type LUMP_MSG_TYPE_DATA mode 2 (COUNT/CALIB) tot size 18
device header: 0xe3 => type LUMP_MSG_TYPE_DATA mode 3 (REFLT) tot size 18
device header: 0xe4 => type LUMP_MSG_TYPE_DATA mode 4 (AMBI ) tot size 18
device header: 0xe5 => type LUMP_MSG_TYPE_DATA mode 5 (COL_O) tot size 18
device header: 0xe6 => type LUMP_MSG_TYPE_DATA mode 6 (RGB_I) tot size 18
device header: 0xe7 => type LUMP_MSG_TYPE_DATA mode 7 (IR_TX) tot size 18
device header: 0xe8 => type LUMP_MSG_TYPE_DATA mode 0 (COLOR/SPEC1) tot size 34
device header: 0xe9 => type LUMP_MSG_TYPE_DATA mode 1 (PROX/DEBUG) tot size 34
device header: 0xea => type LUMP_MSG_TYPE_DATA mode 2 (COUNT/CALIB) tot size 34
device header: 0xeb => type LUMP_MSG_TYPE_DATA mode 3 (REFLT) tot size 34
device header: 0xec => type LUMP_MSG_TYPE_DATA mode 4 (AMBI ) tot size 34
device header: 0xed => type LUMP_MSG_TYPE_DATA mode 5 (COL_O) tot size 34
device header: 0xee => type LUMP_MSG_TYPE_DATA mode 6 (RGB_I) tot size 34
device header: 0xef => type LUMP_MSG_TYPE_DATA mode 7 (IR_TX) tot size 34
device header: 0xf0 => type LUMP_MSG_TYPE_DATA mode 0 (COLOR/SPEC1) tot size 66
device header: 0xf1 => type LUMP_MSG_TYPE_DATA mode 1 (PROX/DEBUG) tot size 66
device header: 0xf2 => type LUMP_MSG_TYPE_DATA mode 2 (COUNT/CALIB) tot size 66
device header: 0xf3 => type LUMP_MSG_TYPE_DATA mode 3 (REFLT) tot size 66
device header: 0xf4 => type LUMP_MSG_TYPE_DATA mode 4 (AMBI ) tot size 66
device header: 0xf5 => type LUMP_MSG_TYPE_DATA mode 5 (COL_O) tot size 66
device header: 0xf6 => type LUMP_MSG_TYPE_DATA mode 6 (RGB_I) tot size 66
device header: 0xf7 => type LUMP_MSG_TYPE_DATA mode 7 (IR_TX) tot size 66
device header: 0xf8 => type LUMP_MSG_TYPE_DATA mode 0 (COLOR/SPEC1) tot size 130
device header: 0xf9 => type LUMP_MSG_TYPE_DATA mode 1 (PROX/DEBUG) tot size 130
device header: 0xfa => type LUMP_MSG_TYPE_DATA mode 2 (COUNT/CALIB) tot size 130
device header: 0xfb => type LUMP_MSG_TYPE_DATA mode 3 (REFLT) tot size 130
device header: 0xfc => type LUMP_MSG_TYPE_DATA mode 4 (AMBI ) tot size 130
device header: 0xfd => type LUMP_MSG_TYPE_DATA mode 5 (COL_O) tot size 130
device header: 0xfe => type LUMP_MSG_TYPE_DATA mode 6 (RGB_I) tot size 130
device header: 0xff => type LUMP_MSG_TYPE_DATA mode 7 (IR_TX) tot size 130
```
