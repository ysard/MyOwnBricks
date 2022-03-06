/*
 * MyOwnBricks is a library for the emulation of PoweredUp sensors on microcontrollers.
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

/*
 *   IR LED   Pro Micro
 *   DATA     pin 5, Timer 10-bits High Speed
 *   -        VCC (3.3V)
 *   GND      GND
 *
 *   IR LED is connected to VCC via 1K resistance and GND via the collector of a
 *   2N3904 transistor.
 *   The transistor base is driven via pin 5 through a 330 ohms resistance.
 *
 *   Pro Micro:
 *   Serial: UART via USB
 *   Serial1: pin 1 (TX), pin 0 (RX)
 */
// IRremote 3.5.x configuration
// See defines in ~/Arduino/libraries/IRremote/IRremote.hpp
// See pin alternatives: https://github.com/Arduino-IRremote/Arduino-IRremote#hardware-pwm-signal-generation-for-sending
//#define IR_SEND_PIN   5
#define SEND_PWM_BY_TIMER
#define NO_LEGACY_COMPATIBILITY
#define EXCLUDE_UNIVERSAL_PROTOCOLS
#define EXCLUDE_EXOTIC_PROTOCOLS
#define NO_DECODER

#include <IRremote.hpp>
#include "ColorDistanceSensor.h"

// Init IR sender
IRsend              irsend;
// Init sensor
ColorDistanceSensor myDevice;
bool                connection_status;


/**
 * @brief Callback for IR code sent by the hub
 */
void IRCallback(const uint16_t value) {
#if (defined(INFO) || defined(DEBUG))
    Serial.print(F("IR callback: "));
    Serial.println(value, HEX);
#endif

    irsend.sendLegoPowerFunctions(value, false);
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
    myDevice.setIRCallback(&IRCallback);
    connection_status = false;
}


void loop() {
    myDevice.process();

    if (myDevice.isConnected()) {
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
