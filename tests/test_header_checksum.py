# MyOwnBricks is a library for the emulation of PoweredUp sensors on microcontrollers
# Copyright (C) 2021-2022 Ysard - <ysard@users.noreply.github.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
"""Test header & checksum API"""
import pytest
from my_own_bricks.header_checksum import (
    get_hub_header,
    parse_device_header,
    get_device_header,
    get_cheksum,
)
from my_own_bricks.header_checksum import lump_msg_type_t, lump_cmd_t
from my_own_bricks.header_checksum import get_all_possible_device_headers


def test_get_cheksum():
    """Test checksum computation on a given packet (header + payload)"""

    ret = get_cheksum(
        [bytes.fromhex(val)[0] for val in "5C 23 00 00 10 40 00 00 00".split()]
    )
    assert ret == 0xD0


def test_parse_device_header():
    """Parse headers from responses from device"""
    # extened mode info: LUMP_MSG_TYPE_CMD + LUMP_CMD_EXT_MODE + size 3
    parse_device_header(0x46)
    ret = parse_device_header(0xC0)  # mode 0 data
    assert ret == ("LUMP_MSG_TYPE_DATA", 0, 3)
    ret = parse_device_header(0xC1)  # mode 1 data
    assert ret == ("LUMP_MSG_TYPE_DATA", 1, 3)
    ret = parse_device_header(0xC5)  # mode 5 data
    assert ret == ("LUMP_MSG_TYPE_DATA", 5, 3)
    ret = parse_device_header(0xDE)  # mode 6 data
    assert ret == ("LUMP_MSG_TYPE_DATA", 6, 10)
    ret = parse_device_header(0xD0)  # mode 8 data
    assert ret == ("LUMP_MSG_TYPE_DATA", 0, 6), ret


def test_get_device_header():
    """Forge device header: msg_type, mode, msg_size"""
    header = get_device_header(
        lump_msg_type_t["LUMP_MSG_TYPE_CMD"], lump_cmd_t["LUMP_CMD_EXT_MODE"], 1
    )
    print("device header extened mode info:", hex(header))
    assert header == 0x46

    header = get_device_header(lump_msg_type_t["LUMP_MSG_TYPE_DATA"], 0, 1)
    print("device header mode 0:", hex(header))
    assert header == 0xC0

    header = get_device_header(lump_msg_type_t["LUMP_MSG_TYPE_DATA"], 1, 1)
    print("device header mode 1:", hex(header))
    assert header == 0xC1

    # size payload = 6, tot 8
    # size 24 0x18, LUMP_MSG_SIZE_8 recognized 10 !!!!!!!!!!!!
    # => 2 padding bytes here
    header = get_device_header(lump_msg_type_t["LUMP_MSG_TYPE_DATA"], 6, 8)
    print("device header mode 6:", hex(header))
    assert header == 0xde
    ret = parse_device_header(header)
    assert ret == ("LUMP_MSG_TYPE_DATA", 6, 10)

    # size payload = 4, tot 6
    # size soit 16 0x10 LUMP_MSG_SIZE_4 ou 18 ??, recognized 6
    header = get_device_header(lump_msg_type_t["LUMP_MSG_TYPE_DATA"], 8, 6)
    print("device header mode 8:", hex(header))
    assert header == 0xD0
    ret = parse_device_header(header)
    assert ret == ("LUMP_MSG_TYPE_DATA", 0, 6)


def test_get_hub_header():
    """Forge hub header for get/set (read/write) queries"""
    # Read commands initialized by pb_device_get_values()
    # OR first message sent by pb_device_set_values().
    # => message sent by set_mode()
    # msg type, cmd, length
    # 1: The mode is sent as "data" (index 1) in a 0x43 packet, NOT in the header
    ret = get_hub_header(
        lump_msg_type_t["LUMP_MSG_TYPE_CMD"], lump_cmd_t["LUMP_CMD_SELECT"], 1
    )
    assert ret == (None, 0x43)

    # Write commands initialized by pb_device_set_values().
    # => message sent by set_mode() and is probably not wanted because the next
    # ones already define the mode in their header (with message type
    # LUMP_MSG_TYPE_DATA).
    # 1: The mode is sent as "data" (index 1) in a 0x43 packet, NOT in the header
    _ = get_hub_header(
        lump_msg_type_t["LUMP_MSG_TYPE_CMD"], lump_cmd_t["LUMP_CMD_SELECT"], 1
    )
    # 2: Mode 5 (set color idx): write 1 x 1 bytes (int8),
    # the mode is sent in the "cmd" field this time.
    ret = get_hub_header(lump_msg_type_t["LUMP_MSG_TYPE_DATA"], 5, 1)
    assert ret == (0x46, 0xc5)

    # 2: Mode 7 (TX IR): 2 bytes (1 of 16bits to be casted later...)
    # the mode is sent in the "cmd" field this time.
    ret = get_hub_header(lump_msg_type_t["LUMP_MSG_TYPE_DATA"], 7, 2)
    assert ret == (0x46, 0xcf)

    with pytest.raises(ValueError, match=r"Message size must be.*"):
        get_hub_header(lump_msg_type_t["LUMP_MSG_TYPE_DATA"], 5, 0)


def test_get_all_possible_device_headers():
    """No exception should be raised during the enumeration of all possible headers"""
    get_all_possible_device_headers()
