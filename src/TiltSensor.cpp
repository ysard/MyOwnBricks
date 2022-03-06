/*
 * A library for the emulation of PoweredUp sensors on microcontrollers
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
#include "TiltSensor.h"

/**
 * @brief Default constructor
 */
TiltSensor::TiltSensor(){
    m_sensorTiltX = nullptr;
    m_sensorTiltY = nullptr;
}


/**
 * @brief  Constructor allowing to set X, Y angle measures.
 * @param pSensorTiltX
 * @param pSensorTiltY
 */
TiltSensor::TiltSensor(int8_t *pSensorTiltX, int8_t *pSensorTiltY){
    m_sensorTiltX = pSensorTiltX;
    m_sensorTiltY = pSensorTiltY;
}


/**
 * @brief Setter for m_sensorTiltX
 * @param pData
 */
void TiltSensor::setSensorTiltX(int8_t *pData){
    m_sensorTiltX = pData;
}


/**
 * @brief Setter for m_sensorTiltY
 * @param pData
 */
void TiltSensor::setSensorTiltY(int8_t *pData){
    m_sensorTiltY = pData;
}


/**
 * @brief Send initialization sequences for the current Color & Distance sensor.
 * @see https://github.com/pybricks/pybricks-micropython/lib/pbio/test/src/uartdev.c
 */
void TiltSensor::commSendInitSequence(){
    // Initialize uart
    SerialTTL.begin(2400);

    SerialTTL.write("\x40\x22\x9D\x49\x03\x02\xB7\x52\x00\xC2\x01\x00\x6E\x5F\x00", 15);SerialTTL.flush();
    SerialTTL.write("\x00\x00\x10\x00\x00\x00\x10\xA0\x9B\x00\x4C\x50\x46\x32\x2D\x43", 16);SerialTTL.flush();
    SerialTTL.write("\x41\x4C\x6F\x9B\x01\x00\x00\x34\xC2\x00\x00\x34\x42\xE5\x9B\x02", 16);SerialTTL.flush();
    SerialTTL.write("\x00\x00\xC8\xC2\x00\x00\xC8\x42\xE6\x9B\x03\x00\x00\x34\xC2\x00", 16);SerialTTL.flush();
    SerialTTL.write("\x00\x34\x42\xE7\x93\x04\x43\x41\x4C\x00\x26\x8B\x05\x10\x00\x61", 16);SerialTTL.flush();
    SerialTTL.write("\x93\x80\x03\x00\x03\x00\xEC\xA2\x00\x4C\x50\x46\x32\x2D\x43\x52", 16);SerialTTL.flush();
    SerialTTL.write("\x41\x53\x48\x00\x00\x00\x00\x00\x00\x53\x9A\x01\x00\x00\x00\x00", 16);SerialTTL.flush();
    SerialTTL.write("\x00\x00\xC8\x42\xEE\x9A\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xED", 16);SerialTTL.flush();
    SerialTTL.write("\x9A\x03\x00\x00\x00\x00\x00\x00\xC8\x42\xEC\x92\x04\x43\x4E\x54", 16);SerialTTL.flush();
    SerialTTL.write("\x00\x30\x8A\x05\x10\x00\x60\x92\x80\x03\x00\x03\x00\xED\xA1\x00", 16);SerialTTL.flush();
    SerialTTL.write("\x4C\x50\x46\x32\x2D\x54\x49\x4C\x54\x00\x00\x00\x00\x00\x00\x00", 16);SerialTTL.flush();
    SerialTTL.write("\x1E\x99\x01\x00\x00\x00\x00\x00\x00\x20\x41\x06\x99\x02\x00\x00", 16);SerialTTL.flush();
    SerialTTL.write("\x00\x00\x00\x00\xC8\x42\xEE\x99\x03\x00\x00\x00\x00\x00\x00\x20", 16);SerialTTL.flush();
    SerialTTL.write("\x41\x04\x91\x04\x44\x49\x52\x00\x35\x89\x05\x04\x00\x77\x91\x80", 16);SerialTTL.flush();
    SerialTTL.write("\x01\x00\x02\x00\xED\xA0\x00\x4C\x50\x46\x32\x2D\x41\x4E\x47\x4C", 16);SerialTTL.flush();
    SerialTTL.write("\x45\x00\x00\x00\x00\x00\x00\x5B\x98\x01\x00\x00\x34\xC2\x00\x00", 16);SerialTTL.flush();
    SerialTTL.write("\x34\x42\xE6\x98\x02\x00\x00\xC8\xC2\x00\x00\xC8\x42\xE5\x98\x03", 16);SerialTTL.flush();
    SerialTTL.write("\x00\x00\x34\xC2\x00\x00\x34\x42\xE4\x90\x04\x44\x45\x47\x00\x2D", 16);SerialTTL.flush();
    SerialTTL.write("\x88\x05\x10\x00\x62\x90\x80\x02\x00\x03\x00\xEE\x04\xF0\x00\x40", 16);SerialTTL.flush();
    SerialTTL.write("\x22\x9D\x49\x03\x02\xB7\x52\x00\xC2\x01\x00\x6E\x5F\x00\x00\x00", 16);SerialTTL.flush();
    SerialTTL.write("\x10\x00\x00\x00\x10\xA0\x9B\x00\x4C\x50\x46\x32\x2D\x43\x41\x4C", 16);SerialTTL.flush();
    SerialTTL.write("\x6F\x9B\x01\x00\x00\x34\xC2\x00\x00\x34\x42\xE5\x9B\x02\x00\x00", 16);SerialTTL.flush();
    SerialTTL.write("\xC8\xC2\x00\x00\xC8\x42\xE6\x9B\x03\x00\x00\x34\xC2\x00\x00\x34", 16);SerialTTL.flush();
    SerialTTL.write("\x42\xE7\x93\x04\x43\x41\x4C\x00\x26\x8B\x05\x10\x00\x61\x93\x80", 16);SerialTTL.flush();
    SerialTTL.write("\x03\x00\x03\x00\xEC\xA2\x00\x4C\x50\x46\x32\x2D\x43\x52\x41\x53", 16);SerialTTL.flush();
    SerialTTL.write("\x48\x00\x00\x00\x00\x00\x00\x53\x9A\x01\x00\x00\x00\x00\x00\x00", 16);SerialTTL.flush();
    SerialTTL.write("\xC8\x42\xEE\x9A\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xED\x9A\x03", 16);SerialTTL.flush();
    SerialTTL.write("\x00\x00\x00\x00\x00\x00\xC8\x42\xEC\x92\x04\x43\x4E\x54\x00\x30", 16);SerialTTL.flush();
    SerialTTL.write("\x8A\x05\x10\x00\x60\x92\x80\x03\x00\x03\x00\xED\xA1\x00\x4C\x50", 16);SerialTTL.flush();
    SerialTTL.write("\x46\x32\x2D\x54\x49\x4C\x54\x00\x00\x00\x00\x00\x00\x00\x1E\x99", 16);SerialTTL.flush();
    SerialTTL.write("\x01\x00\x00\x00\x00\x00\x00\x20\x41\x06\x99\x02\x00\x00\x00\x00", 16);SerialTTL.flush();
    SerialTTL.write("\x00\x00\xC8\x42\xEE\x99\x03\x00\x00\x00\x00\x00\x00\x20\x41\x04", 16);SerialTTL.flush();
    SerialTTL.write("\x91\x04\x44\x49\x52\x00\x35\x89\x05\x04\x00\x77\x91\x80\x01\x00", 16);SerialTTL.flush();
    SerialTTL.write("\x02\x00\xED\xA0\x00\x4C\x50\x46\x32\x2D\x41\x4E\x47\x4C\x45\x00", 16);SerialTTL.flush();
    SerialTTL.write("\x00\x00\x00\x00\x00\x5B\x98\x01\x00\x00\x34\xC2\x00\x00\x34\x42", 16);SerialTTL.flush();
    SerialTTL.write("\xE6\x98\x02\x00\x00\xC8\xC2\x00\x00\xC8\x42\xE5\x98\x03\x00\x00", 16);SerialTTL.flush();
    SerialTTL.write("\x34\xC2\x00\x00\x34\x42\xE4\x90\x04\x44\x45\x47\x00\x2D\x88\x05", 16);SerialTTL.flush();
    SerialTTL.write("\x10\x00\x62\x90\x80\x02\x00\x03\x00\xEE\x04",11);SerialTTL.flush();
	delay(5);
}


/**
 * @brief Handle the protocol queries & responses from/to the hub.
 *      Queries can be read/write according to the requested mode.
 * @warning In the situation where the processing of the responses to the
 *      queries from the hub takes longer than 200ms, a disconnection
 *      will be performed here.
 */
void TiltSensor::handleModes(){
    if (SerialTTL.available() == 0)
        return;

    unsigned char header = SerialTTL.read();

    if (header == 0x02) {  // NACK
        m_lastAckTick = millis();

        // Send default mode: 0 (angles data)
        this->sensorAngleMode();
    } else if (header == 0x43) {
        // "Get value" commands (3 bytes message: header, mode, checksum)
        size_t ret = SerialTTL.readBytes(m_rxBuf, 2);
        if (ret < 2) {
            // check if all expected bytes are received without timeout
            DEBUG_PRINT("incomplete 0x43 message");
            return;
        }

        switch (m_rxBuf[0]) {
            case TiltSensor::PBIO_IODEV_MODE_PUP_WEDO2_TILT_SENSOR__ANGLE:
                this->sensorAngleMode();
                break;
            default:
                break;
        }
    }
}


/**
 * @brief Mode 0 response (read): Send X/roll/roulis, Y/pitch/tangage angles.
 */
void TiltSensor::sensorAngleMode(){
    // Mode 0
    m_txBuf[0] = 0xC8;                       // header (LUMP_MSG_TYPE_DATA, mode 0, size 4)
    m_txBuf[1] = _(uint8_t)(*m_sensorTiltX); // X/roll
    m_txBuf[2] = _(uint8_t)(*m_sensorTiltY); // Y/pitch
    sendUARTBuffer(2);
}
