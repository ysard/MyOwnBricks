/*
 * MyOwnBricks is a library for the emulation of PoweredUp sensors on microcontrollers
 * Copyright (C) 2022 Ysard - <ysard@users.noreply.github.com>
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

/*
 *   Sensor   Pro Micro
 *   -        VCC (3.3V)
 *   -        GND
 *
 *   Pro Micro:
 *   Serial: UART via USB
 *   Serial1: pin 1 (TX), pin 0 (RX)
 */
#include "MyOwnBricks.h"
#define DEBUG
// Normalize values from sensor to 0...100 interval
// See getPercentage() function notes
#define LUX_TO_PERCENTAGE(val)                (getPercentage(val, <a_coef>, <b_coef>))
#define REFLECTED_LIGHT_TO_PERCENTAGE(val)    (getPercentage(val, <a_coef>, <b_coef>))

uint8_t       sensorColor;
uint8_t       reflectedLight;
uint8_t       ambientLight;
uint16_t      sensorRGB[3];
uint16_t      sensorHSV[3];
bool          connection_status;

ColorSensor myDevice;


/**
 * @brief Callback for LED brightness change.
 *
 * @note This sensor has 3 built-in lights: 0: left, 1: bottom, 2: right.
 *      Set the brightness of each light.
 */
void LEDBrightnessesChanged(const uint8_t *LEDBrightnesses) {
    // DO stuff here to set brightness of LED segments.
#if (defined(INFO) || defined(DEBUG))
    Serial.print(F("Received LEDBrightnesses (Left,Bottom,Right): "));
    Serial.print(LEDBrightnesses[0], HEX);
    Serial.print(F(", "));
    Serial.print(LEDBrightnesses[1], HEX);
    Serial.print(F(", "));
    Serial.println(LEDBrightnesses[2], HEX);
#endif
}


/**
 * @brief Map lux/reflected light values to percentages
 *    Weights of the equation must be calculated empirically
 *    Map equation: y = ax + b
 *    System to solve:
 *      100% = MaxRawValue * a + b
 *      0% = MinRawValue * a + b
 */
uint8_t getPercentage(const uint16_t rawValue, const float& a_coef, const float& b_coef) {
    int8_t percent = static_cast<int8_t>(rawValue * a_coef + b_coef);
    if (percent > 100)
        return 100;
    if (percent < 0)
        return 0;
    return static_cast<uint8_t>(percent);
}


void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

#if (defined(INFO) || defined(DEBUG))
    Serial.begin(115200); // USB CDC
    while (!Serial) {
        // Wait for serial port to connect.
    }
#endif

    // Device config
    myDevice.setSensorRGB_I(sensorRGB);
    myDevice.setSensorHSV(sensorHSV);
    myDevice.setSensorColor(&sensorColor);
    myDevice.setSensorReflectedLight(&reflectedLight);
    myDevice.setSensorAmbientLight(&ambientLight);

    myDevice.setLEDBrightnessesCallback(&LEDBrightnessesChanged);
    connection_status = false;
}


void loop() {
    // Set arbitrary detected color
    sensorColor = COLOR_RED;

    // Set ambient light (lux) - map 0-100
    //ambientLight = map(<current_value>, 0, <max_value>, 0, 100);
    // Or:
    //ambientLight = LUX_TO_PERCENTAGE(<current_value>);

    // Set reflected light - map 0-100
    //reflectedLight = map(<current_value>, 0, <max_value>, 0, 100);
    // Or:
    //reflectedLight = REFLECTED_LIGHT_TO_PERCENTAGE(<current_value>);

    // Set RGB channels: blue
    sensorRGB[0] = 0;       // Red
    sensorRGB[1] = 0;       // Green
    sensorRGB[2] = 0x3ff;   // Blue

    // Set HSV channels
    sensorHSV[0] = 0;       // Hue
    sensorHSV[1] = 0;       // Saturation
    sensorHSV[2] = 0;       // Value/Brightness

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

#if (defined(INFO) || defined(DEBUG))
    Serial.flush();
#endif
}
