/*
 * MyOwnBricks is a library for the emulation of PoweredUp sensors on microcontrollers
 * Copyright (C) 2021-2022 Ysard - <ysard@users.noreply.github.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <TiltSensor.h>

int8_t sensorX;
int8_t sensorY;
bool   connection_status;

TiltSensor myOwnTilt(& sensorX, & sensorY);


void setup() {
#if (defined(INFO) || defined(DEBUG))
    Serial.begin(115200); // USB CDC
    while (!Serial) {
        // Wait for serial port to connect.
    }
#endif

    // Init roll & pitch values if not made via the constructor
    //myOwnTilt.setSensorTiltX(&sensorX);
    //myOwnTilt.setSensorTiltY(&sensorY);
    connection_status = false;
}


void loop() {
    // Get data from orientation sensor
    int arbitraryValue = 512;

    sensorX = map(arbitraryValue, 0, 1023, -45, 45);
    sensorY = map(arbitraryValue, 0, 1023, -45, 45);

    myOwnTilt.Process();

    if (myOwnTilt.isConnected()) {
        // Already connected ?
        if (!connection_status) {
            INFO_PRINTLN(F("Connected !"));
            connection_status = true;
        }
    } else {
        INFO_PRINTLN(F("Not Connected !"));
        connection_status = false;
    }
}
