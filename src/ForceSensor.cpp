/*
 * A library for the emulation of PoweredUp sensors on microcontrollers
 * Copyright (C) 2022 Ysard - <ysard@users.noreply.github.com>
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
#include "ForceSensor.h"

/**
 * @brief Default constructor
 */
ForceSensor::ForceSensor(){
    m_force = nullptr;
    m_rawForce= nullptr;
    m_touched = nullptr;
    m_tapped = nullptr;
    m_defaultComboModesEnabled = false;
}


/**
 * @brief Constructor allowing to set force value, touched status, tapped force.
 * @param pForce
 * @param pTouched
 * @param pTapped
 */
ForceSensor::ForceSensor(uint8_t *pForce, bool *pTouched, uint8_t *pTapped){
    m_force = pForce;
    m_touched = pTouched;
    m_tapped = pTapped;
    m_rawForce = nullptr;
    m_defaultComboModesEnabled = false;
}

/**
 * @brief Setter for m_force
 * @param pData
 */
void ForceSensor::setSensorForce(int8_t *pData){
    m_force = pData;
}


/**
 * @brief Setter for m_touched
 * @param pData
 */
void ForceSensor::setSensorTouched(bool *pData){
    m_touched = pData;
}


/**
 * @brief Setter for m_tapped
 * @param pData
 */
void ForceSensor::setSensorTapped(uint8_t *pData){
    m_tapped = pData;
}


/**
 * @brief Setter for m_rawForce
 * @param pData
 */
void ForceSensor::setSensorRawForce(uint16_t *pData){
    m_rawForce = pData;
}


/**
 * @brief Setter for sensor calibration values.
 * @param raw_offset
 * @param raw_released
 * @param raw_end
 */
void ForceSensor::setSensorCalibrationValues(uint16_t raw_offset, uint16_t raw_released, uint16_t raw_end){
    m_raw_offset   = raw_offset;
    m_raw_released = raw_released;
    m_raw_end      = raw_end;
}


/**
 * @brief Send initialization sequences for the current sensor.
 * @see https://github.com/pybricks/pybricks-micropython/lib/pbio/test/src/uartdev.c
 * @todo fullfill init sequence
 */
void ForceSensor::commSendInitSequence(){
    // Initialize uart
    SerialTTL.begin(2400);

    SerialTTL.write("\x40\x3F\x82", 3);                              // Type ID: 0x3F
    SerialTTL.write("\x51???", 6);                  // CMD_MODES: ? modes, ? views, Ext. Modes: ? modes, ? view
    SerialTTL.write("\x52\x00\xC2\x01\x00\x6E", 6);                  // CMD_SPEED: 115200
    SerialTTL.write("\x5F\x00\x00\x00\x10\x00\x00\x00\x10\xA0", 10); // CMD_VERSION
    SerialTTL.flush();
    delay(10);
}


/**
 * @brief Handle the protocol queries & responses from/to the hub.
 *      Queries can be read/write according to the requested mode.
 * @warning In the situation where the processing of the responses to the
 *      queries from the hub takes longer than 200ms, a disconnection
 *      will be performed here.
 * @todo Definitive fix for combos mode messages
 */
void ForceSensor::handleModes(){
    if (SerialTTL.available() == 0)
        return;

    unsigned char header = SerialTTL.read();

    if (header == 0x02) {  // NACK
        m_lastAckTick = millis();
        // Note: In theory the default mode is always the lowest (0).
        // If combos mode is enabled, prefer to send this data
        if (m_defaultComboModesEnabled)
            this->defaultCombosMode();
        else
            this->sensorAngleMode();
    } else if (header == 0x43) {
        // "Get value" commands (3 bytes message: header, mode, checksum)
        size_t ret = SerialTTL.readBytes(m_rxBuf, 2);
        if (ret < 2) {
            // check if all expected bytes are received without timeout
            DEBUG_PRINT("incomplete 0x43 message");
            return;
        }

        // Test requested mode
        switch (m_rxBuf[0]) {
            case ForceSensor::PBIO_IODEV_MODE_PUP_FORCE_SENSOR__FORCE:
                this->sensorForceMode();
                break;
            case ForceSensor::PBIO_IODEV_MODE_PUP_FORCE_SENSOR__TOUCHED:
                this->sensorTouchedMode();
                break;
            case ForceSensor::PBIO_IODEV_MODE_PUP_FORCE_SENSOR__TAPPED:
                this->sensorTappedMode();
                break;
            case ForceSensor::PBIO_IODEV_MODE_PUP_FORCE_SENSOR__FRAW:
                this->sensorForceRawMode();
                break;
            case ForceSensor::PBIO_IODEV_MODE_PUP_FORCE_SENSOR__CALIB:
                this->sensorCalibrationMode();
                break;
            default:
                break;
        }
    } else if (header == 0x4C) {
        // Reset the Combination modes (supposed to)
        // Currently (03/2022) the packet is the following:
        // { 4C 20 00 93 }
        // Note: There is no parsing of the message, we just check the checksum
        // and discard the message if it doesn't match.

        // Get data (4 bytes message)
        size_t ret = SerialTTL.readBytes(m_rxBuf, 3);
        if (ret < 3) {
            // check if all expected bytes are received without timeout
            return;
        }

        if (m_rxBuf[2] != 0x93)
            // Structure not expected
            return;

        this->m_defaultComboModesEnabled = false;
        // Send acknowledgement
        this->ackResetCombosMode();

    } else if (header == 0x5C) {
        // Receive a combination modes query to define the default data to send after each NACK
        // Currently (03/2022) the packet is the following:
        // { 5C 23 00 00 10 40 00 00 00 D0 }
        // Note: There is no parsing of the message, we just check the checksum
        // and discard the message if it doesn't match.

        // Get data (10 bytes message)
        size_t ret = SerialTTL.readBytes(m_rxBuf, 9);
        if (ret < 9) {
            // check if all expected bytes are received without timeout
            DEBUG_PRINT(F("incomplete combos message"));
            return;
        }

        if (m_rxBuf[8] != 0xD0)
            // Structure not expected
            return;

        this->m_defaultComboModesEnabled = true;
        // Send acknowledgement
        this->ackSetCombosMode();
    }
}


/**
 * @brief Mode 0 response (read): Send force value.
 */
void ForceSensor::sensorForceMode(){
    m_txBuf[0] = 0xC0;                       // header (LUMP_MSG_TYPE_DATA, mode 0, size 3)
    m_txBuf[1] = *m_force;
    sendUARTBuffer(1);
}


/**
 * @brief Mode 1 response (read): Touched status.
 */
void ForceSensor::sensorTouchedMode(){
    m_txBuf[0] = 0xC1;                       // header (LUMP_MSG_TYPE_DATA, mode 1, size 3)
    m_txBuf[1] = *m_touched;
    sendUARTBuffer(1);
}


/**
 * @brief Mode 2 response (read): Send Tapped status.
 */
void ForceSensor::sensorTappedMode(){
    m_txBuf[0] = 0xC2;                       // header (LUMP_MSG_TYPE_DATA, mode 1, size 3)
    m_txBuf[1] = *m_tapped;
    sendUARTBuffer(1);
}


/**
 * @brief Mode 4 response (read): Send raw force value.
 */
void ForceSensor::sensorForceRawMode(){
    m_txBuf[0] = 0xCC;                       // header (LUMP_MSG_TYPE_DATA, mode 4, size 4)
    m_txBuf[1] = (m_rawForce) ? m_rawForce & 0xFF : 0;        // Send LSB
    m_txBuf[2] = (m_rawForce) ? (m_rawForce >> 8) & 0xFF : 0; // Send MSB
    sendUARTBuffer(2);
}


/**
 * @brief Mode 6 response (read): Send calibration array.
 *      Positions:
 *          1: raw_offset
 *          2: raw_released
 *          6: raw_end
 */
void ForceSensor::sensorCalibrationMode(){
    // Send data; payload size = 16; total msg_size = 18
    m_txBuf[0] = getHeader(lump_msg_type_t::LUMP_MSG_TYPE_DATA, 0, 18); // header: 0xe6

    // Init the array
    for (uint8_t i=1; i<= 16; i++) {
        m_txBuf[i] = 0;
    }
    // 2nd value
    m_txBuf[3] = m_raw_offset  & 0xFF;              // Send LSB
    m_txBuf[4] = (m_raw_offset >> 8) & 0xFF;        // Send MSB
    // 3rd value
    m_txBuf[5] = m_raw_released  & 0xFF;
    m_txBuf[6] = (m_raw_released >> 8) & 0xFF;
    // 6th value
    m_txBuf[13] = m_raw_end & 0xFF;
    m_txBuf[14] = (m_raw_end >> 8) & 0xFF;
    sendUARTBuffer(14);
}


/**
 * @brief Combo mode / multi-mode: Overwrite default Mode 0 response after receiving a NACK
 *      from the hub.
 *
 *      Packet dissection:
 *          5C 23 00 00 10 40 00 00 00 D0
 *          5C: header
 *          23: 0x20 | 0x03 : 3 bytes of tuples to follow
 *          00: unknown
 *          00: mode 0 value 0
 *          10: mode 1 value 0
 *          40: mode 4 value 0
 *          00: padding
 *          00: padding
 *          00: padding
 *          D0: checksum
 * @todo Parse dynamically the received packet. Have a struct that could map
 *      mode ids and values for an easier access.
 */
void ForceSensor::defaultCombosMode(){
    // Send data; payload size = 4; total msg_size = 6
    DEBUG_PRINTLN(F("Default combos mode"));

    // Send data
    m_txBuf[0] = getHeader(lump_msg_type_t::LUMP_MSG_TYPE_DATA, 0, 6);  // header: 0xd0
    m_txBuf[1] = *m_force;                                              // mode 0 value 0
    m_txBuf[2] = *m_touched;                                            // mode 1 value 0
                                                                        // mode 4: value 0
    m_txBuf[3] = (m_rawForce) ? m_rawForce & 0xFF : 0;                  // Send LSB
    m_txBuf[4] = (m_rawForce) ? (m_rawForce >> 8) & 0xFF : 0;           // Send MSB
    sendUARTBuffer(4);
}


/**
 * @brief Response to a reset combo mode query.
 */
void ForceSensor::ackResetCombosMode(){
    m_txBuf[0] = 0x44;
    m_txBuf[1] = 0x20;
    sendUARTBuffer(1);
}


/**
 * @brief Response to a combo mode query.
 *      It's the same package as the one received.
 * @todo Replay the received packet instead manually setting it.
 */
void ForceSensor::ackSetCombosMode(){
    m_txBuf[0] = 0x5C;
    m_txBuf[1] = 0x23;
    m_txBuf[2] = 0x00;
    m_txBuf[3] = 0x00;
    m_txBuf[4] = 0x10;
    m_txBuf[5] = 0x40;
    m_txBuf[6] = 0x00;
    m_txBuf[7] = 0x00;
    m_txBuf[8] = 0x00;
    sendUARTBuffer(8);
}

