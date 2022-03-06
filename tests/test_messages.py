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
"""Test forge read/write messages"""
import pytest
from my_own_bricks.messages import forge_mode_msg, forge_write_mode_msg


def test_forge_mode_msg():
    found = forge_mode_msg(9)
    expected = bytearray(b"\x43\x09\xb5")
    assert found == expected


def test_forge_write_mode_msg():
    found = forge_write_mode_msg(5, data=0x9)
    expected = bytearray(b"\x46\x00\xb9\xc5\x09\xcc")
    assert found == expected

    with pytest.raises(AssertionError):
        forge_write_mode_msg(0)
