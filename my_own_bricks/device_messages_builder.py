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
from my_own_bricks.header_checksum import parse_device_header, get_cheksum


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
        #print("New I/O device detected")
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

    :param flags: Dict with type of flags as keys and flag values as values.
        Flags can be given as a int numbers or strings.
    :type flags: <dict <str>: <tuple <int>>>
    :return: Bytes encoded input. NOT padded.
    :rtype: <bytes>
    """
    info_mapping = {
        1 << 0: "N/A",
        1 << 1: "N/A",
        1 << 2: "Discrete",
        1 << 3: "Relative",
        1 << 4: "Absolute",
        1 << 5: "N/A",
        1 << 6: "Func mapping 2.0+",
        1 << 7: "NULL",
    }
    rev_info_mapping = {v: k for k, v in info_mapping.items() if v not in ("N/A",)}

    data = bytearray()
    for flag_type in ("input_flags", "output_flags"):

        raw_flags = flags[flag_type]
        if not raw_flags:
            data.append(0)
            continue

        if "N/A" in raw_flags:
            print(
                "There could be a problem in the identification of your N/A flag,"
                "prefer using ints instead of strings"
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
    :return: 2 bytes encoded input. NOT padded.
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
