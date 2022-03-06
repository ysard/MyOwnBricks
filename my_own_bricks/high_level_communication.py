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
# Standard imports
"""Wrapper tools for an easy communication with devices via LEGO UART protocol."""
import time

# Custom imports
from my_own_bricks.uart_handler import autoconnect
from my_own_bricks.header_checksum import parse_device_header, get_cheksum
from my_own_bricks.messages import get_hex_msg, forge_mode_msg, \
    forge_write_mode_msg
from my_own_bricks.commons import LOOP_TIMEOUT


def get_device_messages():
    """Coroutine for device responses

    It's a wrapper that fully encapsulates the LEGO UART protocol. The user the
    user only has to provide the desired modes and eventually the data to be
    sent to the device.
    This function is blocking until a response is received or the device is ready
    to accept a new query.

    .. seealso:: LOOP_TIMEOUT: The smaller this interval, the greater the number
        of requests that can be sent. The interval should not exceed 200ms
        otherwise the device will disconnect itself.

    :Example: Usage:

    >>> messages = get_device_messages()
    >>> msg = next(messages)
    >>> while True:
    >>>     if msg:
    >>>         # Get response from device
    >>>         header, response = msg
    >>>         parse_response(header, response)
    >>>
    >>>     # Send command to coroutine
    >>>     # messages.send((5, 0))
    >>>     msg = messages.send(6)

    :return: Coroutine that yields a tuple with (header, response) and accepts
        values: `mode` and `payload` for write queries;
        `mode` can be set alone if the query is read-only.
    """
    serial_handler = autoconnect()
    serial_handler.timeout = LOOP_TIMEOUT

    # Keep-alive packet: Send NACK to force the device response
    serial_handler.write(b"\x02")

    # Now wait the first header
    multipart_packet = False
    nack_timestamp = time.time()
    command_timestamp = nack_timestamp
    next_query = None
    while True:
        print(time.time() - nack_timestamp, "S")
        if time.time() - nack_timestamp >= 0.1:
            # Keep-alive packet every 100ms: Send NACK to force the device response
            serial_handler.write(b"\x02")
            nack_timestamp = time.time()
            print("> NACK")

        if not multipart_packet and next_query:
            current_time = time.time()
            if current_time - command_timestamp > 0.01:
                # Do not send new query if almost 10ms are not elapsed
                # => slow down query number

                # print("PREPARE TO SEND RET:", next_query)
                # Handle query modes
                if isinstance(next_query, tuple):
                    next_mode, payload = next_query
                    # Write query
                    msg = forge_write_mode_msg(next_mode, data=payload)
                else:
                    # Read only query
                    msg = forge_mode_msg(next_query)  # query = mode in this case

                # Send query
                serial_handler.write(msg)
                command_timestamp = current_time
                print(">", get_hex_msg(msg))

        # Parse header to get the size of the message
        raw_val = serial_handler.read(1)  # 100ms timeout
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
        print(time.time() - nack_timestamp)

        # Print data fields (no header, no checksum)
        hex_values = get_hex_msg(response[:-1])
        print("<\t", hex_values)

        # LUMP_CMD_EXT_MODE packet: expect a 2nd part
        # Do not send another query between those 2 packets
        multipart_packet = (header == 0x46)

        # Check checksum
        checksum = get_cheksum(bytearray(response[:-1])) ^ header
        try:
            found_checksum = response[-1]
        except IndexError:
            print(time.time() - nack_timestamp)
            print("Disconnected!")
            exit()
        if checksum != found_checksum:  # pragma: no cover (impossible to test)
            raise ValueError(
                "Bad checksum! <{}> vs <{}>".format(
                    hex(checksum), hex(found_checksum)
                )
            )

        if not multipart_packet:
            # Multipart packets are acknowledgements for extended mode switch
            # i.e. to prepare the hub to distinct mode 0 and mode 8.
            # TODO: maybe this extended_mode information should be also yelled...

            # Ready to accept and send a new query
            next_query = yield header, response
