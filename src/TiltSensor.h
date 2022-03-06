/*
 * MyOwnBricks is a library for the emulation of PoweredUp sensors on microcontrollers
 * Copyright (C) 2021-2022 Ysard - <ysard@users.noreply.github.com>
 *
 * Based on the original work of Ahmed Jouirou - <ahmed.jouirou@gmail.com>
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
#ifndef TILTSENSOR_H
#define TILTSENSOR_H

#include "BaseSensor.h"


/**
 * @brief Handle the LegoUART protocol and define modes of the
 * Tilt sensor.
 *
 * @param m_sensorTiltX Angle value in degrees for rotation along x-axis
 *      also called roll/roulis.
 *      Continuous values ??...??
 * @param m_sensorTiltY Angle value in degrees for rotation along y-axis
 *      also called pitch/tangage.
 *      Continuous values ??...??
 */
class TiltSensor : BaseSensor {
    // LEGO POWERED UP WEDO 2.0 Tilt sensor modes
    // https://github.com/pybricks/pybricks-micropython/blob/master/pybricks/util_pb/pb_device.h
    enum {
        PBIO_IODEV_MODE_PUP_WEDO2_TILT_SENSOR__ANGLE  = 0,  // read 2x int8_t
        //PBIO_IODEV_MODE_PUP_WEDO2_TILT_SENSOR__DIR    = 1,  // read 1x int8_t
        //PBIO_IODEV_MODE_PUP_WEDO2_TILT_SENSOR__CNT    = 2,  // read 3x int8_t
        //PBIO_IODEV_MODE_PUP_WEDO2_TILT_SENSOR__CAL    = 2,  // read 3x int8_t
    };

public:
    TiltSensor();
    TiltSensor(int8_t *pSensorTiltX, int8_t *pSensorTiltY);

    void setSensorTiltX(int8_t *pData);
    void setSensorTiltY(int8_t *pData);

    void sensorAngleMode();

private:
    // Process queries from/to hub
    virtual void handleModes();
    virtual void commSendInitSequence();

    int8_t *m_sensorTiltX;
    int8_t *m_sensorTiltY;
};

#endif
