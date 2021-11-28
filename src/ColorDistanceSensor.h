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
#ifndef LegoPupColorDistance_h
#define LegoPupColorDistance_h

#include "lego_uart.h"
#include "Arduino.h"

#if !defined(ARDUINO_AVR_PROMICRO) && !defined(ARDUINO_AVR_MICRO)
#include <cinttypes>
#endif

#if defined(ARDUINO_AVR_PROMICRO)
#define SerialTTL Serial1
#define DbgSerial Serial
#else
#define SerialTTL Serial
#endif

#define _(type) static_cast<type>

#define COLOR_NONE     0xFF
#define COLOR_BLACK    0
#define COLOR_BLUE     3
#define COLOR_GREEN    5
#define COLOR_RED      9
#define COLOR_WHITE    10

class LegoPupColorDistance {
    // LEGO POWERED UP Color and Distance Sensor
    enum {
        PBIO_IODEV_MODE_PUP_COLOR_DISTANCE_SENSOR__COLOR = 0, // read 1x int8_t
        PBIO_IODEV_MODE_PUP_COLOR_DISTANCE_SENSOR__PROX  = 1, // read 1x int8_t
        //PBIO_IODEV_MODE_PUP_COLOR_DISTANCE_SENSOR__COUNT = 2,  // read 1x int32_t
        PBIO_IODEV_MODE_PUP_COLOR_DISTANCE_SENSOR__REFLT = 3, // read 1x int8_t
        PBIO_IODEV_MODE_PUP_COLOR_DISTANCE_SENSOR__AMBI  = 4, // read 1x int8_t
        PBIO_IODEV_MODE_PUP_COLOR_DISTANCE_SENSOR__COL_O = 5, // writ 1x int8_t
        PBIO_IODEV_MODE_PUP_COLOR_DISTANCE_SENSOR__RGB_I = 6, // read 3x int16_t
        PBIO_IODEV_MODE_PUP_COLOR_DISTANCE_SENSOR__IR_TX = 7, // writ 1x int16_t
        PBIO_IODEV_MODE_PUP_COLOR_DISTANCE_SENSOR__SPEC1 = 8, // rrwr 4x int8_t
        PBIO_IODEV_MODE_PUP_COLOR_DISTANCE_SENSOR__DEBUG = 9, // ?? 2x int16_t
        //PBIO_IODEV_MODE_PUP_COLOR_DISTANCE_SENSOR__CALIB = 10, // ?? 8x int16_t
    };

public:
    LegoPupColorDistance();
    LegoPupColorDistance(uint8_t *pSensorColor, uint8_t *pSensorDistance);
    void Process(void);

    void setSensorColor(uint8_t *pData);
    void setSensorDistance(uint8_t *pData);
    void setSensorRGB(uint16_t *pData);
    uint16_t getSensorIRCode();
    void setIRCallback(void (pfunc)(const uint16_t));
    void setSensorLEDColor(uint8_t *pData);
    void setLEDColorCallback(void (pfunc)(const uint8_t));
    void setSensorReflectedLight(uint8_t *pData);
    void setSensorAmbientLight(uint8_t *pData);

    bool isConnected(void);

private:
    // Protocol handy functions
    uint8_t calcChecksum(uint8_t *pData, int length);
    uint8_t getHeader(const lump_msg_type_t& msg_type, const uint8_t& mode, const uint8_t& msg_size);
    void parseHeader(const uint8_t& header, uint8_t& mode, uint8_t& msg_size);
    uint8_t getMsgSize(const uint8_t& header);
    void sendUARTBuffer(uint8_t msg_size);
    void commWaitForHubIdle(void);
    void commSendInitSequence(void);
    void extendedModeInfoResponse();

    // Handle queries from the hub
    void setLEDColorMode();
    void setIRTXMode();
    void LEDColorMode();
    void sensorDistanceMode();
    void sensorReflectedLightMode();
    void sensorAmbientLight();
    void sensorRGBIMode();
    void sensorSpec1Mode();
    void sensorDebugMode();

    uint8_t *m_LEDColor;
    uint8_t *m_sensorDistance;
    uint8_t *m_reflectedLight;
    uint8_t *m_ambientLight;
    uint16_t *m_sensorRGB;
    uint16_t m_IR_code;
    uint8_t *m_sensorColor;
    void (*m_pIRfunc)(const uint16_t);      // Callback for IR change
    void (*m_pLEDColorfunc)(const uint8_t); // Callback for Led color change

    uint8_t m_connSerialRX_pin;
    uint8_t m_connSerialTX_pin;
    uint8_t m_currentExtMode;

    unsigned char m_rxBuf[32];
    unsigned char m_txBuf[32];
    unsigned long m_lastAckTick;

    bool m_connected;
};

#endif
