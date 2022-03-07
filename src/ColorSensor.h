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
#ifndef COLOR_SENSOR_H
#define COLOR_SENSOR_H

#include "BaseSensor.h"


// Colors (detected & LED (except NONE for this last one)) expected values
#define COLOR_NONE      0xFF
#define COLOR_BLACK     0
#define COLOR_PINK      1
#define COLOR_PURPLE    2
#define COLOR_BLUE      3
#define COLOR_LIGHTBLUE 4
#define COLOR_CYAN      5
#define COLOR_GREEN     6
#define COLOR_YELLOW    7
#define COLOR_ORANGE    8
#define COLOR_RED       9
#define COLOR_WHITE     10

// CMD_EXT_MODE payload
#define EXT_MODE_0      0x00  // for mode numbers < 8
#define EXT_MODE_8      0x08  // for mode numbers >= 8

/**
 * @brief Handle the LegoUART protocol and define modes of the
 *      Spike/Technic Color Sensor.
 *
 * @param m_sensorColor Detected color; Available values:
 *      COLOR_NONE, COLOR_BLACK, COLOR_BLUE, COLOR_LIGHTBLUE, COLOR_GREEN,
 *      COLOR_YELLOW, COLOR_RED, COLOR_PURPLE, COLOR_WHITE.
 *      The color of a surface or illuminated objects is obtained through
 *      the RGB values obtained by measuring the reflected light.
 *      For the identification of colors of a screen or external light sources,
 *      the identification is done via the HSV color set.
 *      See https://docs.pybricks.com/en/stable/pupdevices/colorsensor.html
 *      See https://github.com/pybricks/pybricks-micropython/blob/master/pybricks/pupdevices/pb_type_pupdevices_colorsensor.c
 * @param m_reflectedLight Reflected light (from clear channel value or
 *      calculations based on rgb channels).
 *      In theory, it's the sum of RGB channels, divided by 1024*3, scaled to 100.
 *      Continuous values 0...100.
 * @param m_ambientLight Ambient light based on lux value.
 *      In theory, it's the value of Value in SHSV array.
 *      Continuous values 0...100.
 * @param m_LEDBrightnesses This sensor has 3 built-in lights:
 *      0: left, 1: bottom, 2: right.
 *      Values in the array are the brightness of each light.
 *      (supposed to be transmitted via the Power Functions RC Protocol).
 * @param m_sensorRGB_I Raw values of Red Green Blue channels.
 *      It should be an array of uint16_t size 4.
 *      TODO: We use an array of size 3 (4th channel is unknown).
 *      Continuous values 0..1023.
 * @param m_sensorHSV Raw values of Hue, Saturation, Value/Brightness channels.
 *      Continuous values 0..1023.
 * @param m_pLEDBrightnessesfunc Callback set by user receiving m_LEDBrightnesses.
 *
 * @param m_currentExtMode Extended mode switch for modes >= 8. Available values:
 *      EXT_MODE_0, EXT_MODE_8.
 */
class ColorSensor : public BaseSensor {
    // LEGO SPIKE Color Sensor modes
    // Pybricks uses modes 3, 5, 7 only
    // https://github.com/pybricks/pybricks-micropython/blob/master/pybricks/util_pb/pb_device.h
    enum {
        PBIO_IODEV_MODE_PUP_COLOR_SENSOR__COLOR = 0,  // read 1x int8_t
        PBIO_IODEV_MODE_PUP_COLOR_SENSOR__REFLT = 1,  // read 1x int8_t
        PBIO_IODEV_MODE_PUP_COLOR_SENSOR__AMBI  = 2,  // read 1x int8_t
        PBIO_IODEV_MODE_PUP_COLOR_SENSOR__LIGHT = 3,  // writ 3x int8_t
        PBIO_IODEV_MODE_PUP_COLOR_SENSOR__RREFL = 4,  // read 2x int16_t
        PBIO_IODEV_MODE_PUP_COLOR_SENSOR__RGB_I = 5,  // read 4x int16_t
        PBIO_IODEV_MODE_PUP_COLOR_SENSOR__HSV   = 6,  // read 3x int16_t
        PBIO_IODEV_MODE_PUP_COLOR_SENSOR__SHSV  = 7,  // read 4x int16_t
        PBIO_IODEV_MODE_PUP_COLOR_SENSOR__DEBUG = 8,  // ??   2x int16_t
        //PBIO_IODEV_MODE_PUP_COLOR_SENSOR__CALIB = 9,  // ??   7x int16_t
    };

public:
    ColorSensor();
    ColorSensor(uint8_t *pSensorColor, uint16_t *pRGB_I, uint16_t *pHSV);
    virtual ~ColorSensor();

    uint16_t getSensorIRCode();
    void setSensorRGB_I(uint16_t *pData);
    void setSensorHSV(uint16_t *pData);
    void setSensorColor(uint8_t *pData);
    void setLEDBrightnessesCallback(void(pfunc)(const uint8_t*));
    void setSensorReflectedLight(uint8_t *pData);
    void setSensorAmbientLight(uint8_t *pData);

private:
    // Process queries from/to hub
    virtual void handleModes();
    // Protocol handy functions
    virtual void commSendInitSequence();
    void extendedModeInfoResponse();

    // Handle queries from the hub
    void setLEDBrightnessesMode();
    void sensorColorMode();
    void sensorReflectedLightMode();
    void sensorAmbientLight();
    void sensorRGB_IMode();
    void sensorHSVMode();
    //void sensorSHSVMode();
    void sensorDebugMode();

    uint8_t  *m_sensorColor;
    uint8_t  *m_reflectedLight;
    uint8_t  *m_ambientLight;
    uint8_t  *m_LEDBrightnesses;
    uint16_t *m_sensorRGB_I;
    uint16_t *m_sensorHSV;
    void     (*m_pLEDBrightnessesfunc)(const uint8_t*); // Callback for Led brightness change
    uint8_t *m_defaultIntVal;

    // UART protocol
    uint8_t m_currentExtMode = 0;
};

#endif
