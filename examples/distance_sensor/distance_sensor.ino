/*
 * MyOwnBricks is a library for the emulation of PoweredUp sensors on microcontrollers
 * Copyright (C) 2021 Ysard - <ysard@users.noreply.github.com>
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
 *   INT      EXT INT6 pin 7
 *   VIN      VCC (3.3V)
 *   GND      GND
 *
 *   Pro Micro:
 *   Serial: UART via USB
 *   Serial1: pin 1 (TX), pin 0 (RX)
 */
#include <Wire.h>
#include <VL6180X.h>
#include "ColorDistanceSensor.h"

#define SENSOR_INTERRUPT_PIN    7
uint8_t       sensorDistance;
bool          connection_status;
volatile bool distSensorReady;
uint16_t      detectionCount;
uint8_t       previousDistStatus;

VL6180X             distSensor;
ColorDistanceSensor myDevice;


/**
 * @brief Callback for INT6 interrupt
 */
void ISR_sensor() {
    distSensorReady = true;
}


/**
 * @brief Read reading of range;
 *    Must be called after the interrupt has been triggered
 */
uint8_t readRangeNonBlocking() {
    uint8_t range = distSensor.readReg(VL6180X::RESULT__RANGE_VAL);

    distSensor.writeReg(VL6180X::SYSTEM__INTERRUPT_CLEAR, 0x01);
    return range;
}


/**
 * @brief Convert raw range value to millimeters.
 *    Mostly useful when the scale factor is modified to increase the measuring range.
 */
uint16_t readRangeNonBlockingMillimeters() {
    return static_cast<uint16_t>(readRangeNonBlocking() * distSensor.getScaling());
}


/**
 * @brief Map values to percentages
 *    Weights of the equation must be calculated empirically
 *    Equation: y = ax + b
 *
 *    For scale factor 1 (default): a = 0.543, b = -8.152
 *    For scale factor 2: a = 0.3401, b = -5.4422
 */
uint8_t getDistancePercent(uint16_t& rawValue) {
    int8_t percent = static_cast<int8_t>(rawValue * 0.543 - 8.152);
    if (percent > 100)
        return 100;
    if (percent < 0)
        return 0;
    return static_cast<uint8_t>(percent);
}


void initDistSensor() {
    distSensor.init();
    distSensor.configureDefault();
    distSensor.setTimeout(100);

    // If scaling is modified, do not forget to update weights in getDistancePercent()
    // a = 0.3401, b = -5.4422
    //sensor.setScaling(2);

    // stop continuous mode if already active
    distSensor.stopContinuous();
    // in case stopContinuous() triggered a single-shot
    // measurement, wait for it to complete
    delay(300); // TODO: check this! Can be too long for reboot actions

    // enable interrupt output on GPIO1
    distSensor.writeReg(VL6180X::SYSTEM__MODE_GPIO1, 0x10);
    // clear any existing interrupts
    distSensor.writeReg(VL6180X::SYSTEM__INTERRUPT_CLEAR, 0x03);

    distSensor.startRangeContinuous(); // default period = 100ms
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
    myDevice.setSensorDistance(&sensorDistance);
    connection_status = false;
    distSensorReady   = false;

    // Distance sensor config
    // pin 7, INT6 EXT
    // Note: INT-0,1,2,3 are occupied by UART and i2c transmissions on pro-micro
    // /!\ DO NOT activate pullup from the arduino, the GPIO1 pin is usually already
    // pulled up into the sensor board itself to 2.8V. These pins (SCL, SDA, GPIO0, GPIO1)
    // ARE NOT tolerant to more than VDD + 0.5V (= 3.3V). Note that I2C pins are connected
    // to level shifters, but not the others.
    // pinMode(SENSOR_INTERRUPT_PIN, INPUT_PULLUP);  // DON'T do this!
    cli(); // Disable all interrupts: Avoid first and not wanted trigger of the interrupt
    attachInterrupt(digitalPinToInterrupt(SENSOR_INTERRUPT_PIN), ISR_sensor, FALLING);
    sei(); // Enable all interrupts

    Wire.begin();
    initDistSensor();
}


void loop() {
    if (distSensorReady) {
        // Get distance
        uint16_t raw_distance = readRangeNonBlockingMillimeters(); // 1ms
        // Get error status
        uint8_t status = distSensor.readRangeStatus();

        if (status == VL6180X_ERROR_NONE) {
            // Correct detection occured
            // Set distance percentage to the vision sensor
            sensorDistance = getDistancePercent(raw_distance);

            // If previous status is 0 and this one is != 0: target is gone
            // If previous status is != 0 and this one is 0: target appears
            //    => increment detection count
            //if (previousDistStatus != status && raw_distance <= 50)
            //  detectionCount++;

            previousDistStatus = status;

            INFO_PRINT("Distance (mm): ");
            INFO_PRINTLN(raw_distance);
        } else {
            DEBUG_PRINT("Status: ");
            DEBUG_PRINTLN(status);
        }

        distSensorReady = false;
        EIFR &= ~(1 << INTF6); // clear interrupt flag in case of bounce
    }
    // Synchronous reading
    //INFO_PRINTLN(sensor.readRangeContinuousMillimeters()); // 12ms, DON'T do this!

    // Send data to PoweredUp Hub
    myDevice.process();

    if (myDevice.isConnected()) {
        // Already connected ?
        if (!connection_status) {
            INFO_PRINTLN("Connected !");
            pinMode(LED_BUILTIN_TX, INPUT);
            pinMode(LED_BUILTIN_RX, INPUT);

            connection_status = true;
        }
    } else {
        INFO_PRINTLN("Not Connected !");
        pinMode(LED_BUILTIN_TX, OUTPUT);
        pinMode(LED_BUILTIN_RX, OUTPUT);

        connection_status = false;
    }

#if (defined(INFO) || defined(DEBUG))
    Serial.flush();
#endif
}
