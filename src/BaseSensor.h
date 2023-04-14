/*
 * MyOwnBricks is a library for the emulation of PoweredUp sensors on microcontrollers
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
#ifndef BASESENSOR_H
#define BASESENSOR_H

#include "global.h"
#include "lego_uart.h"
#include "Arduino.h"


/**
 * @brief Handle basic functions for LegoUART protocol.
 *      Designed to be inherited in specific classes of sensors.
 *
 * @param m_connSerialRX_pin Serial RX pin of the board. (default: 0).
 * @param m_connSerialTX_pin Serial TX pin of the board. (default: 1).
 * @param m_rxBuf Buffer used to store bytes emitted by the hub.
 * @param m_txBug Buffer used to store bytes before being sent to the hub.
 * @param m_lastAckTick Time flag used to detect disconnection from the hub.
 * @param m_connected Connection flag.
 */
class BaseSensor {

public:
    BaseSensor();
    // virtual ~BasicSensor(){}
    void process();
    bool isConnected();

protected:
    // Protocol handy functions
    uint8_t calcChecksum(uint8_t *pData, int length);
    uint8_t getHeader(const lump_msg_type_t& msg_type, const uint8_t& mode, const uint8_t& msg_size);
    void parseHeader(const uint8_t& header, uint8_t& mode, uint8_t& msg_size);
    uint8_t getMsgSize(const uint8_t& header);
    void sendUARTBuffer(uint8_t msg_size);
    void commWaitForHubIdle();
    void connectToHub();
    // Could/should use virtual pure (..() = 0) but it uses 14bytes for nothing
    virtual void commSendInitSequence();
    virtual void handleModes();

    uint8_t m_connSerialRX_pin;
    uint8_t m_connSerialTX_pin;

    unsigned char m_rxBuf[16];
    unsigned char m_txBuf[16];
    unsigned long m_lastAckTick;

    bool m_connected;
};

#endif // BASESENSOR_H
