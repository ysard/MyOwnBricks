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
#include "basicsensor.h"

// Colors (detected & LED (except NONE for this last one)) expected values
#define COLOR_NONE     0xFF
#define COLOR_BLACK    0
#define COLOR_BLUE     3
#define COLOR_GREEN    5
#define COLOR_RED      9
#define COLOR_WHITE    10

// CMD_EXT_MODE payload
#define EXT_MODE_0     0x00  // for mode numbers < 8
#define EXT_MODE_8     0x08  // for mode numbers >= 8

/**
 * @brief Handle the LegoUART protocol and define modes of the
 * Color & Distance sensor.
 *
 * @param m_LEDColor Current color of the LED; Available values:
 *      COLOR_BLACK, COLOR_BLUE, COLOR_GREEN, COLOR_RED, COLOR_WHITE.
 * @param m_sensorDistance Distance measured to the the nearest object.
 *      Discretized values 0..10.
 * @param m_reflectedLight Reflected light (from clear channel value or
 *      calculations based on rgb channels).
 *      Discretized values 0..5F.
 * @param m_ambientLight Ambient light based on lux value.
 *      Discretized values ???
 * @param m_sensorRGB Raw values of Red Green Blue channels.
 * @param m_sensorColor Detected color. See available values of m_LEDColor
 *      + COLOR_NONE.
 * @param m_IR_code IR code for Power Functions IR devices
 *      (supposed to be transmitted via the Power Functions RC Protocol).
 * @param m_pIRfunc Callback set by user receiving m_IR_code.
 * @param m_pLEDColorfunc Callback set by user receiving m_LEDColor.
 *
 * @param m_currentExtMode Extended mode switch for modes >= 8. Available values:
 *      EXT_MODE_0, EXT_MODE_8.
 */
class LegoPupColorDistance : public BasicSensor {
    // LEGO POWERED UP Color and Distance Sensor modes
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
    virtual ~LegoPupColorDistance();
    void process();

    uint16_t getSensorIRCode();
    void setSensorColor(uint8_t *pData);
    void setSensorDistance(uint8_t *pData);
    void setSensorRGB(uint16_t *pData);
    void setIRCallback(void(pfunc)(const uint16_t));
    void setSensorLEDColor(uint8_t *pData);
    void setLEDColorCallback(void(pfunc)(const uint8_t));
    void setSensorReflectedLight(uint8_t *pData);
    void setSensorAmbientLight(uint8_t *pData);

private:
    // Protocol handy functions
    void commSendInitSequence();
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

    uint8_t m_currentExtMode;
    uint8_t *m_defaultIntVal;
};

#endif
