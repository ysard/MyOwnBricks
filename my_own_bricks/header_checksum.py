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
"""Handy functions implementing the UART protocol about messages headers, modes
sizes and checksums.

Doc:
https://github.com/pybricks/technical-info/blob/master/uart-protocol.md
"""
LUMP_MSG_TYPE_MASK = 0xC0

lump_msg_type_t = {
    "LUMP_MSG_TYPE_SYS": 0 << 6,
    "LUMP_MSG_TYPE_CMD": 1 << 6,
    "LUMP_MSG_TYPE_INFO": 2 << 6,
    "LUMP_MSG_TYPE_DATA": 3 << 6,
}

LUMP_MSG_SIZE_MASK = 0x38

lump_msg_size_t = {
    "LUMP_MSG_SIZE_1": 0 << 3,  # 0
    "LUMP_MSG_SIZE_2": 1 << 3,  # 8
    "LUMP_MSG_SIZE_4": 2 << 3,  # 16
    "LUMP_MSG_SIZE_8": 3 << 3,  # 24
    "LUMP_MSG_SIZE_16": 4 << 3, # 32
    "LUMP_MSG_SIZE_32": 5 << 3, # 40
}

LUMP_MSG_CMD_MASK = 0x07

lump_cmd_t = {
    "LUMP_CMD_TYPE": 0x0,
    "LUMP_CMD_MODES": 0x1,
    "LUMP_CMD_SPEED": 0x2,
    "LUMP_CMD_SELECT": 0x3,
    "LUMP_CMD_WRITE": 0x4,
    "LUMP_CMD_UNK1": 0x5,
    "LUMP_CMD_EXT_MODE": 0x6,
    "LUMP_CMD_VERSION": 0x7,
}

color_distance_modes = {
    "COLOR/SPEC1": 0,  # read 1x int8_t
    "PROX/DEBUG": 1,  # read 1x int8_t
    "COUNT/CALIB": 2,  # read 1x int32_t
    "REFLT": 3,  # read 1x int8_t
    "AMBI ": 4,  # read 1x int8_t
    "COL_O": 5,  # writ 1x int8_t
    "RGB_I": 6,  # read 3x int16_t
    "IR_TX": 7,  # writ 1x int16_t
    "SPEC1": 8,  # rrwr 4x int8_t
    "DEBUG": 9,  # ?? 2x int16_t
    "CALIB": 10  # ?? 8x int16_t
}

rev_lump_msg_type_t = {v: k for k, v in lump_msg_type_t.items()}
rev_lump_cmd_t = {v: k for k, v in lump_cmd_t.items()}
rev_lump_msg_size_t = {v: k for k, v in lump_msg_size_t.items()}
rev_color_distance_modes = {v: k for k, v in color_distance_modes.items()}


def get_hub_header(msg_type, cmd, msg_size):
    """Get the header for the corresponding message parameters

    This code is for hub side.

    .. note:: This is an implementation inspired by ev3_uart_begin_tx_msg() in
        https://github.com/pybricks/pybricks-micropython/blob/master/lib/pbio/src/uartdev.c

    :param msg_type: Type of message found in lump_msg_type_t.
    :param size: Size of message, should fit in (1, 2, 8, 16, 32).
    :param cmd: Command, cand be in lump_cmd_t but can also be mode for "set mode"
        queries.
    :type msg_type: <int>
    :type size: <int>
    :type cmd: <int>
    :return: 1 or 2 bytes:
        1 header: for all messages except write queries
        2 headers: only for write queries with msg type LUMP_MSG_TYPE_DATA
    :rtype: <int>
    """
    header1 = None
    header2 = None
    length = msg_size

    if msg_size == 0 or msg_size > 32:
        raise ValueError("Message size must be > 0 and <= 32")

    if msg_type == lump_msg_type_t["LUMP_MSG_TYPE_DATA"]:
        # Write message: send fixed 1st part of message
        header1 = _get_hub_header(
            lump_msg_type_t["LUMP_MSG_TYPE_CMD"],
            lump_msg_size_t["LUMP_MSG_SIZE_1"],
            lump_cmd_t["LUMP_CMD_EXT_MODE"]
        )

    if msg_size == 1:
        size = lump_msg_size_t["LUMP_MSG_SIZE_1"]
    elif msg_size == 2:
        size = lump_msg_size_t["LUMP_MSG_SIZE_2"]
    elif msg_size <= 4:
        size = lump_msg_size_t["LUMP_MSG_SIZE_4"]
        length = 4
    elif msg_size <= 8:
        size = lump_msg_size_t["LUMP_MSG_SIZE_8"]
        length = 8
    elif msg_size <= 16:
        size = lump_msg_size_t["LUMP_MSG_SIZE_16"]
        length = 16
    elif msg_size <= 32:
        size = lump_msg_size_t["LUMP_MSG_SIZE_32"]
        length = 32

    header2 = _get_hub_header(msg_type, size, cmd)

    print("size", msg_size, "size used", rev_lump_msg_size_t[size], "length", length,
          "headers", "" if not header1 else hex(header1), hex(header2))
    return header1, header2


def _get_hub_header(msg_type, size, cmd):
    """Return header byte; This code is for hub side.

    .. note:: This an implementation of ev3_uart_set_msg_hdr in
        https://github.com/pybricks/pybricks-micropython/blob/master/lib/pbio/src/uartdev.c

    .. seealso:: `get_hub_header`

    :param msg_type: Type of message found in lump_msg_type_t.
    :param size: Size of message, should fit in (1, 2, 8, 16, 32).
    :param cmd: Command, cand be in lump_cmd_t but can also be mode for "set mode"
        queries.
    :type msg_type: <int>
    :type size: <int>
    :type cmd: <int>
    :return: Header byte
    :rtype: <int>
    """
    return (msg_type & LUMP_MSG_TYPE_MASK) | (size & LUMP_MSG_SIZE_MASK) | (cmd & LUMP_MSG_CMD_MASK)


def get_size(header):
    """Get message size from the given header

    .. note:: This is an implementation inspired by ev3_uart_get_msg_size() in
        https://github.com/pybricks/pybricks-micropython/blob/master/lib/pbio/src/uartdev.c

    :param header: Header byte.
    :type header: <int>
    :return: Size. LUMP_MSG_TYPE_SYS and LUMP_MSG_TYPE_INFO have special processing.
    :rtype: <int>
    """
    if header & LUMP_MSG_TYPE_MASK == lump_msg_type_t["LUMP_MSG_TYPE_SYS"]:
        # print("LUMP_MSG_TYPE_SYS ?")
        return 1
    size = (1 << ((header >> 3) & 0x7)) + 2
    if header & LUMP_MSG_TYPE_MASK == lump_msg_type_t["LUMP_MSG_TYPE_INFO"]:
        # print("LUMP_MSG_TYPE_INFO ?")
        size += 1
    return size


def parse_device_header(header):
    """Parse a header emitted from a device to the hub

    Extract message type, mode/command and size.
    The command part of LUMP_MSG_TYPE_DATA messages is called a mode.

    .. seealso:: pbio_uartdev_parse_msg() in
        https://github.com/pybricks/pybricks-micropython/blob/master/lib/pbio/src/uartdev.c
        https://github.com/pybricks/technical-info/blob/master/uart-protocol.md
    :return: Tuple of type, mode, size values.
    :rtype: <tuple <int>, <int>, <int>>
    """
    msg_type = rev_lump_msg_type_t[header & LUMP_MSG_TYPE_MASK]
    mode = header & LUMP_MSG_CMD_MASK
    cmd = rev_lump_cmd_t[mode]
    msg_size = get_size(header)
    if msg_type == "LUMP_MSG_TYPE_DATA":
        # cmd message is useless in this case, it's assimilated to the mode (integer value)
        # Also display a meaning mapping of modes for the color & distance sensor
        print("device header:", hex(header), "=> type", msg_type, "mode", mode,
              f"({rev_color_distance_modes[mode]})", "tot size", msg_size)
    elif msg_type == "LUMP_MSG_TYPE_INFO":
        # Meaning of the header is mainly due to the byte next to the header,
        # which is unknown here.
        # Also, the mode obtained is modulo INFO_MODE_PLUS_8, which is also set
        # in the next byte.
        print("device header:", hex(header), "=> type", msg_type, "cmd ? mode",
              "{}/{}".format(mode, mode + 8), "tot size", msg_size)
    else:
        print("device header:", hex(header), "=> type", msg_type, "cmd", cmd,
              "tot size", msg_size)
    return msg_type, mode, msg_size


def get_device_header(msg_type, mode, msg_size):
    """Return header byte: This code is for device side.

    .. seealso:: `_get_hub_header`
    .. warning:: For some reason, msg_size is multiplied by 3 to fit in
        expected sizes decoded in the hub.
        (There are restrictions of sizes due to the masks used).

    :param msg_size: payload size only, WITHOUT header & checksum.
    :type msg_type: <int>
    :type mode: <int>
    :type msg_size: <int>
    :return: Header byte
    :rtype: <int>
    """
    msg_size *= 3
    return (msg_type & LUMP_MSG_TYPE_MASK) | (msg_size & LUMP_MSG_SIZE_MASK) | (mode & LUMP_MSG_CMD_MASK)


def get_all_possible_device_headers():
    """Bruteforce all headers accepted by the hub

    Since the header is in 1 byte, we test all 256 combinations.
    Used to show size restrictions of messages. This could explain 0 padding
    of some messages shorter than their real transmitted size.

    Note that some combinations of settings for a header will never happen
    due to mask constraints on the bits.
    """
    for i in range(256):
        ret = parse_device_header(i)
        if ret == ("LUMP_MSG_TYPE_DATA", 6, 8):  # < this will never happen
            print(ret, i)
            raise ValueError


def get_cheksum(data):
    """Compute checksum for the given data

    :param data: Iterable of values
    :return: checksum
    :rtype: <int>
    """
    checksum = 0xFF
    for i in data:
        checksum ^= i
    #print(checksum, hex(checksum))
    return checksum
