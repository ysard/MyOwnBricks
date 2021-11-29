#ifndef BASICSENSOR_H
#define BASICSENSOR_H

#include "lego_uart.h"
#include "Arduino.h"

#if !defined(ARDUINO_AVR_PROMICRO) && !defined(ARDUINO_AVR_MICRO)
#include <cinttypes>
#endif

#if defined(ARDUINO_AVR_PROMICRO)
#define SerialTTL    Serial1
#define DbgSerial    Serial
#else
#define SerialTTL    Serial
#endif

#define _(type)    static_cast<type>

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
class BasicSensor {

public:
    BasicSensor();
    // virtual ~BasicSensor(){}
    // virtual void rocess();

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
    //virtual void commSendInitSequence(void);

    uint8_t m_connSerialRX_pin;
    uint8_t m_connSerialTX_pin;

    unsigned char m_rxBuf[16];
    unsigned char m_txBuf[16];
    unsigned long m_lastAckTick;

    bool m_connected;
};

#endif // BASICSENSOR_H
