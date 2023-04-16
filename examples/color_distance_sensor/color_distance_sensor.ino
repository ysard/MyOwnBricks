/*
 * MyOwnBricks is a library for the emulation of PoweredUp sensors on microcontrollers
 * Copyright (C) 2021-2023 Ysard - <ysard@users.noreply.github.com>
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
 *   SCL      SCL pin 3
 *   SDA      SDA pin 2
 *   INTRGB   PCINT4, port PB4, pin 8
 *   INTDIST  EXT INT6 pin 7
 *   SHUTDIST pin 4
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
#include <VL6180X.h>
#include "distance_sensor.hpp"
#include "rgb_sensor.hpp"

#define DIST_SHUTDOWN_PIN            4
#define DIST_SENSOR_INTERRUPT_PIN    7
#define RGB_SENSOR_INTERRUPT_PIN     8
#define RGB_SENSOR_INTERRUPT_PORT    PB4
// Equivalent of digitalRead but for PORTB pins & much more quicker for a use in an ISR
// https://www.arduino.cc/en/Reference/PortManipulation
#define tstPin(b)    ((PINB & (1 << (b))) != 0)

uint8_t       sensorColor;
uint8_t       reflectedLight;
uint8_t       ambientLight;
uint16_t      red, green, blue, clear, lux;
uint16_t      sensorRGB[3];
uint8_t       sensorDistance;
bool          connection_status;
uint8_t       previousDistStatus;
volatile bool rgbSensorReady;
volatile bool distSensorReady;

// Default settings: TCS34725_GAIN_4X,  TCS34725_INTEGRATIONTIME_154MS
TCS34725            rgb_sensor;
VL6180X             dist_sensor;
ColorDistanceSensor myDevice;


/**
 * @brief Callback for PCINT4 interrupt (PCINT0 - PCINT7)
 */
ISR(PCINT0_vect) {
    // If PB4 is LOW, sensor is ready
    if (!tstPin(RGB_SENSOR_INTERRUPT_PORT))
        rgbSensorReady = true;
}


/**
 * @brief Callback for INT6 interrupt
 */
void ISR_sensor() {
    distSensorReady = true;
}


/**
 * @brief Callback for LED color change.
 *
 * @note You may want to disable the TCS LEDs when the selected color is black.
 *      The LED pin should be connected to an arduino pin driven here.
 */
void LEDColorChanged(uint8_t color) {
    if (color == COLOR_BLACK) {
        // Disable the LEDs of rgb sensor
        // Do stuff here to deassert LED pin by setting it to LOW...
        return;
    }
    // Enable the LEDs of rgb sensor
    // DO stuff here to set the LED pin to HIGH or follow the status of the
    // interrupt pin.
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


/**
 * @brief Workaround to solve the problem of the 2 sensors with the same address.
 *    - TCS34725: No way to change address nor shutdown, but can send Power Off command.
 *    - VL6180X: Can change address and assert/deassert Chip Enable pin.
 *
 *    - Shutdown distance sensor to start rgb sensor on the same address.
 *    - Disable rgb sensor to configure distance sensor (and change its address).
 */
void i2cSameAddressWorkaround() {
    // Shutdown distance sensor
    pinMode(DIST_SHUTDOWN_PIN, OUTPUT);
    digitalWrite(DIST_SHUTDOWN_PIN, LOW);

    // Start RGB sensor
    while (!rgb_sensor.begin()) {
        INFO_PRINTLN(F("TCS34725 NOT found"));
        delay(200);
    }
    INFO_PRINTLN(F("Found sensor"));

    // Disable rgb sensor
    rgb_sensor.tcs.disable();
    delay(5);

    // Restart distance sensor
    // Let's float the shutdown pin
    pinMode(DIST_SHUTDOWN_PIN, INPUT);

    // Configure distance sensor (and change its address)
    // For some reason, init() MUST be called twice.
    // The first time, the device is not configured (commands go on the TCS's registers ?)
    // The second time (after changing the address) config is effective.
    //Wire.begin();
    dist_sensor.init();
    dist_sensor.setAddress(0x39);
    initDistSensor();
    delay(5);

    // Restart rgb sensor
    rgb_sensor.tcs.enable();
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
    myDevice.setSensorDistance(&sensorDistance);
    // myDevice.setLEDColorCallback(&LEDColorChanged); // See notes
    connection_status = false;
    rgbSensorReady    = false;
    distSensorReady   = false;

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

    // Distance sensor config
    // pin 7, INT6 EXT
    // Note: INT-0,1,2,3 are occupied by UART and i2c transmissions on pro-micro
    // /!\ DO NOT activate pullup from the arduino, the GPIO1 pin is usually already
    // pulled up into the sensor board itself to 2.8V. These pins (SCL, SDA, GPIO0, GPIO1)
    // ARE NOT tolerant to more than VDD + 0.5V (= 3.3V). Note that I2C pins are connected
    // to level shifters, but not the others.
    // pinMode(DIST_SENSOR_INTERRUPT_PIN, INPUT_PULLUP);  // DON'T do this!
    cli(); // Disable all interrupts: Avoid first and not wanted trigger of the interrupt
    attachInterrupt(digitalPinToInterrupt(DIST_SENSOR_INTERRUPT_PIN), ISR_sensor, FALLING);
    sei(); // Enable all interrupts

    i2cSameAddressWorkaround();

    // Set persistence filter to generate an interrupt for every RGB Cycle,
    // regardless of the integration limits
    rgb_sensor.tcs.write8(TCS34725_PERS, TCS34725_PERS_NONE);
    // RGBC interrupt enable. When asserted, permits RGBC interrupts to be generated.
    rgb_sensor.tcs.setInterrupt(true);
}


void loop()
{
    handleRGBSensorData();
    handleDistSensorData();

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
