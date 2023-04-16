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
"""Build messages sent by a device
Ex: Init sequence messages

Most of these functions are the Python implementation following the
documentation of PyBricks available at:
https://github.com/pybricks/technical-info/blob/master/uart-protocol.md

The functions are focused on 'Command Messages' and 'Mode Information Messages'.
"""
# Standard imports
from math import ceil
from struct import pack
from operator import or_
from functools import reduce

# Custom imports
from my_own_bricks.header_checksum import (
    parse_device_header,
    get_cheksum,
    get_device_header,
)
from my_own_bricks.header_checksum import lump_msg_type_t, lump_cmd_t


def forge_info_name(text, flags=None):
    """Encode the given text into payload

    :param text: 11 chars max, 5 chars if flags are used with new I/O devices
    :key: flags: Flag bytes for new I/O devices added as they are.
    :type text: <str>
    :type flags: <list <str>>
    :return: Bytes encoded input. NOT padded.
    :rtype: <bytes>
    """
    if not isinstance(text, str):
        # Try autodetection
        # print("New I/O device detected")
        text, flags = text

    if flags:
        assert len(text) < 6
        # text + null char separator + 6 bytes for flags + padding (not added here)
        return text.encode() + b"\x00" + bytearray(ord(i) for i in flags)

    assert len(text) < 12
    if not text:
        return b"\x00"
    return text.encode()


def forge_info_raw(values):
    """Encode the given range of values into payload

    :param values: Tuple of min & max values.
    :type values: <tuple <int>>
    :return: Bytes encoded input. NOT padded.
        4 bytes per value.
    :rtype: <bytes>
    """
    return pack("<" + "f" * len(values), *values)


def forge_info_mapping(flags):
    """Encode the given flags into payload

    .. warning: "N/A" flag name is not supported;
        it will be always replaced by 1 << 0 = 1

    :param flags: Dict with type of flags as keys and flag values as values.
        Flags can be given as a int numbers or strings.
    :type flags: <dict <str>: <tuple <int>>>
    :return: Bytes encoded input. NOT padded.
    :rtype: <bytes>
    """
    info_mapping = {
        1 << 0: "N/A",
        #1 << 1: "N/A",
        1 << 2: "Discrete",
        1 << 3: "Relative",
        1 << 4: "Absolute",
        #1 << 5: "N/A",
        1 << 6: "Func mapping 2.0+",
        1 << 7: "NULL",
    }
    rev_info_mapping = {v: k for k, v in info_mapping.items()}

    data = bytearray()
    for flag_type in ("input_flags", "output_flags"):

        raw_flags = flags[flag_type]
        if not raw_flags:
            data.append(0)
            continue

        if "N/A" in raw_flags:
            print(
                "There could be a problem in the identification of your N/A flag,"
                " prefer using ints instead of strings"
            )

        if all([isinstance(i, str) for i in raw_flags]):
            # Flags as strings
            data.append(reduce(or_, [rev_info_mapping[flag] for flag in raw_flags]))
        else:
            # Flags as numbers
            data.append(reduce(or_, raw_flags))

    return data


def forge_info_mode_combos(modes):
    """Encode the given combo modes into payload

    :param modes: List of expected mode numbers in default combo mode.
    :type modes: <list <int>>
    :return: 2 bytes encoded input. NOT padded.
    :rtype: <bytes>
    """
    val = reduce(or_, [1 << mode for mode in modes])
    # Must fit in 2 bytes
    ret = val.to_bytes(2, byteorder="little")
    return ret


def forge_info_format(*args):
    """Encode the given format numbers describing a mode into payload

    :param args: List of 4 expected format numbers.
        Expected order: data_sets, data_format, figures, decimals.
    :type args: <list <int>>
    :return: 4 bytes encoded input. NOT padded.
    :rtype: <bytes>
    """
    data_sets_data_format_figures_decimals = list(*args)
    assert len(data_sets_data_format_figures_decimals) == 4

    return bytearray(data_sets_data_format_figures_decimals)


def forge_cmd_type(type_id):
    """Encode the given type id into payload

    :param type_id: Id number for the device.
    :type type_id: <int>
    :return: 1 byte encoded input. NOT padded.
    :rtype: <bytes>
    """
    return type_id.to_bytes(1, byteorder="little")


def forge_cmd_speed(baudrate=115200):
    """Encode the given baudrate value into payload

    :key baudrate: (optional) baudrate value. (default: 115200)
    :type baudrate: <int>
    :return: 4 bytes encoded input. NOT padded.
    :rtype: <bytes>
    """
    data_bytes_nb = ceil(baudrate.bit_length() / 8)
    return baudrate.to_bytes(4, byteorder="little")


def forge_cmd_version(versions):
    """Encode the given flags into payload

    :param versions: Dict with type of version number as keys and version numbers as values.
        Expected keys:

            - fw-version
            - hw-version

        Each value is composed of 4 numbers: major, minor, bug_fix, build numbers.
    :type versions: <dict <str>: <tuple <int>>>
    :return: 8 bytes encoded input. NOT padded.
    :rtype: <bytes>
    """
    data = bytearray()
    for version_type in ("fw-version", "hw-version"):

        major, minor, bug_fix, build = versions[version_type]

        # Rebuild the common byte for major & minor versions
        # Add most significant 4 bits
        major_minor = major << 4
        # Add less significant 4 bits
        major_minor |= minor

        ret = pack("<HBB", build, bug_fix, major_minor)
        data += ret
    return data


def forge_cmd_modes(modes):
    """Encode the given number of modes & views into payload

    :param modes: Dict with mode name as keys and number of modes as values.
        Expected keys:

            - <modes> is the total number of modes (limited to a max value of 7).
            - <views> is the number of modes that can be used in Port View or Data Logger (limited to a max value of 7).
            - <modes2> and <views2> are only provided by Powered Up devices.

    :type modes: <dict <str>: <int>>
    :return: 1 to 4 bytes encoded input. 4 bytes max. NOT padded.
    :rtype: <bytes>
    """
    expected_names = ("modes", "views", "modes2", "views2")
    return bytearray([modes[name] - 1 for name in expected_names if name in modes])


def forge_packets(messages_definitions):
    """Main builder method for messages coming from a device

    Support is focused on LUMP_MSG_TYPE_INFO & LUMP_MSG_TYPE_CMD messages used
    in init sequences.

    .. note:: There is NO support for CMD_SELECT, CMD_EXT_MODE & CMD_WRITE messages,
        since they are commands sent by the host.
        See :meth:`my_own_bricks.messages.forge_mode_msg` and
        :meth:`my_own_bricks.messages.forge_write_mode_msg` for the first two.
        See how to use them in :meth:`my_own_bricks.messages.get_device_messages`.

    :param messages_data: Descriptions of messages as they could be obtained
        from :meth:`my_own_bricks.device_messages_parser.parse_messages`.

        :Example:

        [
            (
                ("LUMP_MSG_TYPE_INFO", info_type, mode),
                data
            ),
            (
                ("LUMP_MSG_TYPE_CMD", cmd, None),
                data,
            ),
        ]

    :return: Generator of messages (header, payload, checksum).
    :rtype: <generator <bytearray>>
    """
    # Bit flag used in powered up devices to indicate that the
    # mode is 8 + the mode specified in the first byte
    INFO_MODE_PLUS_8 = 0x20

    # Mapping of functions for LUMP_MSG_TYPE_INFO messages
    info_type_mapping = {
        0: forge_info_name,
        1: forge_info_raw,
        2: forge_info_raw,
        3: forge_info_raw,
        4: forge_info_name,
        5: forge_info_mapping,
        6: forge_info_mode_combos,
        0x80: forge_info_format,
    }

    # Mapping for humand readable names of LUMP_MSG_TYPE_INFO messages
    info_type_descr_mapping = {
        0: "INFO_NAME",
        1: "INFO_RAW",
        2: "INFO_PCT",
        3: "INFO_SI",
        4: "INFO_UNITS",
        5: "INFO_MAPPING",
        6: "INFO_MODE_COMBOS",
        0x80: "INFO_FORMAT",
    }

    # Mapping of functions for LUMP_MSG_TYPE_CMD messages
    cmd_type_mapping = {
        "LUMP_CMD_TYPE": forge_cmd_type,
        "LUMP_CMD_MODES": forge_cmd_modes,
        "LUMP_CMD_SPEED": forge_cmd_speed,
        # "LUMP_CMD_SELECT": forge_cmd_select,
        # "LUMP_CMD_WRITE": forge_cmd_write,
        # "LUMP_CMD_UNK1": None,
        # "LUMP_CMD_EXT_MODE": forge_cmd_ext_mode,
        "LUMP_CMD_VERSION": forge_cmd_version,
    }

    rev_info_type_descr_mapping = {v: k for k, v in info_type_descr_mapping.items()}

    for msg_def in messages_definitions:

        msg_type, info_cmd_type, mode = msg_def[0]
        data = msg_def[1]

        print("=>", msg_type, info_cmd_type, "MODE", mode)

        if msg_type == "LUMP_MSG_TYPE_CMD":
            # Mode is a nonsense for this kind of messages
            # => replace it by cmd
            assert mode is None, "'mode' should be None with LUMP_MSG_TYPE_CMD"
            cmd = info_cmd_type
            mode = lump_cmd_t[cmd]
            payload = cmd_type_mapping[cmd](data)

            # Padding
            # Original payload size
            min_length = len(payload)

        elif msg_type == "LUMP_MSG_TYPE_INFO":
            info_type = info_cmd_type
            info_type_val = rev_info_type_descr_mapping[info_type]
            raw_info_type = info_type_val

            if mode >= 8:
                # INFO_MODE_PLUS_8 is set: mode should be updated
                mode -= 8
                raw_info_type |= INFO_MODE_PLUS_8

            # Next bytes:
            payload = info_type_mapping[info_type_val](data)

            # Padding
            # Original payload size: info_type + payload size
            min_length = 1 + len(payload)

        # Header
        # TODO
        # erreur: payload taille 4, renvoie pour un message une taille 5 (mangque 1 place)
        # payload taille 5, renvoie pour un message de taille 5 (manque 2 places)
        size_offset = 0
        size = 0
        while True:
            header = get_device_header(
                lump_msg_type_t[msg_type], mode, min_length + size_offset
            )
            *_, size = parse_device_header(header)

            if size < min_length + 2:
                # print("Effective size too small:", size)
                size_offset += 1
            else:
                # print("Effective size OK:", size)
                break

        # 1st payload byte is info_type for LUMP_MSG_TYPE_INFO
        msg = (
            bytearray([header, raw_info_type])
            if msg_type == "LUMP_MSG_TYPE_INFO"
            else bytearray([header])
        )
        msg += payload

        # Apply padding to fill the expected size
        padding = size - 2 - min_length
        if padding > 0:
            # Add null bytes of padding
            msg += bytes(padding)

        # Checksum
        msg.append(get_cheksum(msg))

        assert len(msg) == size

        yield msg
