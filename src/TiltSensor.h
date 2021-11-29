/*
 * A library for the emulation of PoweredUp sensors on microcontrollers
 * Copyright (C) 2021 Ysard - <ysard@users.noreply.github.com>
 *
 * Based on the original work of Ahmed Jouirou - <ahmed.jouirou@gmail.com>
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
#ifndef LegoPupTilt_h
#define LegoPupTilt_h

#include "basicsensor.h"

class LegoPupTilt : BasicSensor {
public:
    LegoPupTilt();
    LegoPupTilt(int8_t *pSensorTiltX, int8_t *pSensorTiltY);
    void process();

    void setSensorTiltX(int8_t *pData);
    void setSensorTiltY(int8_t *pData);

private:
    void commSendInitSequence(void);

    int8_t *m_sensorTiltX;
    int8_t *m_sensorTiltY;
};

#endif
