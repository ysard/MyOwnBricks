/*
 * MyOwnBricks is a library for the emulation of PoweredUp sensors on microcontrollers
 * Copyright (C) 2021 Ysard - <ysard@users.noreply.github.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @brief This is a test sketch with fixed values expected by the unittest Python module
 */
#define DEBUG
#define INFO
#include "MyOwnBricks.h"

uint8_t       LEDColor;
uint8_t       sensorColor;
uint8_t       reflectedLight;
uint8_t       ambientLight;
uint16_t      red, green, blue, clear, lux;
uint16_t      sensorRGB[3];
uint8_t       sensorDistance;
bool          connection_status;

ColorDistanceSensor myDevice;

/**
 * @brief Callback for LED color change
 */
void LEDColorChanged(uint8_t color) {
    INFO_PRINT(F("Color changed callback: "));
    INFO_PRINTLN(color, HEX);
    LEDColor = color;
}

/**
 * @brief Callback for IR code sent by the hub
 */
void IRCallback(const uint16_t value) {
    INFO_PRINT(F("IR callback: "));
    INFO_PRINTLN(value, HEX);
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

#if (defined(INFO) || defined(DEBUG))
    Serial.begin(115200); // USB CDC
    while (!Serial) {
        // Wait for serial port to connect.
    }
#endif

    // Default values
    LEDColor = COLOR_BLACK; // 0
    reflectedLight = 1;
    ambientLight = 2;
    sensorColor = COLOR_BLUE; // 3
    sensorRGB[0] = red = 4;
    sensorRGB[1] = green = 5;
    sensorRGB[2] = blue = 6;
    sensorDistance = 0x0A;

    // Device config
    myDevice.setSensorColor(&sensorColor);
    myDevice.setSensorReflectedLight(&reflectedLight);
    myDevice.setSensorAmbientLight(&ambientLight);
    myDevice.setSensorRGB(sensorRGB);
    myDevice.setSensorDistance(&sensorDistance);
    myDevice.setLEDColorCallback(&LEDColorChanged);
    myDevice.setIRCallback(&IRCallback);
    connection_status = false;
}

void loop()
{
    // Send data to PoweredUp Hub
    myDevice.process();

    if (myDevice.isConnected()) {
        // Already connected ?
        if (!connection_status) {
            INFO_PRINTLN(F("Connected !"));
            pinMode(LED_BUILTIN_TX, INPUT);
            pinMode(LED_BUILTIN_RX, INPUT);

            connection_status = true;
        }
    } else {
        INFO_PRINTLN(F("Not Connected !"));
        pinMode(LED_BUILTIN_TX, OUTPUT);
        pinMode(LED_BUILTIN_RX, OUTPUT);

        connection_status = false;
    }
}
