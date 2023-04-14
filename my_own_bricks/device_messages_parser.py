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
"""Parse messages sent by a device
Ex: Init sequence messages

Most of these functions are the Python implementation following the
documentation of PyBricks available at:
https://github.com/pybricks/technical-info/blob/master/uart-protocol.md

The functions are focused on 'Command Messages' and 'Mode Information Messages'.

.. note:: CMD_SELECT & CMD_WRITE are basically supported (the raw payload is returned)
"""
# Standard imports
from struct import unpack


def parse_info_name(payload):
    """Get the content of text payload

    Used for INFO_NAME & INFO_UNITS.

    Newer I/O devices may also supply 6 bytes of information after the name.
    In this case, the name will be 5 bytes or less, followed by a zero terminator
    and the last 6 bytes will be the motor info.

    .. seealso::
        https://github.com/pybricks/technical-info/blob/master/uart-protocol.md#info_name
        https://github.com/pybricks/technical-info/blob/master/uart-protocol.md#info_units

    :param payload: List of chars found in the data part of a message.
    :type payload: <list <str>>
    :return: Raw data parsed and human readable formatted data
    :rtype <tuple <str>, <str>>
    """
    # INFO_NAME:
    # - Newer I/O devices:
    # In this case, the name will be 5 bytes or less, followed by a zero terminator
    # - default:
    # This is limited to 11 characters.
    if len(payload) > 12 and "\x00" in payload and payload.index("\x00") <= 5:
        print("New I/O devices with flags & padding: Not implemented")
    # strip trailing null chars
    raw_text = "".join(payload).rstrip("\x00")
    text = "'" + raw_text + "'"
    return raw_text, text


def parse_info_raw(payload, percentage=False):
    """Provide info about the scaling (min/max) of the raw sensor (legacy)

    Used for INFO_RAW, INFO_SI and INFO_PCT (with percentage param set to True).

    .. seealso::
        https://github.com/pybricks/technical-info/blob/master/uart-protocol.md#info_raw

    :param payload: List of chars found in the data part of a message.
    :key percentage: Only for INFO_PCT messages. Just add '%' symbol for each
        values in human readable text.
    :type payload: <list <str>>
    :return: Raw data parsed (min/max values) and human readable formatted data
    :rtype <tuple <tuple <float>, <float>>, <float>>, <str>>
    """
    # Convert sequence to bytes
    payload = "".join(payload).encode("latin1")

    # Convert the 2 floats at once
    min_value, max_value = unpack("<ff", payload)

    if percentage:
        return (min_value, max_value), f"{min_value}% to {max_value}%"
    return (min_value, max_value), f"{min_value} to {max_value}"


def parse_info_mapping(payload):
    """Provide mode mapping info for Powered Up devices

    Mapping information is the description of input & output data
    via some flags.

    .. seealso::
        https://github.com/pybricks/technical-info/blob/master/uart-protocol.md#info_mapping

    :param payload: List of chars found in the data part of a message.
    :type payload: <list <str>>
    :return: Raw data parsed (input/output flags) and human readable formatted data
    :rtype <tuple <dict <str>:<int>>, <str>>
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
    input_flags, output_flags = payload

    raw_flags = {
        "input_flags": ord(input_flags),
        "output_flags": ord(output_flags),
    }

    text_flags = dict()
    for flags_type, flags in raw_flags.items():
        found_flags = [
            descr if flag & flags else ""
            for flag, descr in info_mapping.items()
            # Do not show unset flag
            if flag & flags
        ]
        raw_flags[flags_type] = found_flags
        text_flags[flags_type] = ",".join(found_flags)

    text = ", ".join(
        [f"{k}: {v if v else 'None'}" for k, v in text_flags.items()]
    )
    return raw_flags, text


def parse_info_mode_combos(payload):
    """Provide mode combination info for Powered Up devices

    .. warning:: INFO_MODE_PLUS_8 is not and should not be tested,
        because this command is always tied to the mode 0.
        It is not the case in this code, where mode is the result of
        INFO_MODE_PLUS_8 suppression.

     .. seealso::
        https://github.com/pybricks/technical-info/blob/master/uart-protocol.md#info_mode_combos

    :param payload: List of chars found in the data part of a message.
    :type payload: <list <str>>
    :return: Raw data parsed (modes in the given combo mode) and human readable
        formatted data
    :rtype <tuple <list <int>>, <str>>
    """
    # Convert sequence to bytes
    payload = "".join(payload).encode("latin1")
    # data values are 16-bit flags
    # The number of data values will depend on the device
    all_flags = unpack("<" + "h" * (len(payload) // 2), payload)

    # Each bit representing a mode
    found_flags = [
        i if (1 << i) & flags else ""
        for flags in all_flags
        for i in range(16)
        if (1 << i) & flags
    ]
    text = ", ".join(map(str, found_flags))
    return found_flags, text


def parse_info_format(payload):
    """Provide info about the data format and presentation for a mode

    .. todo::
        <data-sets> times the size of <format> must not exceed 32-byte.
        For example 32 8-bit values are allowed, but only 8 32-bit values are allowed.
        => assert data_sets * data_format size <= 32 bytes

    .. seealso::
        https://github.com/pybricks/technical-info/blob/master/uart-protocol.md#info_format

    :param payload: List of chars found in the data part of a message.
    :type payload: <list <str>>
    :return: Raw data parsed (data_sets, data_format, figures, decimals)
        and human readable formatted data
    :rtype <tuple <tuple <int>>, <str>>
    """
    data_formats = {
        0x00: "DATA8",  # 8-bit signed integer
        0x01: "DATA16",  # 16-bit little-endian signed integer
        0x02: "DATA32",  # 32-bit little-endian signed integer
        0x03: "DATAF",  # 32-bit little-endian IEEE 754 floating point
    }
    data_formats2 = {
        0x00: "int8",  # 8-bit signed integer
        0x01: "int16",  # 16-bit little-endian signed integer
        0x02: "int32",  # 32-bit little-endian signed integer
        0x03: "float32",  # 32-bit little-endian IEEE 754 floating point
    }
    data_sets, data_format, figures, decimals = map(ord, payload)
    # text = f"data_sets: {data_sets}, data_format: {data_formats[data_format]}, figures: {figures}, decimals: {decimals}"
    text = f"{data_sets} {data_formats2[data_format]}, each {figures} chars, {decimals} decimals"

    return (data_sets, data_format, figures, decimals), text


def parse_cmd_type(payload):
    """Get the type of device

    .. seealso::
        https://github.com/pybricks/technical-info/blob/master/uart-protocol.md#cmd_type

    :param payload: List of chars found in the data part of a message.
    :type payload: <list <str>>
    :return: Raw data parsed (device id) and human readable formatted data
    :rtype <tuple <int>, <str>>
    """
    type_id = ord(payload[0])
    return type_id, "Type ID: " + hex(type_id)


def parse_cmd_speed(payload):
    """Get the baud rate to use after synchronizing

    .. seealso::
        https://github.com/pybricks/technical-info/blob/master/uart-protocol.md#cmd_speed

    :param payload: List of chars found in the data part of a message.
    :type payload: <list <str>>
    :return: Raw data parsed (baud rate) and human readable formatted data
    :rtype <tuple <int>, <str>>
    """
    # Convert sequence to bytes
    payload = "".join(payload).encode("latin1")

    speed = unpack("<l", payload)[0]
    return speed, f"Speed: {speed}"


def parse_cmd_version(payload):
    """Get the firmware and hardware versions

    .. seealso::
        https://github.com/pybricks/technical-info/blob/master/uart-protocol.md#cmd_version

    :param payload: List of chars found in the data part of a message.
    :type payload: <list <str>>
    :return: Raw data parsed and human readable formatted data
        Each version (fw-version/hw-version) is composed of 4 digits:
        Major, minor and bug fix revision values, then, the build number
    :rtype <tuple <dict <str>: <tuple <int>, <int>, <int>, <int>>>, <str>>
    """
    # Convert sequence to bytes
    payload = "".join(payload).encode("latin1")

    ret = unpack("<HBBHBB", payload)

    raw_versions = {
        "fw-version": ret[:3],
        "hw-version": ret[3:],
    }
    versions = dict()
    for version_name, raw_val in raw_versions.items():
        build, bug_fix, major_minor = raw_val

        # Get most significant 4 bits
        major = (major_minor >> 4) & 0x0F

        # Get less significant 4 bits
        minor = major_minor & 0x0F

        versions[version_name] = (major, minor, bug_fix, build)

    text = ", ".join([f"{k}: " + ".".join(map(str, v)) for k, v in versions.items()])
    return versions, text


def parse_cmd_modes(payload):
    """Get how many modes this device has

    .. seealso::
        https://github.com/pybricks/technical-info/blob/master/uart-protocol.md#cmd_modes

    :param payload: List of chars found in the data part of a message.
    :type payload: <list <str>>
    :return: Raw data parsed and human readable formatted data
    :rtype <tuple <dict <str>:<int>>, <str>>
    """
    expected_names = ("modes", "views", "modes2", "views2")

    # Increase by 1 all values
    # By default there are 8 modes starting from 0 to 7),
    # or 15 modes for modes2 & views2 starting from 0 to 15)
    modes = dict(zip(expected_names, [val + 1 for val in map(ord, payload)]))
    text = ", ".join([f"{k}: {v}" for k, v in modes.items()])
    return modes, text
