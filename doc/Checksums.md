# Checksums

## Compute checksum of a packet

Here is a method to compute a checksum of a known payload with the Python binding.

```python
>>> from my_own_bricks.header_checksum import get_cheksum
>>> hex(get_cheksum([bytes.fromhex(val)[0] for val in "5C 23 00 00 10 40 00 00 00".split()]))
'0xd0'
```

Or if you don't know the header, let's compute it before:

```python
>>> from my_own_bricks.header_checksum import get_cheksum
>>> hex(get_cheksum([bytes.fromhex(val)[0] for val in "5C 23 00 00 10 40 00 00 00".split()]))
'0xd0'
```

```python
>>> from my_own_bricks.header_checksum import (
...     get_device_header,
...     get_cheksum,
... )
>>> from my_own_bricks.header_checksum import lump_msg_type_t, lump_cmd_t
>>> header = hex(get_device_header(lump_msg_type_t["LUMP_MSG_TYPE_CMD"], lump_cmd_t["LUMP_CMD_WRITE"], 10))
>>> print(hex(header))
'0x5c'
>>> hex(get_cheksum([header] + [bytes.fromhex(val)[0] for val in "23 00 00 10 40 00 00 00".split()]))
'0xd0'
```

See also https://github.com/ysard/MyOwnBricks/blob/master/doc/Headers_cheatsheet.md
