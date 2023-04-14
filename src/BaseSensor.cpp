/*
 * A library for the emulation of PoweredUp sensors on microcontrollers
 * Copyright (C) 2021-2023 Ysard - <ysard@users.noreply.github.com>
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
#include "BaseSensor.h"

BaseSensor::BaseSensor() :
    m_connSerialRX_pin(0),
    m_connSerialTX_pin(1),
    m_lastAckTick(0),
    m_connected(false)
{}

/**
 * @brief Get status of connection with the hub.
 * @return bool
 */
bool BaseSensor::isConnected(){
    return m_connected;
}


/**
 * @brief Get checksum for the given message
 * @param pData Message array: Header + Payload
 * @param length Length of the payload (size WITHOUT header & checksum)
 * @return Checksum byte
 */
uint8_t BaseSensor::calcChecksum(uint8_t *pData, int length){
    uint8_t lRet, i;

    lRet = 0xFF;
    for(i=0; i<=length; i++){
        lRet ^= pData[i];
    }
    return lRet;
}


/**
 * @brief Wait until the hub is available on the serial RX line.
 *      Then, assert the TX line.
 */
void BaseSensor::commWaitForHubIdle(){
    // Disable uart: manual control TX and RX pins
    SerialTTL.end();

    unsigned long idletick;

    pinMode(m_connSerialTX_pin, OUTPUT);
    digitalWrite(m_connSerialTX_pin, LOW);

    pinMode(m_connSerialRX_pin, INPUT);
    idletick = millis();
    while (1) {
        if (digitalRead(m_connSerialRX_pin) == LOW) {
            idletick = millis();
        }
        if (millis() - idletick > 100) {
            break;
        }
    }

    digitalWrite(m_connSerialTX_pin, HIGH);
    delay(100);
    digitalWrite(m_connSerialTX_pin, LOW);
    delay(100);
}


/**
 * @brief Handle initialization of a connection to the hub.
 *      Workflow:
 *          - Wait RX line deasserted by the hub to LOW
 *          - Wait 100 ms
 *          - Assert TX line briefly
 *          - Start UART connection at 2400 bauds
 *          - Send sensor init sequence
 *          - Send ACK (0x04)
 *          - Wait ACK during 2s
 *          - Start UART connection at 115200 bauds
 */
void BaseSensor::connectToHub() {
    DEBUG_PRINTLN("INIT SENSOR");

    // Wait for HUB to idle it's TX pin (idle = High)
    // TODO: ces bidouilles émettent b'\x00\x00' avant tout choses sur la ligne série !!
    commWaitForHubIdle();

    // Starting initialization sequence
    commSendInitSequence();
    unsigned long starttime = millis();

    // Check if the hub send a ACK
    unsigned long currenttime = starttime;
    while ((currenttime - starttime) < 2000) {
        if (SerialTTL.available() > 0) {
            // read the incoming byte
            unsigned char dat = SerialTTL.read();
            if (dat == 0x04) { // ACK
                //DEBUG_PRINTLN("Connection established !");
                SerialTTL.begin(115200);
                m_connected   = true;
                m_lastAckTick = millis();
                break;
            }
        }
        currenttime = millis();
        delay(10);
    }
}


/**
 * @brief Initialize UART communication with specific sensor sequence.
 * @see https://github.com/pybricks/pybricks-micropython/lib/pbio/test/src/uartdev.c
 * @note Must be reimplemented in derived classes.
 *      This function is declared as virtual because connectToHub uses it
 *      and must use the derived method, sepcific of a sensor.
 */
void BaseSensor::commSendInitSequence(){}


/**
 * @brief Handle the protocol queries & responses from/to the hub.
 *      Queries can be read/write according to the requested mode.
 *      This function is specific to one sensor and MUST BE reimplemented.
 * @warning In the situation where the processing of the responses to the
 *      queries from the hub takes longer than 200ms, a disconnection
 *      will be performed here.
 */
void BaseSensor::handleModes(){}


/**
 * @brief Handle the connection process to the hub.
 * @see Protocol queries & responses are processed by `handleModes()`.
 * @warning Do not forget to check at each iteration if `millis() - m_lastAckTick > 200`.
 *      If true, the device must go in reset mode by setting the m_connected
 *      boolean to false.
 */
void BaseSensor::process(){
    if(!m_connected){
        this->connectToHub();
        return;
    }

    // Connection established
    handleModes();

    // Check disconnection from the Hub and go in reset/init mode if needed
    if (millis() - m_lastAckTick > 200) {
        INFO_PRINT(F("Disconnect; Too much time since last NACK - "));
        INFO_PRINTLN(millis() - m_lastAckTick);
        m_connected = false;
    }
}


/**
 * @brief Get header from the given message type, mode and size
 * @param msg_type Basically lump_msg_type_t::LUMP_MSG_TYPE_DATA for emitted messages.
 * @param mode Mode number.
 * @param msg_size Size of the message WITH header & checksum!
 *
 * @warning msg_size is multiplied by 3 in order the given params fits in expected
 *      headers (There are restrictions due to the masks used).
 * @todo Maybe we should expect the payload size instead of the full message size.
 *      It is already the use case of sendUARTBuffer and it's not intuitive to have
 *      2 behaviors... Also this function is able to automatically add padding bytes
 *      to it's computation (Ex: payload of size 8 is considered of size 10).
 * @return Header byte
 */
uint8_t BaseSensor::getHeader(
        const lump_msg_type_t& msg_type,
        const uint8_t& mode,
        const uint8_t& msg_size){
    return (msg_type & LUMP_MSG_TYPE_MASK) | \
           (mode & LUMP_MSG_CMD_MASK) | \
           ((msg_size * 3) & LUMP_MSG_SIZE_MASK);
}


/**
 * @brief Get mode and message size from the given header.
 *      Currently used to parse 2nd part of write queries
 *      (1st part has the known header 0x46).
 * @param header Header of a received/transmitted message.
 * @param mode Reference See the class enumeration of modes.
 * @param msg_size Reference to message size.
 */
void BaseSensor::parseHeader(const uint8_t& header, uint8_t& mode, uint8_t& msg_size){
    // Type is known to be LUMP_MSG_TYPE_DATA because of 0x46 header
    // msg_type = header & LUMP_MSG_TYPE_MASK;
    mode     = header & LUMP_MSG_CMD_MASK;
    msg_size = getMsgSize(header);
}


/**
 * @brief Get size of a message from the given header. Used by parseHeader().
 *
 * @warning /!\ This is a simplified version that works on LUMP_MSG_TYPE_DATA messages.
 *      DO NOT use on LUMP_MSG_TYPE_SYS, LUMP_MSG_TYPE_INFO messages.
 * @param header
 * @return Expected size
 */
uint8_t BaseSensor::getMsgSize(const uint8_t& header){
    // Simplified version that implicitly asserts that msg_type is LUMP_MSG_TYPE_DATA
    return _(uint8_t) ((1 << (((header) >> 3) & 0x7)) + 2);
}


/**
 * @brief Send the TX buffer content to the hub
 *      Also add the checksum of the message.
 * @param msg_size Size of the message WITHOUT header & checksum: Payload size.
 */
void BaseSensor::sendUARTBuffer(uint8_t msg_size){
    // Add checksum to the last index
    m_txBuf[msg_size + 1] = calcChecksum(this->m_txBuf, msg_size);
    // Send data (size = payload + header + checksum = payload + 2)
    SerialTTL.write((char *)this->m_txBuf, msg_size + 2);
    SerialTTL.flush();
}
