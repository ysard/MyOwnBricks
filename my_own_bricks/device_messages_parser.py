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
"""
# Standard imports
from struct import unpack
from functools import partial

# Custom imports
from my_own_bricks.header_checksum import parse_device_header, get_cheksum


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
        flags_start_pos = payload.index("\x00")
        # Skip null char after the end of the name
        flags = [value for value in payload[flags_start_pos + 1:]]
        # code toremove trailing null values
        while flags[-1] in ("\x00", 0x00):
            flags.pop()
        print("New I/O device with flags NOT parsed: ", flags)

        # Keep only name part
        payload = payload[:flags_start_pos]
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

        - output_flags: for writable value on the device;
        - input_flags: for readable value from the device.
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

    text = ", ".join([f"{k}: {v if v else 'None'}" for k, v in text_flags.items()])
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

        Ex: (8, 1, 4, 0): 8 int16 values, 4 chars to display each, 0 decimal

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
    """Get the type id of device

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


def parse_messages(stream):
    """Main parser method for messages coming from a device

    Support is focused on LUMP_MSG_TYPE_INFO & LUMP_MSG_TYPE_CMD messages used
    in init sequences.

    .. note:: Support for CMD_SELECT, CMD_EXT_MODE & CMD_WRITE messages is basic
        (the raw payload is returned) since they are commands sent by the host.

    :return: Generator of message descriptions (info_type converted into
        human readable format for LUMP_MSG_TYPE_INFO messages, cmd otherwise).
        The message itself and analysed data in its payload are also yielded.
    :rtype: <generator <tuple <str>, <list>, <object>>
    """
    # Bit flag used in powered up devices to indicate that the
    # mode is 8 + the mode specified in the first byte
    INFO_MODE_PLUS_8 = 0x20

    # Mapping of parsers for LUMP_MSG_TYPE_INFO messages
    info_type_mapping = {
        0: parse_info_name,
        1: parse_info_raw,
        2: partial(parse_info_raw, percentage=True),
        3: parse_info_raw,
        4: parse_info_name,
        5: parse_info_mapping,
        6: parse_info_mode_combos,
        0x80: parse_info_format,
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

    # Mapping of parsers for LUMP_MSG_TYPE_CMD messages
    cmd_type_mapping = {
        "LUMP_CMD_TYPE": parse_cmd_type,
        "LUMP_CMD_MODES": parse_cmd_modes,
        "LUMP_CMD_SPEED": parse_cmd_speed,
        # "LUMP_CMD_SELECT": parse_cmd_select,
        # "LUMP_CMD_WRITE": parse_cmd_write,
        # "LUMP_CMD_UNK1": None,
        # "LUMP_CMD_EXT_MODE": parse_cmd_ext_mode,
        "LUMP_CMD_VERSION": parse_cmd_version,
    }

    stream = iter(stream)

    while True:
        # Rebuild the current message
        message = list()
        try:
            header = ord(next(stream))
        except StopIteration:
            # Yeah, I am a coward, I consider the packets to be complete including checksums.
            # This exception *should not* occurs elsewhere.
            break

        msg_type, mode, cmd, msg_size = parse_device_header(header)
        message.append(header)

        payload_size = msg_size - 2  # Remove checksum & header from size
        if msg_type == "LUMP_MSG_TYPE_CMD":
            # Note: The range explicitly removes the checksum from the payload size
            payload = [next(stream) for _ in range(payload_size)]

            raw_data, text = cmd_type_mapping.get(cmd, lambda x: (x, "Not supported"))(
                payload
            )

            print("\t" + cmd, text)

        elif msg_type == "LUMP_MSG_TYPE_INFO":
            raw_info_type = ord(next(stream))
            message.append(raw_info_type)

            if raw_info_type & INFO_MODE_PLUS_8 != 0:
                # INFO_MODE_PLUS_8 is set: mode should be updated
                mode += 8
            # Remove mode flag from the byte
            info_type = raw_info_type & ~INFO_MODE_PLUS_8

            # Remove previous byte from the payload size
            payload_size -= 1
            # Note: The range explicitly removes the checksum from the payload size
            payload = [next(stream) for _ in range(payload_size)]

            msg_descr = info_type_descr_mapping.get(info_type, "UKN")

            raw_data, text = info_type_mapping.get(
                info_type, lambda x: (x, "Not supported")
            )(payload)

            print("\tMODE", mode, msg_descr, text)

        else:
            # LUMP_MSG_TYPE_SYS, LUMP_MSG_TYPE_DATA messages
            payload = [next(stream) for _ in range(payload_size)]
            raw_data = text = None
            # Message description corresponds to the message type here
            msg_descr = msg_type
            print("\t" + msg_type,"Not supported", payload)

        # Throw the last byte (checksum)
        found_checksum = ord(next(stream))
        payload_values = list(map(ord, payload))
        message += payload_values
        # Compare checksums
        expected_checksum = get_cheksum(message)

        if found_checksum != expected_checksum:
            print("ERROR: Bad checksum! Found vs expected:", hex(found_checksum), hex(expected_checksum))

        message.append(expected_checksum)

        # Use msg_descr for LUMP_MSG_TYPE_INFO and all not supported types
        yield  cmd if msg_type == "LUMP_MSG_TYPE_CMD" else msg_descr, message, raw_data
