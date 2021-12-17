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
"""
Attente rx pin à LOW
pause 100ms

Serial.begin(2400);
...
SW_F("\x04",1);
Serial.flush();

2 secondes d'attente pour que le hub envoie un ACK 0x04
passage à 115200 bauds
pause 10 ms

toutes les 200ms au max, un NACK doit être envoyé 0x02
"""
import os
import time
import serial

SERIAL_PORT = "/dev/ttyUSB0"
BAUDRATE_INIT = 2400
BAUDRATE = 115200

colour_expected_init_seq = b"\x00\x40\x25\x9A\x51\x07\x07\x0A\x07\xA3\x52\x00\xC2\x01\x00\x6E\x5F\x00\x00\x00\x10\x00\x00\x00\x10\xA0\x9A\x20\x43\x41\x4C\x49\x42\x00\x00\x00\x00\x9A\x21\x00\x00\x00\x00\x00\xFF\x7F\x47\x83\x9A\x22\x00\x00\x00\x00\x00\x00\xC8\x42\xCD\x9A\x23\x00\x00\x00\x00\x00\xFF\x7F\x47\x81\x92\x24\x4E\x2F\x41\x00\x69\x8A\x25\x10\x00\x40\x92\xA0\x08\x01\x05\x00\xC1\x99\x20\x44\x45\x42\x55\x47\x00\x00\x00\x17\x99\x21\x00\x00\x00\x00\x00\xC0\x7F\x44\xBC\x99\x22\x00\x00\x00\x00\x00\x00\xC8\x42\xCE\x99\x23\x00\x00\x00\x00\x00\x00\x20\x41\x24\x91\x24\x4E\x2F\x41\x00\x6A\x89\x25\x10\x00\x43\x91\xA0\x02\x01\x05\x00\xC8\x98\x20\x53\x50\x45\x43\x20\x31\x00\x00\x53\x98\x21\x00\x00\x00\x00\x00\x00\x7F\x43\x7A\x98\x22\x00\x00\x00\x00\x00\x00\xC8\x42\xCF\x98\x23\x00\x00\x00\x00\x00\x00\x7F\x43\x78\x90\x24\x4E\x2F\x41\x00\x6B\x88\x25\x00\x00\x52\x90\xA0\x04\x00\x03\x00\xC8\x9F\x00\x49\x52\x20\x54\x78\x00\x00\x00\x77\x9F\x01\x00\x00\x00\x00\x00\xFF\x7F\x47\xA6\x9F\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xE8\x9F\x03\x00\x00\x00\x00\x00\xFF\x7F\x47\xA4\x97\x04\x4E\x2F\x41\x00\x4C\x8F\x05\x00\x04\x71\x97\x80\x01\x01\x05\x00\xED\x9E\x00\x52\x47\x42\x20\x49\x00\x00\x00\x5F\x9E\x01\x00\x00\x00\x00\x00\xC0\x7F\x44\x9B\x9E\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xE9\x9E\x03\x00\x00\x00\x00\x00\xc0\x7F\x44\x99\x96\x04\x52\x41\x57\x00\x29\x8E\x05\x10\x00\x64\x96\x80\x03\x01\x05\x00\xEE\x9D\x00\x43\x4F\x4C\x20\x4F\x00\x00\x00\x4D\x9D\x01\x00\x00\x00\x00\x00\x00\x20\x41\x02\x9D\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xEA\x9D\x03\x00\x00\x00\x00\x00\x00\x20\x41\x00\x95\x04\x49\x44\x58\x00\x3B\x8D\x05\x00\x04\x73\x95\x80\x01\x00\x03\x00\xE8\x94\x00\x41\x4D\x42\x49\x6C\x9C\x01\x00\x00\x00\x00\x00\x00\xC8\x42\xE8\x9C\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xEB\x9C\x03\x00\x00\x00\x00\x00\x00\xC8\x42\xEA\x94\x04\x50\x43\x54\x00\x28\x8C\x05\x10\x00\x66\x94\x80\x01\x00\x03\x00\xE9\x9B\x00\x52\x45\x46\x4C\x54\x00\x00\x00\x2D\x9B\x01\x00\x00\x00\x00\x00\x00\xC8\x42\xEF\x9B\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xEC\x9B\x03\x00\x00\x00\x00\x00\x00\xC8\x42\xED\x93\x04\x50\x43\x54\x00\x2F\x8B\x05\x10\x00\x61\x93\x80\x01\x00\x03\x00\xEE\x9A\x00\x43\x4F\x55\x4E\x54\x00\x00\x00\x26\x9A\x01\x00\x00\x00\x00\x00\x00\xC8\x42\xEE\x9A\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xED\x9A\x03\x00\x00\x00\x00\x00\x00\xC8\x42\xEC\x92\x04\x43\x4E\x54\x00\x30\x8A\x05\x08\x00\x78\x92\x80\x01\x02\x04\x00\xEA\x91\x00\x50\x52\x4F\x58\x7B\x99\x01\x00\x00\x00\x00\x00\x00\x20\x41\x06\x99\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xEE\x99\x03\x00\x00\x00\x00\x00\x00\x20\x41\x04\x91\x04\x44\x49\x53\x00\x34\x89\x05\x50\x00\x23\x91\x80\x01\x00\x03\x00\xEC\x98\x00\x43\x4F\x4C\x4F\x52\x00\x00\x00\x3A\x98\x01\x00\x00\x00\x00\x00\x00\x20\x41\x07\x98\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xEF\x98\x03\x00\x00\x00\x00\x00\x00\x20\x41\x05\x90\x04\x49\x44\x58\x00\x3E\x88\x05\xC4\x00\xB6\x90\x80\x01\x00\x03\x00\xED\x88\x06\x4F\x00\x3E\x04"


def _get_serial_handler(serial_path, baudrate):
    """Open serial port and return serial handler

    Low level function, prefer using :meth:get_serial_handler.

    TODO: reset DTR => ok ? marche pas tout le temps
    TODO: DTR on Rpi ? => gpio control
    TODO: flush buffer after open

    .. note:: doc: See rtscts=True and dsrdtr=True
        https://github.com/pyserial/pyserial/issues/59

    :return: Serial port handler.
    :rtype: serial.Serial
    """
    if not os.path.exists(serial_path):
        print("Serial port <%s> not available!" % serial_path)
        return

    # Configure first and open later: allow to assert DTR line during opening
    # PS: Opened mode: os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK
    serial_handler = serial.Serial()
    serial_handler.port = serial_path
    serial_handler.baudrate = baudrate
    serial_handler.timeout = 0.2
    # Reset Arduino
    # TODO: True or false works for a reset :o
    # deassert the DTR line is also working ? it should forbid the reboot of arduino ?
    serial_handler.dtr = True
    try:
        serial_handler.open()
        assert serial_handler.is_open
        serial_handler.dtr = False  # This line raises an exception on emulated interface
        # time.sleep(0.5)
        assert serial_handler.is_open

    except serial.serialutil.SerialException as e:
        # LOGGER.exception(e)
        print(e)
        return

    # Wait Arduino boot after DTR signal
    # time.sleep(1)
    # Flush not expected random input data...
    serial_handler.reset_input_buffer()
    return serial_handler


def get_serial_handler(serial_path, baudrate):
    """Try to open serial port and return serial handler

    :raise: <IOError> if serial port is not available.
    :return: Serial port handler. Test it 5 times until return None.
    :rtype: serial.Serial or None
    """
    error = 0
    while error < 5:
        serial_handler = _get_serial_handler(serial_path, baudrate)
        if serial_handler:
            return serial_handler
        error += 1
        time.sleep(1)
    raise IOError("Serial port <%s> not available!" % serial_path)


def connect_to_hub():
    """Connect to hub, send and receive queries

    .. note:: The maximum time interval allowed is ~200ms between 2 NACKS,
        or between the connection and the first NACK sent to the device.

    :return: Serial handler
    :rtype: serial.Serial
    """
    serial_handler = get_serial_handler(SERIAL_PORT, BAUDRATE_INIT)

    # Wait configuration data until ACK
    ack_exchanged = False
    while not ack_exchanged:
        response = serial_handler.readline()
        # print(response)
        # print("end", response[-1], response[-1], bytes(response[-1]), b"\x04")
        print("Acquisition of device configuration...")

        if not ack_exchanged and response and response[-1] == 0x04:
            print("ACK received, send a ACK to end the init phase")
            assert response in colour_expected_init_seq, "Not expected init seq, please just retry"
            serial_handler.write(b"\x04")
            ack_exchanged = True

    # Upgrade the baudrate to production rate
    # We have 200ms maximum to send the ACK AND change the baudrate,
    # since the device expect a NACK every 100-200ms before resetting.
    # BUT, the baudrate change AND the sending of the ACK need some time...
    serial_handler.flush()
    print("Baudrate upgrade...")
    # serial_handler.close()
    # del serial_handler
    time.sleep(0.2)
    # serial_handler = get_serial_handler(SERIAL_PORT, BAUDRATE)
    serial_handler.baudrate = BAUDRATE
    return serial_handler


def autoconnect():
    """Automatic connection via 3 attempts to find device on serial line
    Sometimes init sequence can be missed, this function accept 5 fails.

    .. seealso:: `connect_to_hub`

    :return: Serial handler
    :rtype: serial.Serial
    """
    i = 0
    serial_handler = None
    while i < 3:
        try:
            serial_handler = connect_to_hub()
        except AssertionError as e:
            if "retry" in e.__str__():
                i += 1
                continue
            else:
                # Another error ?
                raise
        break
    if i == 3:
        # Max attempts
        raise IOError("Device not found on Serial line")
    return serial_handler
