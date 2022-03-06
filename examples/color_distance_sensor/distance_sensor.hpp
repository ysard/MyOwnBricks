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
extern uint8_t       sensorDistance;
extern uint8_t       previousDistStatus;
extern VL6180X       dist_sensor;
extern volatile bool distSensorReady;
extern uint8_t getPercentage(const uint16_t, const float&, const float&);

#define DISTANCE_TO_PERCENTAGE(val)    (getPercentage(val, 0.543, -8.152))

/**
 * @brief Read reading of range;
 *    Must be called after the interrupt has been triggered
 */
uint8_t readRangeNonBlocking() {
    uint8_t range = dist_sensor.readReg(VL6180X::RESULT__RANGE_VAL);

    dist_sensor.writeReg(VL6180X::SYSTEM__INTERRUPT_CLEAR, 0x01);
    return range;
}


/**
 * @brief Convert raw range value to millimeters.
 *    Mostly useful when the scale factor is modified to increase the measuring range.
 */
uint16_t readRangeNonBlockingMillimeters() {
    return static_cast<uint16_t>(readRangeNonBlocking() * dist_sensor.getScaling());
}


/**
 * @brief Init registers of VL6180X sensor and put it online.
 */
void initDistSensor() {
    dist_sensor.init();
    dist_sensor.configureDefault();
    dist_sensor.setTimeout(100);

    // If scaling is modified, do not forget to update weights in getDistancePercent()
    // a = 0.3401, b = -5.4422
    //dist_sensor.setScaling(2);

    // stop continuous mode if already active
    dist_sensor.stopContinuous();
    // in case stopContinuous() triggered a single-shot
    // measurement, wait for it to complete
    delay(300); // TODO: check this! Can be too long for reboot actions

    // enable interrupt output on GPIO1
    dist_sensor.writeReg(VL6180X::SYSTEM__MODE_GPIO1, 0x10);
    // clear any existing interrupts
    dist_sensor.writeReg(VL6180X::SYSTEM__INTERRUPT_CLEAR, 0x03);

    dist_sensor.startRangeContinuous(); // default period = 100ms
}


/**
 * @brief Process raw values from sensor and convert them for the PoweredUp hub,
 *      if needed.
 */
void handleDistSensorData() {
    if (!distSensorReady)
        return;

    // Get distance
    uint16_t raw_distance = readRangeNonBlockingMillimeters(); // 1ms
    // Get error status
    uint8_t status = dist_sensor.readRangeStatus();

    if (status == VL6180X_ERROR_NONE) {
        // Correct detection occured
        // Set distance percentage to the vision sensor
        sensorDistance = DISTANCE_TO_PERCENTAGE(raw_distance);

        // If previous status is 0 and this one is != 0: target is gone
        // If previous status is != 0 and this one is 0: target appears
        //    => increment detection count
        //if (previousStatus != status && raw_distance <= 50)
        //  detectionCount++;

        previousDistStatus = status;

        INFO_PRINT("Distance (mm): ");
        INFO_PRINTLN(raw_distance);
        INFO_PRINTLN(sensorDistance);
    } else {
        DEBUG_PRINT("Status: ");
        DEBUG_PRINTLN(status);
    }

    distSensorReady = false;
    EIFR &= ~(1 << INTF6); // clear interrupt flag in case of bounce
}
