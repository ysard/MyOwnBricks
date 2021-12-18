# MyOwnBricks is a library for the emulation of PoweredUp sensors on microcontrollers
# Copyright (C) 2021 Ysard - <ysard@users.noreply.github.com>
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
"""Test UART connection"""
import time
import pytest

# Custom imports
import my_own_bricks.uart_handler as uart_handler
from my_own_bricks.uart_handler import connect_to_hub, autoconnect
from my_own_bricks.header_checksum import parse_device_header, get_cheksum
from my_own_bricks.messages import get_hex_msg, forge_mode_msg, forge_write_mode_msg


@pytest.yield_fixture()
def bad_serial_port():
    """Set & reset wrong serial port"""
    # Set wrong serial port
    original_port = uart_handler.SERIAL_PORT
    uart_handler.SERIAL_PORT = "/dev/ttyUSB99"

    yield None

    # Reset serial port
    uart_handler.SERIAL_PORT = original_port


@pytest.yield_fixture()
def unexpected_init_sequence():
    """Set & reset wrong expected init sequence from device"""
    # Set wrong serial port
    original_seq = uart_handler.colour_expected_init_seq
    uart_handler.colour_expected_init_seq = b"\xFF"

    yield None

    # Reset serial port
    uart_handler.colour_expected_init_seq = original_seq


def test_modes():
    """Test most common modes of Color & Distance sensor

    This test must be conducted with the test sketch executed on the device.
    Mode 2 (count occurrences) is not tested.
    """
    found_messages = list()
    expected_messages = [
        (0x46, b'\x08'),  # NACK response
        (0xd0, b'\x03\x0A\x00\x01'),  # Mode 8 response, sensor color, distance, LED color, reflected
        (0xc0, b'\x00'),  # Mode 0; LED color
        (0x46, b'\x08'), (0xd0, b'\x03\x0A\x00\x01'),  # NACK response
        (0xc1, b'\x0A'),  # Mode 1; distance
        (0x46, b'\x08'), (0xd0, b'\x03\x0A\x00\x01'),  # NACK response
        (0xc3, b'\x01'),  # Mode 3; reflected
        (0x46, b'\x08'), (0xd0, b'\x03\x0A\x00\x01'),  # NACK response
        (0xc4, b'\x02'),  # Mode 4; ambient
        (0x46, b'\x08'), (0xd0, b'\x03\x0A\x00\x01'),  # NACK response
        (0x46, b'\x08'),  # NACK response
        (0xd0, b'\x03\x0A\x42\x01'),  # Mode 5 effect: LED color change to 0x42
        (0xde, b'\x04\x00\x05\x00\x06\x00\x00\x00'),  # Mode 6; RGB
        (0x46, b'\x08'), (0xd0, b'\x03\x0A\x42\x01'),  # NACK response
        # Mode 7; No visible effect
    ]
    modes = iter((0, 1, 3, 4, 5, 6, 7, 8))

    # Begin communication
    serial_handler = autoconnect()
    assert serial_handler is not None
    # Small adjustment in place of the commons.LOOP_TIMEOUT value
    serial_handler.timeout = 0.09

    # Keep-alive packet: Send NACK to force the device response
    # serial_handler.write(b"\x02")

    # Now wait the first header
    i = 0
    while True:
        i += 1
        # Keep-alive packet: Send NACK to force the device response
        serial_handler.write(b"\x02")

        # Parse header to get the size of the message
        raw_val = serial_handler.read(1)
        if not raw_val:
            # Nothing to parse
            continue
        header = ord(raw_val)
        print("<\t", end="")
        msg_type, cmd_mode, msg_size = parse_device_header(header)

        # Get the full message (minus the header)
        # NOTE: If we are here, data is already pending after header reception
        # We shouldn't risk any timeout and partial packet issues...
        response = serial_handler.read(msg_size - 1)

        # Print data fields (no header, no checksum)
        hex_values = get_hex_msg(response[:-1])
        print("<\t", hex_values)

        # LUMP_CMD_EXT_MODE packet: expect a 2nd part
        # Do not send another query between those 2 packets
        # multipart_packet = (header == 0x46)

        # Check checksum
        checksum = get_cheksum(bytearray(response[:-1])) ^ header
        try:
            found_checksum = response[-1]
        except IndexError:
            print(time.time() - NACK_timestamp)
            print("Disconnected!")
            raise ValueError
        if checksum != found_checksum:
            print("Bad checksum !", hex(checksum), "vs", hex(found_checksum))
            raise ValueError()

        # Keep full response for analysis
        # Do not test checksum (already tested above)
        # Human readable: (hex(header), hex_values)
        found_messages.append((header, response[:-1]))

        # Prepare next query
        try:
            next_mode = next(modes)
        except StopIteration:
            # No more mode to test
            if i >= 19:
                # All expected packets are received
                # Restore previous device state
                serial_handler.write(forge_write_mode_msg(5, data=0x00))
                break
            else:
                # Expect more packets...
                continue

        if next_mode in (5, 7):
            # Write query
            msg = forge_write_mode_msg(next_mode, data=0x42)
        else:
            # Read only query
            msg = forge_mode_msg(next_mode)

        # Send query
        print(">", get_hex_msg(msg))
        serial_handler.write(msg)

    print(found_messages)
    assert found_messages == expected_messages


def test_autoconnect_bad_port(bad_serial_port):
    """Test if serial port is not available"""
    # Exception is expected
    with pytest.raises(IOError, match=r"Serial port .* not available!"):
        found_handler = autoconnect()


def test_autoconnect_missing_device(unexpected_init_sequence):
    """Test if Serial device is not available; should lead to missing device"""
    with pytest.raises(IOError, match="Device not found on Serial line"):
        found_handler = autoconnect()
