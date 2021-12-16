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

/*
 *   Sensor   Pro Micro
 *   SCL      SCL pin 3
 *   SDA      SDA pin 2
 *   INT      PCINT4, port PB4, pin 8
 *   VIN      VCC (3.3V)
 *   GND      GND
 *
 *   LED pin of the sensor can be connected to its INT pin.
 *   By doing this the leds will turn off when the measurements are done.
 *
 *   Pro Micro:
 *   Serial: UART via USB
 *   Serial1: pin 1 (TX), pin 0 (RX)
 */
#include <Wire.h>
#include "tcs34725.h"
#define MANHATTAN
#include "MyOwnBricks.h"

#define RGB_SENSOR_INTERRUPT_PIN     8
#define RGB_SENSOR_INTERRUPT_PORT    PB4
// Equivalent of digitalRead but for PORTB pins & much more quicker for a use in an ISR
// https://www.arduino.cc/en/Reference/PortManipulation
#define tstPin(b)                             ((PINB & (1 << (b))) != 0)
#define LUX_TO_PERCENTAGE(val)                (getPercentage(val, 0.0105, -0.0843))
#define REFLECTED_LIGHT_TO_PERCENTAGE(val)    (getPercentage(val, 0.0017, -8))

uint8_t       sensorColor;
uint8_t       reflectedLight;
uint8_t       ambientLight;
uint16_t      red, green, blue, clear, lux;
uint16_t      sensorRGB[3];
bool          connection_status;
volatile bool sensorReady;

// Default settings: TCS34725_GAIN_4X,  TCS34725_INTEGRATIONTIME_154MS
TCS34725            rgb_sensor;
ColorDistanceSensor myDevice;


/**
 * @brief Callback for PCINT4 interrupt (PCINT0 - PCINT7)
 */
ISR(PCINT0_vect) {
    // If PB4 is LOW, sensor is ready
    if (!tstPin(RGB_SENSOR_INTERRUPT_PORT))
        sensorReady = true;
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
    sensorColor = COLOR_NONE;

    myDevice.setSensorColor(&sensorColor);
    myDevice.setSensorReflectedLight(&reflectedLight);
    myDevice.setSensorAmbientLight(&ambientLight);
    myDevice.setSensorRGB(sensorRGB);
    connection_status = false;

    // Colour sensor config
    // Configure PinChange Interrupt
    // See https://github.com/NicoHood/PinChangeInterrupt
    // Note: INT-0,1,2,3 are occupied by UART and i2c transmissions on pro-micro
    // /!\ DO NOT activate pullup from the arduino, the INT pin is usually already
    // pulled up into the sensor board itself to 3.3V. These pins (SCL, SDA, INT)
    // ARE NOT tolerant to more than VDD + 0.5V. Note that I2C pins are connected
    // to level shifters, but not the others.
    pinMode(RGB_SENSOR_INTERRUPT_PIN, INPUT); // TCS interrupt output is Active-LOW and Open-Drain
    cli();                                    // Disable all interrupts: Avoid first and not wanted trigger of the interrupt
    PCICR  |= 0b00000001;                     // enable PORTB pin change interrupt
    PCMSK0 |= 0b00010000;                     // enable PB4, PCINT4, pin 8
    sei();                                    // Enable all interrupts

    while (!rgb_sensor.begin()) {
        INFO_PRINTLN(F("TCS34725 NOT found"));
        delay(200);
    }
    INFO_PRINTLN(F("Found sensor"));

    // Set persistence filter to generate an interrupt for every RGB Cycle,
    // regardless of the integration limits
    rgb_sensor.m_tcs.write8(TCS34725_PERS, TCS34725_PERS_NONE);
    // RGBC interrupt enable. When asserted, permits RGBC interrupts to be generated.
    rgb_sensor.m_tcs.setInterrupt(true);
}


void loop()
{
    if (sensorReady) {
        // Data measurement
        boolean status = rgb_sensor.updateData(true);

        if (status) {
            // Ambient light (lux) computation
            rgb_sensor.updateLux();

            int16_t lux = lround(rgb_sensor.lux);

            // Sometimes lux values are below 0; this coincides with erroneous data
            // Moreover, we discard data taken below 40 lux.
            if ((lux >= 40) && (static_cast<uint16_t>(lux) <= rgb_sensor.maxlux)) {
                // Set ambient light (lux) - map 0-100
                //ambientLight = map(rgb_sensor.lux, 0, rgb_sensor.maxlux, 0, 100);
                ambientLight = LUX_TO_PERCENTAGE(lux); // cast ?

                // RGBC Channels are usable
                // Map values to max ~440;
                // Continuous values from 0-65535 (16bits) to 0-1023 (10bits)
                // Note: 440 gives ~28000 (which is the quasi maximum value observed in the channels)
                red   = rgb_sensor.r_comp >> 6,
                green = rgb_sensor.g_comp >> 6,
                blue  = rgb_sensor.b_comp >> 6,
                clear = rgb_sensor.c_comp >> 6;

                // Set clear channel as reflected light - map 0-100
                reflectedLight = REFLECTED_LIGHT_TO_PERCENTAGE(clear);

                // Set RGB channels
                sensorRGB[0] = red;
                sensorRGB[1] = green;
                sensorRGB[2] = blue;

                // Set detected color
                detectColor();
            } else {
                sensorColor = COLOR_NONE;
            }
#if (defined(INFO) || defined(DEBUG))

            /*
            Serial.print("Lux: "); Serial.print(rgb_sensor.lux, DEC);
            Serial.print("; max: "); Serial.print(rgb_sensor.maxlux);
            Serial.print("; R: "); Serial.print(red, DEC);
            Serial.print("; G: "); Serial.print(green, DEC);
            Serial.print("; B: "); Serial.print(blue, DEC);
            Serial.print("; C: "); Serial.println(clear, DEC);
            */

            Serial.print(rgb_sensor.lux, DEC); Serial.print(";");
            Serial.print(rgb_sensor.maxlux); Serial.print(";");
            Serial.print(red, DEC); Serial.print(";");
            Serial.print(green, DEC); Serial.print(";");
            Serial.print(blue, DEC); Serial.print(";");
            Serial.println(clear, DEC);
#endif
        } else {
            sensorColor = COLOR_NONE;
            INFO_PRINTLN(F("not valid data! wait next measure"));
        }
        // Interrupt tear down
        rgb_sensor.m_tcs.clearInterrupt();
        sensorReady = false;
        PCIFR      &= ~(1 << PCIF0); // clear PC interrupt flag in case of bounce
    }

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
