# MyOwnBricks is a library for the emulation of PoweredUp sensors on microcontrollers
# Copyright (C) 2021-2023 Ysard - <ysard@users.noreply.github.com>
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
"""Forge messages sent to a device"""
# Standard imports
from math import ceil

# Custom imports
from my_own_bricks.header_checksum import get_cheksum, get_hub_header
from my_own_bricks.header_checksum import lump_msg_type_t


def get_hex_msg(data):
    """Convert bytes into hex values"""
    return "".join(['\\x{0:0{1}X}'.format(val, 2) for val in data])


def forge_mode_msg(mode):
    """Force read mode message initiated from hub, sent to device

    :Example: For mode 9:
        0x43, 0x09, 0xb5,
        ^     ^     ^
        |     |     checksum
        |     MODE_9
        MSG_TYPE_CMD | LENGTH_1 | CMD_SELECT

    :return: <bytes>
    """
    msg = bytearray(b"\x43") + mode.to_bytes(1, byteorder='little')
    return msg + get_cheksum(msg).to_bytes(1, byteorder='little')


def forge_write_mode_msg(mode, data=0x04):
    """Forge write mode message (5, 7) initiated from hub, sent to device

    :Example: For mode 5:
        0x46, 0x00, 0xb9, 0xC5, 0x00, 0x3a
        ^     ^     ^     ^     ^     ^
        |     |     |     |     |     checksum
        |     |     |     |     color index = 0
        |     |     |     MESSAGE_DATA | LENGTH_1 | MODE_5
        |     |     checksum
        |     EXT_MODE_0
        MESSAGE_CMD | LENGTH_1 | CMD_SELECT

    :param mode: Write mode queried. Must be 5 or 7.
    :param data: Value to set (1 int for now).
    :type mode: <int>
    :type data: <int>
    :return: <bytes>
    """
    assert mode in (5, 7)
    # 2 bytes for mode 7, 1 byte for mode 5
    data_bytes_nb = 2 if mode == 7 else 1

    headers = get_hub_header(lump_msg_type_t["LUMP_MSG_TYPE_DATA"], mode, data_bytes_nb)
    assert headers[0] == 0x46

    # First part of the message is fixed
    # modes 5, 7 don't require EXT_MODE_8,
    # so we have EXT_MODE_0 at index 1
    msg = bytearray(b"\x46\x00\xb9")
    # Second part
    msg += headers[1].to_bytes(1, byteorder='little')  # Header
    # Get byte number
    msg += data.to_bytes(data_bytes_nb, byteorder='little')  # Data
    return msg + get_cheksum(msg).to_bytes(1, byteorder='little')  # Checksum
