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
"""Test high level and handy functions for UART communication"""
from unittest.mock import patch
import pytest

from my_own_bricks.high_level_communication import get_device_messages
from my_own_bricks.commons import COLOR_MAP


def test_get_device_messages():
    """Just send LED color and expect it to be changed on the device side
    Mainly test coroutine implementation.
    """
    messages = get_device_messages()
    msg = next(messages)
    i = 0
    responses = set()
    while True:
        if msg:
            # Get response from device
            header, response = msg
            print("iteration:", i, "header:", header, "response:", response)
            assert header == 0xd0  # Mode 8 + payload size 6
            responses.add(response[2])
            i += 1

        # Limit packets nb, this should be sufficient to get our response among NACKs
        if i == 5:
            # Reset to original state and exit test loop
            messages.send((5, 0))
            break
        # Send command to coroutine
        if i < 2:
            messages.send((5, 9))  # Set LED color to RED
        else:
            messages.send(8)  # Mode 8, just read 4 values

        msg = next(messages)

    print("LED color responses", responses)
    assert len(responses) == 2  # 0 (initial state), 9 (new LED color)
    assert 9 in responses  # LED color set


@patch('my_own_bricks.high_level_communication.LOOP_TIMEOUT', 0.5)
def test_mock_timeout():
    messages = get_device_messages()
    _ = next(messages)
    # Code should exit because device disconnects itself
    with pytest.raises(SystemExit) as pytest_wrapped_e:
        while True:
            _ = next(messages)

    assert pytest_wrapped_e.type == SystemExit
