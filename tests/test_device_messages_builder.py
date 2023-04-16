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
"""Test the build of packets from a device point of view"""
import pytest
from my_own_bricks.device_messages_builder import *


def test_forge_info_name():
    """"""
    found = forge_info_name("")
    expected = b"\x00"
    assert expected == found

    found = forge_info_name("CALIB")
    expected = b"CALIB"
    assert expected == found

    # See the null char between
    found = forge_info_name(("CALIB", ["@", "@", "\x00", "\x00", "\x04", "\x84"]))
    expected = b"CALIB\x00@@\x00\x00\x04\x84"
    assert expected == found


def test_forge_info_raw():

    found = forge_info_raw((0, 100))
    expected = b"\x00\x00\x00\x00\x00\x00\xc8B"
    assert expected == found

    # No diff for int/float (everything is converted to float)
    found = forge_info_raw((0.0, 100.0))
    assert expected == found


def test_forge_info_mapping(capsys):
    # input_flags: 'N/A', 'N/A' but in numbers version
    found = forge_info_mapping({"input_flags": [1 << 0, 1 << 1], "output_flags": []})
    expected = b"\x03\x00"
    assert expected == found

    # N/A will be always replaced by 1 << 0 = 1
    # This will display a warning on stdout
    found = forge_info_mapping({"input_flags": ["N/A", "N/A"], "output_flags": []})
    expected = b"\x01\x00"
    assert expected == found

    captured = capsys.readouterr()
    assert (
        "There could be a problem in the identification of your N/A flag"
        in captured.out
    )

    found = forge_info_mapping({"input_flags": ["Absolute"], "output_flags": []})
    expected = b"\x10\x00"
    assert expected == found


def test_forge_info_mode_combos():
    found = forge_info_mode_combos((0, 1, 5, 6))
    expected = b"\x63\x00"
    assert expected == found


def test_forge_info_format():
    found = forge_info_format((3, 0, 3, 0))
    expected = b"\x03\x00\x03\x00"
    assert expected == found


def test_forge_cmd_type():
    found = forge_cmd_type(0x23)
    expected = b"\x23"
    assert expected == found


def test_forge_cmd_speed():
    found = forge_cmd_speed(115200)
    expected = b"\x00\xC2\x01\x00"
    assert expected == found


def test_forge_cmd_version():
    found = forge_cmd_version({"fw-version": (2, 0, 0, 0), "hw-version": (1, 0, 0, 0)})
    expected = b"\x00\x00\x00\x20\x00\x00\x00\x10"
    assert expected == found


def test_forge_cmd_modes():
    found = forge_cmd_modes({"modes": 4, "views": 3})
    expected = b"\x03\x02"
    assert expected == found

    found = forge_cmd_modes({"modes": 8, "views": 8, "modes2": 10, "views2": 1})
    expected = b"\x07\x07\x09\x00"
    assert expected == found


def test_forge_packets():
    """Test reconstruction of messages from formatted descriptions"""
    data = [
        # Color & distance sensor (new I/O device)
        (
            ("LUMP_MSG_TYPE_INFO", "INFO_NAME", 9),
            ("CALIB", ["@", "@", "\x00", "\x00", "\x04", "\x84"]),
        ),
        (
            ("LUMP_MSG_TYPE_INFO", "INFO_UNITS", 9),
            "",
        ),
        # Tilt sensor
        # LUMP_MSG_TYPE_INFO messages
        (
            ("LUMP_MSG_TYPE_INFO", "INFO_NAME", 3),
            "LPF2-CAL",
        ),
        (
            ("LUMP_MSG_TYPE_INFO", "INFO_RAW", 3),
            (-45.0, 45.0),
        ),
        (
            ("LUMP_MSG_TYPE_INFO", "INFO_PCT", 3),
            (-100.0, 100.0),
        ),
        (
            ("LUMP_MSG_TYPE_INFO", "INFO_SI", 3),
            (-45.0, 45.0),
        ),
        (
            ("LUMP_MSG_TYPE_INFO", "INFO_UNITS", 3),
            "CAL",
        ),
        (
            ("LUMP_MSG_TYPE_INFO", "INFO_MAPPING", 3),
            {"input_flags": ["Absolute"], "output_flags": []},
        ),
        (
            ("LUMP_MSG_TYPE_INFO", "INFO_FORMAT", 3),
            (3, 0, 3, 0),
        ),
        # Tilt sensor
        # LUMP_MSG_TYPE_CMD messages
        (
            ("LUMP_MSG_TYPE_CMD", "LUMP_CMD_TYPE", None),
            0x22,
        ),
        (
            ("LUMP_MSG_TYPE_CMD", "LUMP_CMD_MODES", None),
            {"modes": 4, "views": 3},
        ),
        (
            ("LUMP_MSG_TYPE_CMD", "LUMP_CMD_SPEED", None),
            115200,
        ),
        (
            ("LUMP_MSG_TYPE_CMD", "LUMP_CMD_VERSION", None),
            {"fw-version": (1, 0, 0, 0), "hw-version": (1, 0, 0, 0)},
        ),
    ]

    expected = [
        # Color & distance sensor (new I/O device)
        b"\xA1\x20\x43\x41\x4C\x49\x42\x00\x40\x40\x00\x00\x04\x84\x00\x00\x00\x00\xBB",
        b"\x81\x24\x00\x5A",
        # Tilt sensor
        # LUMP_MSG_TYPE_INFO messages
        b"\x9B\x00\x4C\x50\x46\x32\x2D\x43\x41\x4C\x6F",
        b"\x9B\x01\x00\x00\x34\xC2\x00\x00\x34\x42\xE5",
        b"\x9B\x02\x00\x00\xC8\xC2\x00\x00\xC8\x42\xE6",
        b"\x9B\x03\x00\x00\x34\xC2\x00\x00\x34\x42\xE7",
        b"\x93\x04\x43\x41\x4C\x00\x26",
        b"\x8B\x05\x10\x00\x61",
        b"\x93\x80\x03\x00\x03\x00\xEC",
        # Tilt sensor
        # LUMP_MSG_TYPE_CMD messages
        b"\x40\x22\x9D",
        b"\x49\x03\x02\xB7",
        b"\x52\x00\xC2\x01\x00\x6E",
        b"\x5F\x00\x00\x00\x10\x00\x00\x00\x10\xA0",
    ]

    for index, msg in enumerate(forge_packets(data)):
        # from messages import get_hex_msg
        hex_bytes = "".join("\\x{0:0{1}X}".format(i, 2) for i in msg)
        size = len(msg)
        text = f'SerialTTL.write("{hex_bytes}", {size}); '
        print("\t", text, sep="")

        assert isinstance(msg, bytearray)

        assert expected[index] == msg


def test_forge_packets_wrong_values():
    """Test not allowed value in mode with LUMP_MSG_TYPE_CMD messages"""
    data = [
        (
            ("LUMP_MSG_TYPE_CMD", "LUMP_CMD_TYPE", 0),
            34,
        ),
    ]
    with pytest.raises(
        AssertionError, match="'mode' should be None with LUMP_MSG_TYPE_CMD"
    ):
        list(forge_packets(data))
