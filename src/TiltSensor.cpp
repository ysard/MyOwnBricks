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
 * @brief Send initialization sequences for the current sensor.
 * @see https://github.com/pybricks/pybricks-micropython/lib/pbio/test/src/uartdev.c
 */
void TiltSensor::commSendInitSequence(){
    // Initialize uart
    SerialTTL.begin(2400);

    SerialTTL.write("\x40\x22\x9D", 3);                              // Type ID: 0x22
    SerialTTL.write("\x49\x03\x02\xB7", 4);                          // CMD_MODES: modes: 4, views: 3, Ext. Modes: 0 modes, 0 views
    SerialTTL.write("\x52\x00\xC2\x01\x00\x6E", 6);                  // CMD_SPEED: 115200
    SerialTTL.write("\x5F\x00\x00\x00\x10\x00\x00\x00\x10\xA0", 10); // CMD_VERSION: fw-version: 1.0.0.0, hw-version: 1.0.0.0
    SerialTTL.flush();

    SerialTTL.write("\x9B\x00\x4C\x50\x46\x32\x2D\x43\x41\x4C\x6F", 11);                                 // Name: "LPF2-CAL"
    SerialTTL.write("\x9B\x01\x00\x00\x34\xC2\x00\x00\x34\x42\xE5", 11);                                 // Range: -45.0 to 45.0
    SerialTTL.write("\x9B\x02\x00\x00\xC8\xC2\x00\x00\xC8\x42\xE6", 11);                                 // PCT Range: -100.0% to 100.0%
    SerialTTL.write("\x9B\x03\x00\x00\x34\xC2\x00\x00\x34\x42\xE7", 11);                                 // Si Range: -45.0 to 45.0
    SerialTTL.write("\x93\x04\x43\x41\x4C\x00\x26", 7);                                                  // Si Symbol: CAL
    SerialTTL.write("\x8B\x05\x10\x00\x61", 5);                                                          // input_flags: Absolute, output_flags: None
    SerialTTL.write("\x93\x80\x03\x00\x03\x00\xEC", 7);                                                  // Format: 3 int8, each 3 chars, 0 decimals
    SerialTTL.flush();

    SerialTTL.write("\xA2\x00\x4C\x50\x46\x32\x2D\x43\x52\x41\x53\x48\x00\x00\x00\x00\x00\x00\x53", 19); // Name: "LPF2-CRASH"
    SerialTTL.write("\x9A\x01\x00\x00\x00\x00\x00\x00\xC8\x42\xEE", 11);                                 // Range:0.0 to 100.0
    SerialTTL.write("\x9A\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xED", 11);                                 // PCT Range: 0.0% to 100.0%
    SerialTTL.write("\x9A\x03\x00\x00\x00\x00\x00\x00\xC8\x42\xEC", 11);                                 // Si Range: 0.0 to 100.0
    SerialTTL.write("\x92\x04\x43\x4E\x54\x00\x30", 7);                                                  // Si Symbol: CNT
    SerialTTL.write("\x8A\x05\x10\x00\x60", 5);                                                          // input_flags: Absolute, output_flags: None
    SerialTTL.write("\x92\x80\x03\x00\x03\x00\xED", 7);                                                  // Format: 3 int8, each 3 chars, 0 decimals
    SerialTTL.flush();

    SerialTTL.write("\xA1\x00\x4C\x50\x46\x32\x2D\x54\x49\x4C\x54\x00\x00\x00\x00\x00\x00\x00\x1E", 19); // Name: "LPF2-TILT"+ flags
    SerialTTL.write("\x99\x01\x00\x00\x00\x00\x00\x00\x20\x41\x06", 11);                                 // Range: 0.0 to 10.0
    SerialTTL.write("\x99\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xEE", 11);                                 // PCT Range: 0.0% to 100.0%
    SerialTTL.write("\x99\x03\x00\x00\x00\x00\x00\x00\x20\x41\x04", 11);                                 // Si Range: 0.0 to 10.0
    SerialTTL.write("\x91\x04\x44\x49\x52\x00\x35", 7);                                                  // Si Symbol: DIR
    SerialTTL.write("\x89\x05\x04\x00\x77", 5);                                                          // input_flags: Discrete, output_flags: None
    SerialTTL.write("\x91\x80\x01\x00\x02\x00\xED", 7);                                                  // Format: 1 int8, each 2 chars, 0 decimals
    SerialTTL.flush();

    SerialTTL.write("\xA0\x00\x4C\x50\x46\x32\x2D\x41\x4E\x47\x4C\x45\x00\x00\x00\x00\x00\x00\x5B", 19); // Name: "LPF2-ANGLE"+ flags
    SerialTTL.write("\x98\x01\x00\x00\x34\xC2\x00\x00\x34\x42\xE6", 11);                                 // Range: -45.0 to 45.0
    SerialTTL.write("\x98\x02\x00\x00\xC8\xC2\x00\x00\xC8\x42\xE5", 11);                                 // PCT Range: -100.0% to 100.0%
    SerialTTL.write("\x98\x03\x00\x00\x34\xC2\x00\x00\x34\x42\xE4", 11);                                 // Si Range: -45.0 to 45.0
    SerialTTL.write("\x90\x04\x44\x45\x47\x00\x2D", 7);                                                  // Si Symbol: DEG
    SerialTTL.write("\x88\x05\x10\x00\x62", 5);                                                          // input_flags: Absolute, output_flags: None
    SerialTTL.write("\x90\x80\x02\x00\x03\x00\xEE", 7);                                                  // Format: 2 int8, each 3 chars, 0 decimals
    SerialTTL.flush();

    SerialTTL.write("\x04", 1); // ACK
    SerialTTL.flush();
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
