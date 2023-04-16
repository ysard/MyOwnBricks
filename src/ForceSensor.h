/*
 * A library for the emulation of PoweredUp sensors on microcontrollers
 * Copyright (C) 2022-2023 Ysard - <ysard@users.noreply.github.com>
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
#ifndef FORCE_SENSOR_H
#define FORCE_SENSOR_H

#include "BaseSensor.h"

/**
 * @brief Handle the LegoUART protocol and define modes of the
 *      Spike/Technic Force Sensor.
 *
 * @param m_force Detected force applied
 *      Expected force applied: 2.5 to 10 Newtons (~1Kg).
 *      Resolution: 0.1 Newton.
 *      Continuous values 0...10.
 * @param m_touched Boolean set to true if the sensor is touched.
 *      This value is supposed to be true if the raw value of the sensor
 *      is just above a detectable change (modulo a small calibration tolerance).
 *      Expected force applied: 0.5-1.0 Newton +/- 10%.
 * @param m_tapped Boolean set to true if the sensor is tapped.
 *      How hard the sensor was tapped.
 *      Expected force applied: 0.5-1.0 Newton +/- 10%.
 *      Built-in values:
 *      0: Nothing, 1: Single tap, 2: Quick tap, 3: Press and hold.
 * @param m_rawForce Raw value of the force sensor.
 *      The higher the pressure, the higher the value is.
 *
 *      Based on 3 internal values used for calibration:
 *          m_raw_offset: ?;
 *          m_raw_released: Minimal value supported by the sensor;
 *          m_raw_end: Max value supported by the sensor.
 *      With m_raw_released < m_raw_end.
 *
 * @param m_defaultComboModesEnabled Boolean set to true if the device receives
 *      a combo mode / multi-mode packet. This packet should overwrite the default
 *      0 mode by asking specific values to the device after each NACK received.
 *      See: https://lego.github.io/MINDSTORMS-Robot-Inventor-hub-API/class_device.html
 *      See: https://github.com/pybricks/technical-info/blob/master/uart-protocol.md
 */
class ForceSensor : public BaseSensor {
    // LEGO SPIKE Force Sensor modes
    // Pybricks uses modes 4, 6 only
    // https://github.com/pybricks/pybricks-micropython/blob/master/pybricks/util_pb/pb_device.h
    // Node PoweredUp uses modes 0, 1, 2
    // https://github.com/nathankellenicki/node-poweredup/blob/master/src/devices/technicforcesensor.ts
    enum {

        PBIO_IODEV_MODE_PUP_FORCE_SENSOR__FORCE = 0,  // read 1x int8_t
        PBIO_IODEV_MODE_PUP_FORCE_SENSOR__TOUCHED = 1,// read 1x int8_t
        PBIO_IODEV_MODE_PUP_FORCE_SENSOR__TAPPED = 2, // read 1x int8_t
        PBIO_IODEV_MODE_PUP_FORCE_SENSOR__FRAW  = 4,  // read 1x int16_t
        PBIO_IODEV_MODE_PUP_FORCE_SENSOR__CALIB = 6,  // ??   8x int16_t
    };

public:
    ForceSensor();
    ForceSensor(uint8_t *pForce, bool *pTouched, uint8_t *pTapped);

    void setSensorForce(int8_t *pData);
    void setSensorTouched(bool *pData);
    void setSensorTapped(uint8_t *pData);
    void setSensorRawForce(uint16_t *pData);
    void setSensorCalibrationValues(uint16_t raw_offset, uint16_t raw_released, uint16_t raw_end);

private:
    // Process queries from/to hub
    virtual void handleModes();
    // Protocol handy functions
    virtual void commSendInitSequence();
    void extendedModeInfoResponse();

    // Handle queries from the hub
    void sensorForceMode();
    void sensorTouchedMode();
    void sensorTappedMode();
    void sensorForceRawMode();
    void sensorCalibrationMode();
    void defaultCombosMode();

    void ackResetCombosMode();
    void ackSetCombosMode();

    uint8_t  *m_force;
    uint16_t *m_rawForce;
    bool     *m_touched;
    uint8_t  *m_tapped;

    // Internal sensor calibration data
    uint16_t m_raw_offset;
    uint16_t m_raw_released;
    uint16_t m_raw_end;

    bool     m_defaultComboModesEnabled;
};

#endif
