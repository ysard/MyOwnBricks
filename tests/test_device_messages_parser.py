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
import pytest
from my_own_bricks.device_messages_parser import *  # parse_message


def strip_info_message(message):
    """Return the payload from the given message

    header, info_type and checksum are removed

    :return: List of chars in the payload
    :rtype: <list <str>>
    """
    return message[2:-1]


def strip_cmd_message(message):
    """Return the payload from the given message

    header, and checksum are removed

    :return: List of chars in the payload
    :rtype: <list <str>>
    """
    return message[1:-1]


@pytest.mark.parametrize(
    "expected, message",
    [
        # INFO_NAME
        (("SPEC 1", "'SPEC 1'"), "\x98\x20\x53\x50\x45\x43\x20\x31\x00\x00\x53"),
        # INFO_NAME Newer I/O devices with flags after the name
        (
            ("CALIB", "'CALIB'"),
            "\xA1\x20\x43\x41\x4C\x49\x42\x00\x40\x40\x00\x00\x04\x84\x00\x00\x00\x00\xBB",
        ),
        # INFO_UNITS
        (("CNT", "'CNT'"), "\x92\x04\x43\x4e\x54\x00\x30"),
    ],
)
def test_parse_info_name(expected, message):
    """Get the content of text payload

    Used for INFO_NAME & INFO_UNITS.
    """
    expected_raw_text, expected_text = expected

    payload = strip_info_message(message)
    raw_text, text = parse_info_name(payload)

    assert expected_raw_text == raw_text
    assert expected_text == text


@pytest.mark.parametrize(
    "expected, percentage, message",
    [
        # INFO_RAW
        (
            ((0.0, 100.0), "0.0 to 100.0"),
            False,
            "\x9a\x01\x00\x00\x00\x00\x00\x00\xc8\x42\xee",
        ),
        # INFO_SI
        (
            ((0.0, 100.0), "0.0 to 100.0"),
            False,
            "\x9a\x03\x00\x00\x00\x00\x00\x00\xc8\x42\xec",
        ),
        # INFO_PCT
        (
            ((0.0, 100.0), "0.0% to 100.0%"),
            True,
            "\x9a\x02\x00\x00\x00\x00\x00\x00\xc8\x42\xed",
        ),
    ],
)
def test_parse_info_raw(expected, percentage, message):
    """Provide info about the scaling (min/max) of the raw sensor (legacy)

    Used for INFO_RAW, INFO_SI and INFO_PCT (with percentage param set to True).
    """
    expected_raw_text, expected_text = expected

    payload = strip_info_message(message)
    raw_text, text = parse_info_raw(payload, percentage=percentage)

    assert expected_raw_text == raw_text
    assert expected_text == text


@pytest.mark.parametrize(
    "expected, message",
    [
        # INFO_MAPPING without output flag
        (
            (
                {"input_flags": ["Relative"], "output_flags": []},
                "input_flags: Relative, output_flags: None",
            ),
            "\x8a\x05\x08\x00\x78",
        ),
        # No additional info mapping flag
        (
            (
                {"input_flags": [], "output_flags": []},
                "input_flags: None, output_flags: None",
            ),
            "\x89\x25\x00\x00\x53",
        ),
        # input_flags: None, output_flags: Absolute
        (
            (
                {"input_flags": [], "output_flags": ["Absolute"]},
                "input_flags: None, output_flags: Absolute",
            ),
            "\x8B\x05\x00\x10\x61",
        ),
    ],
)
def test_parse_info_mapping(expected, message):
    """ "Provide mode mapping info for Powered Up devices

    Used for INFO_MAPPING.
    """
    expected_raw_text, expected_text = expected

    payload = strip_info_message(message)
    raw_text, text = parse_info_mapping(payload)

    assert expected_raw_text == raw_text
    assert expected_text == text


@pytest.mark.parametrize(
    "expected, message",
    [
        (([0, 1, 2, 3, 6], "0, 1, 2, 3, 6"), "\x88\x06\x4f\x00\x3e"),
    ],
)
def test_parse_info_mode_combos(expected, message):
    """ "Provide mode combination info for Powered Up devices

    Used for INFO_MODE_COMBOS.
    """
    expected_raw_text, expected_text = expected

    payload = strip_info_message(message)
    raw_text, text = parse_info_mode_combos(payload)

    assert expected_raw_text == raw_text
    assert expected_text == text


@pytest.mark.parametrize(
    "expected, message",
    [
        (
            ((1, 2, 4, 0), "1 int32, each 4 chars, 0 decimals"),
            "\x92\x80\x01\x02\x04\x00\x30",
        ),
        (
            ((3, 0, 3, 0), "3 int8, each 3 chars, 0 decimals"),
            "\x92\x80\x03\x00\x03\x00\xED",
        ),
    ],
)
def test_parse_info_format(expected, message):
    """ "Provide info about the data format and presentation for a mode

    Used for INFO_FORMAT.
    """
    expected_raw_text, expected_text = expected

    payload = strip_info_message(message)
    raw_text, text = parse_info_format(payload)

    assert expected_raw_text == raw_text
    assert expected_text == text


def test_parse_cmd_type():
    """Get the type id of device"""
    expected_raw_text, expected_text = (61, "Type ID: 0x3d")
    message = "\x40\x3D\x82"

    payload = strip_cmd_message(message)
    raw_text, text = parse_cmd_type(payload)

    assert expected_raw_text == raw_text
    assert expected_text == text


def test_parse_cmd_speed():
    """Get the baud rate to use after synchronizing"""
    expected_raw_text, expected_text = (115200, "Speed: 115200")
    message = "\x52\x00\xC2\x01\x00\x6E"

    payload = strip_cmd_message(message)
    raw_text, text = parse_cmd_speed(payload)

    assert expected_raw_text == raw_text
    assert expected_text == text


def test_parse_cmd_version():
    """Get the firmware and hardware versions"""
    expected_raw_text, expected_text = (
        {"fw-version": (1, 0, 0, 0), "hw-version": (1, 0, 0, 0)},
        "fw-version: 1.0.0.0, hw-version: 1.0.0.0",
    )
    message = "\x5F\x00\x00\x00\x10\x00\x00\x00\x10\xA0"

    payload = strip_cmd_message(message)
    raw_text, text = parse_cmd_version(payload)

    assert expected_raw_text == raw_text
    assert expected_text == text


@pytest.mark.parametrize(
    "expected, message",
    [
        # Doc pybricks
        (
            (
                {"modes": 8, "views": 8, "modes2": 11, "views2": 8},
                "modes: 8, views: 8, modes2: 11, views2: 8",
            ),
            "\x51\x07\x07\x0a\x07\xa3",
        ),
        # Color & distance sensor
        (
            (
                {"modes": 8, "modes2": 10, "views": 8, "views2": 1},
                "modes: 8, views: 8, modes2: 10, views2: 1",
            ),
            "\x51\x07\x07\x09\x00\xA7",
        ),
        # Tilt sensor
        (({"modes": 4, "views": 3}, "modes: 4, views: 3"), "\x49\x03\x02\xB7"),
    ],
)
def test_parse_cmd_modes(expected, message):
    """Get how many modes this device has"""
    expected_raw_text, expected_text = expected

    payload = strip_cmd_message(message)
    raw_text, text = parse_cmd_modes(payload)

    assert expected_raw_text == raw_text
    assert expected_text == text


def test_parse_messages():
    """Functional test for the main messages parser applied to a full sequence of bytes"""
    # Tilt sensor init seq
    data = "\x40\x22\x9D\x49\x03\x02\xB7\x52\x00\xC2\x01\x00\x6E\x5F\x00\x00\x00\x10\x00\x00\x00\x10\xA0\x9B\x00\x4C\x50\x46\x32\x2D\x43\x41\x4C\x6F\x9B\x01\x00\x00\x34\xC2\x00\x00\x34\x42\xE5\x9B\x02\x00\x00\xC8\xC2\x00\x00\xC8\x42\xE6\x9B\x03\x00\x00\x34\xC2\x00\x00\x34\x42\xE7\x93\x04\x43\x41\x4C\x00\x26\x8B\x05\x10\x00\x61\x93\x80\x03\x00\x03\x00\xEC\xA2\x00\x4C\x50\x46\x32\x2D\x43\x52\x41\x53\x48\x00\x00\x00\x00\x00\x00\x53\x9A\x01\x00\x00\x00\x00\x00\x00\xC8\x42\xEE\x9A\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xED\x9A\x03\x00\x00\x00\x00\x00\x00\xC8\x42\xEC\x92\x04\x43\x4E\x54\x00\x30\x8A\x05\x10\x00\x60\x92\x80\x03\x00\x03\x00\xED\xA1\x00\x4C\x50\x46\x32\x2D\x54\x49\x4C\x54\x00\x00\x00\x00\x00\x00\x00\x1E\x99\x01\x00\x00\x00\x00\x00\x00\x20\x41\x06\x99\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xEE\x99\x03\x00\x00\x00\x00\x00\x00\x20\x41\x04\x91\x04\x44\x49\x52\x00\x35\x89\x05\x04\x00\x77\x91\x80\x01\x00\x02\x00\xED\xA0\x00\x4C\x50\x46\x32\x2D\x41\x4E\x47\x4C\x45\x00\x00\x00\x00\x00\x00\x5B\x98\x01\x00\x00\x34\xC2\x00\x00\x34\x42\xE6\x98\x02\x00\x00\xC8\xC2\x00\x00\xC8\x42\xE5\x98\x03\x00\x00\x34\xC2\x00\x00\x34\x42\xE4\x90\x04\x44\x45\x47\x00\x2D\x88\x05\x10\x00\x62\x90\x80\x02\x00\x03\x00\xEE"
    # Color & distance sensor, INFO_NAME, Mode 9: (CALIB)
    # Used to test INFO_MODE_PLUS_8 detection
    data += "\xA1\x20\x43\x41\x4C\x49\x42\x00\x40\x40\x00\x00\x04\x84\x00\x00\x00\x00\xBB"
    # LEGO EV3 Gyro sensor, CMD_WRITE
    # Used to test extra init sequence messages but that belongs to LUMP_MSG_TYPE_CMD
    data += "\x44\x17\xac"
    # Color & distance sensor, LUMP_MSG_TYPE_DATA
    # Used to test extra init sequence messages that belongs to LUMP_MSG_TYPE_DATA
    data += "\xC0\x00\x3f"

    expected_analysis = (
        34,
        {"modes": 4, "views": 3},
        115200,
        {"fw-version": (1, 0, 0, 0), "hw-version": (1, 0, 0, 0)},
        "LPF2-CAL",
        (-45.0, 45.0),
        (-100.0, 100.0),
        (-45.0, 45.0),
        "CAL",
        {"input_flags": ["Absolute"], "output_flags": []},
        (3, 0, 3, 0),
        "LPF2-CRASH",
        (0.0, 100.0),
        (0.0, 100.0),
        (0.0, 100.0),
        "CNT",
        {"input_flags": ["Absolute"], "output_flags": []},
        (3, 0, 3, 0),
        "LPF2-TILT",
        (0.0, 10.0),
        (0.0, 100.0),
        (0.0, 10.0),
        "DIR",
        {"input_flags": ["Discrete"], "output_flags": []},
        (1, 0, 2, 0),
        "LPF2-ANGLE",
        (-45.0, 45.0),
        (-100.0, 100.0),
        (-45.0, 45.0),
        "DEG",
        {"input_flags": ["Absolute"], "output_flags": []},
        (2, 0, 3, 0),
        # Color & distance sensor, INFO_NAME, Mode 9: (CALIB)
        'CALIB',
        # LEGO EV3 Gyro sensor, CMD_WRITE
        ['\x17'],
        # Color & distance sensor, LUMP_MSG_TYPE_DATA
        None,
    )

    expected_descr = (
        "LUMP_CMD_TYPE",
        "LUMP_CMD_MODES",
        "LUMP_CMD_SPEED",
        "LUMP_CMD_VERSION",
        "INFO_NAME",
        "INFO_RAW",
        "INFO_PCT",
        "INFO_SI",
        "INFO_UNITS",
        "INFO_MAPPING",
        "INFO_FORMAT",
        "INFO_NAME",
        "INFO_RAW",
        "INFO_PCT",
        "INFO_SI",
        "INFO_UNITS",
        "INFO_MAPPING",
        "INFO_FORMAT",
        "INFO_NAME",
        "INFO_RAW",
        "INFO_PCT",
        "INFO_SI",
        "INFO_UNITS",
        "INFO_MAPPING",
        "INFO_FORMAT",
        "INFO_NAME",
        "INFO_RAW",
        "INFO_PCT",
        "INFO_SI",
        "INFO_UNITS",
        "INFO_MAPPING",
        "INFO_FORMAT",
        # Color & distance sensor, INFO_NAME, Mode 9: (CALIB)
        "INFO_NAME",
        # LEGO EV3 Gyro sensor, CMD_WRITE
        "LUMP_CMD_WRITE",
        # Color & distance sensor, LUMP_MSG_TYPE_DATA
        "LUMP_MSG_TYPE_DATA",
    )

    messages = [msg_analysis for msg_analysis in parse_messages(data)]

    assert len(messages) == 35

    unzip = lambda l: list(zip(*l))

    msgs_descr, msgs_data, msgs_analysis = unzip(messages)

    print(msgs_analysis)
    assert expected_analysis == msgs_analysis

    print(msgs_descr)
    assert expected_descr == msgs_descr


def test_parse_messages_bad_checksum(capsys):
    """Test with bad checksum in message
    For now, it should output a message on stdout
    """
    # Bad checksum (0x00 instead of 0x3f)
    data = "\xC0\x00\x00"

    messages = [msg_analysis for msg_analysis in parse_messages(data)]

    assert len(messages) == 1

    captured = capsys.readouterr()

    assert "Bad checksum! Found vs expected: 0x0 0x3f\n" in captured.out
