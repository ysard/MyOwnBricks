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
#include "ColorSensor.h"

/**
 * @brief Default constructor
 */
ColorSensor::ColorSensor(){
    m_defaultIntVal  = new uint8_t(0);
    uint16_t defaultRGB[3] = {0, 0, 0};
    uint16_t defaultHSV[3] = {0, 0, 0};
    uint8_t LEDBrightnesses[3] = {0, 0, 0};

    // Sensor default values
    m_sensorColor     = m_defaultIntVal;
    m_reflectedLight  = m_defaultIntVal;
    m_ambientLight    = m_defaultIntVal;
    m_sensorRGB_I     = defaultRGB;
    m_sensorHSV       = defaultHSV;
    m_LEDBrightnesses = LEDBrightnesses;
    m_pLEDBrightnessesfunc = nullptr;

    // UART protocol
    m_currentExtMode = 0;
}


/**
 * @brief Constructor allowing to set detected color, RGB_I array pointers.
 * @overload
 * @param pSensorColor Pointer to a discretized detected color. See m_sensorColor.
 * @param pRGB_I Pointer to Raw values of Red Green Blue channels. See m_sensorRGB_I.
 * @param pHSV Pointer to Raw values of Hue, Saturation, Value/Brightness channels. See m_sensorHSV.
 */
ColorSensor::ColorSensor(uint8_t *pSensorColor, uint16_t *pRGB_I, uint16_t *pHSV){
    m_defaultIntVal  = new uint8_t(0);
    uint8_t LEDBrightnesses[3] = {0, 0, 0};

    // Set given values
    m_sensorColor = pSensorColor;
    m_sensorRGB_I = pRGB_I;
    m_sensorHSV   = pHSV;

    // Sensor default values
    m_reflectedLight  = m_defaultIntVal;
    m_ambientLight    = m_defaultIntVal;
    m_LEDBrightnesses = LEDBrightnesses;
    m_pLEDBrightnessesfunc = nullptr;

    // UART protocol
    m_currentExtMode = 0;
}


/**
 * @brief Destructor
 */
ColorSensor::~ColorSensor() {
    delete m_defaultIntVal;
    // TODO: Should also implement copy constructor or copy assignment operator
}

/**
 * @brief Setter for m_sensorRGB; Raw values of Red Green Blue channels.
 * @param pData Expected value pointed is an array of uint16_t size 4.
 *      TODO: We use an array of size 3 (4th channel is unknown).
 *      Continuous values 0..1023.
 */
void ColorSensor::setSensorRGB_I(uint16_t *pData){
    this->m_sensorRGB_I = pData;
}


/**
 * @brief Setter for m_sensorHSV; Raw values of Hue, Saturation, Value/Brightness channels
 * @param pData Expected value pointed is an array of uint16_t size 3.
 *      Continuous values 0..1023.
 */
void ColorSensor::setSensorHSV(uint16_t *pData){
    this->m_sensorHSV = pData;
}


/**
 * @brief Setter for m_sensorColor
 * @param pData: Pointer to the currently detected color.
 *      Available (official) values:
 *      COLOR_NONE, COLOR_BLACK, COLOR_BLUE, COLOR_LIGHTBLUE, COLOR_GREEN,
 *      COLOR_YELLOW, COLOR_RED, COLOR_PURPLE, COLOR_WHITE.
 */
void ColorSensor::setSensorColor(uint8_t *pData){
    // Free constructor's value
    this->m_sensorColor = pData;
}


/**
 * @brief Set callback receiving m_LEDBrightnesses when modified by the hub.
 */
void ColorSensor::setLEDBrightnessesCallback(void(pfunc)(const uint8_t*)){
    this->m_pLEDBrightnessesfunc = pfunc;
}


/**
 * @brief Setter for m_reflectedLight
 * @param pData Pointer to reflected light (from clear channel value or
 *      calculations based on rgb channels). Continuous values 0..100.
 */
void ColorSensor::setSensorReflectedLight(uint8_t *pData){
    this->m_reflectedLight = pData;
}


/**
 * @brief Setter for m_ambientLight
 * @param pData Pointer to ambient light based on lux value.
 *      Continuous values 0..100.
 */
void ColorSensor::setSensorAmbientLight(uint8_t *pData){
    this->m_ambientLight = pData;
}

/**
 * @brief Send initialization sequences for the current Color & Distance sensor.
 * @see https://github.com/pybricks/pybricks-micropython/lib/pbio/test/src/uartdev.c
 */
void ColorSensor::commSendInitSequence(){
    // TODO: put all this strings into flash via PROGMEM
    // Initialize uart
    SerialTTL.begin(2400);

    SerialTTL.write("\x40\x3D\x82",3); // Type ID: 0x3D
    SerialTTL.write("\x51\x07\x07\x0A\x07\xA3",6); // CMD_MODES
    SerialTTL.write("\x52\x00\xC2\x01\x00\x6E",6); // CMD_SPEED: 115200
    SerialTTL.write("\x5F\x00\x00\x00\x10\x00\x00\x00\x10\xA0",10); // CMD_VERSION
    SerialTTL.flush();
    delay(10);
    // TO BE ADDED
    SerialTTL.flush();
    delay(10);
    SerialTTL.write("\x04",1);
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
void ColorSensor::handleModes(){
    if (SerialTTL.available() == 0)
        return;

    unsigned char header;
    unsigned char mode;
    header = SerialTTL.read();

    DEBUG_PRINT(F("<\tHeader "));
    DEBUG_PRINTLN(header, HEX);

    if (header == 0x02) { // NACK
        m_lastAckTick = millis();
        // Note: In theory the default mode is always the lowest (0).
        this->sensorColorMode();
    } else if (header == 0x43) {
        // "Get value" commands (3 bytes message: header, mode, checksum)
        size_t ret = SerialTTL.readBytes(m_rxBuf, 2);
        if (ret < 2) {
            // check if all expected bytes are received without timeout
            DEBUG_PRINT(F("incomplete 0x43 message"));
            return;
        }
        mode = m_rxBuf[0];
        DEBUG_PRINT(F("<\tAsked mode "));
        DEBUG_PRINTLN(mode);

        this->m_currentExtMode = (mode < 8) ? EXT_MODE_0 : EXT_MODE_8;

        switch (mode) {
            case ColorSensor::PBIO_IODEV_MODE_PUP_COLOR_SENSOR__COLOR:
                this->sensorColorMode();
                break;
            case ColorSensor::PBIO_IODEV_MODE_PUP_COLOR_SENSOR__REFLT:
                this->sensorReflectedLightMode();
                break;
            case ColorSensor::PBIO_IODEV_MODE_PUP_COLOR_SENSOR__AMBI:
                this->sensorAmbientLight();
                break;
            /*case ColorSensor::PBIO_IODEV_MODE_PUP_COLOR_SENSOR__RREFL:
                this->sensorReflectedLightMode();
                break;*/
            case ColorSensor::PBIO_IODEV_MODE_PUP_COLOR_SENSOR__RGB_I:
                this->sensorRGB_IMode();
                break;
            case ColorSensor::PBIO_IODEV_MODE_PUP_COLOR_SENSOR__HSV:
                this->sensorHSVMode();
                break;
            //case ColorSensor::PBIO_IODEV_MODE_PUP_COLOR_SENSOR__SHSV:
            //    this->sensorSHSVMode();
            //    break;
            #ifdef DEBUG
            // This implementation doesn't follow Lego's one
            case ColorDistanceSensor::PBIO_IODEV_MODE_PUP_COLOR_SENSOR__DEBUG:
                this->sensorDebugMode();
                break;
            #endif
            default:
                INFO_PRINT(F("unknown R mode: "));
                INFO_PRINTLN(mode, HEX);
                break;
        }
    } else if (header == 0x46) {
        // "Set value" commands
        // The message has 2 parts (each with header, value and checksum):
        // - The EXT_MODE status as value
        // - The LUMP_MSG_TYPE_DATA itself with its data as value

        // Get data1, checksum1, header2 (header of the next message)
        size_t ret = SerialTTL.readBytes(m_rxBuf, 3);
        if (ret < 3)
            // check if all expected bytes are received without timeout
            return;

        this->m_currentExtMode = m_rxBuf[0];

        // Get mode and size of the message from the header
        uint8_t msg_size;
        parseHeader(m_rxBuf[2], mode, msg_size);
        // TODO: avoid buffer overflow: check msg size <= size rx buffer

        // Read the remaining bytes after the header (cheksum included)
        // Data will be in the indexes [0;msg_size-2]
        ret = SerialTTL.readBytes(m_rxBuf, msg_size - 1);
        if (_(signed)(ret) != msg_size - 1)
            return;

        switch(mode) {
            case ColorSensor::PBIO_IODEV_MODE_PUP_COLOR_SENSOR__LIGHT:
                this->setLEDBrightnessesMode();
                break;
            default:
                INFO_PRINT(F("unknown W mode: "));
                INFO_PRINTLN(mode, HEX);
                break;
        }
    }
}


/**
 * @brief Send EXT_MODE status to the hub: extended mode info message
 *      Should be used as a first response after a NACK, and before every responses for
 *      modes >= 8.
 */
void ColorSensor::extendedModeInfoResponse(){
    // extended mode info
    m_txBuf[0] = 0x46;                      // header type LUMP_MSG_TYPE_CMD, cmd LUMP_CMD_EXT_MODE, size 3
    m_txBuf[1] = this->m_currentExtMode;    // current EXT_MODE
    sendUARTBuffer(1);
}


/**
 * @brief Mode 3 response (write)
 *      This sensor has 3 built-in lights: 0: left, 1: bottom, 2: right.
 *      Set the brightness of each light.
 */
void ColorSensor::setLEDBrightnessesMode(){
    // Mode 3 (write mode)
    // Expect brightness values (3 int8_t)
    this->m_LEDBrightnesses[0] = m_rxBuf[0];
    this->m_LEDBrightnesses[1] = m_rxBuf[1];
    this->m_LEDBrightnesses[2] = m_rxBuf[2];

    DEBUG_PRINT(F("LEDBrightnesses set (Left,Bottom,Right): "));
    DEBUG_PRINT(this->m_LEDBrightnesses[0], HEX);
    DEBUG_PRINT(F(", "));
    DEBUG_PRINT(this->m_LEDBrightnesses[1], HEX);
    DEBUG_PRINT(F(", "));
    DEBUG_PRINTLN(this->m_LEDBrightnesses[2], HEX);

    if (this->m_pLEDBrightnessesfunc != nullptr)
        this->m_pLEDBrightnessesfunc(this->m_LEDBrightnesses);
}


/**
 * @brief Mode 0 response (read): Send the currently detected color.
 *      Available (official) values:
 *      COLOR_NONE, COLOR_BLACK, COLOR_BLUE, COLOR_LIGHTBLUE, COLOR_GREEN,
 *      COLOR_YELLOW, COLOR_RED, COLOR_PURPLE, COLOR_WHITE.
 *      See m_sensorColor.
 */
void ColorSensor::sensorColorMode(){
    // Mode 0
    m_txBuf[0] = 0xC0;                      // header
    m_txBuf[1] = *m_sensorColor;            // current detected color
    sendUARTBuffer(1);
}


/**
 * @brief Mode 1 response (read): Send reflected light measure.
 *      See m_reflectedLight.
 */
void ColorSensor::sensorReflectedLightMode(){
    // Mode 1
    m_txBuf[0] = 0xC1;                      // header
    m_txBuf[1] = *this->m_reflectedLight;   // 0..100
    sendUARTBuffer(1);
}


/**
 * @brief Mode 2 response (read): Send lux measure.
 *      See m_ambientLight.
 */
void ColorSensor::sensorAmbientLight(){
    // Mode 2
    m_txBuf[0] = 0xC2;                      // header
    m_txBuf[1] = *this->m_ambientLight;
    sendUARTBuffer(1);
}


/**
 * @brief Mode 5 response (read): Send RGB array.
 * @warning The message should be size 6, but for constraints reasons due to masks,
 *      we must stick to a size of 10 bytes.
 */
void ColorSensor::sensorRGB_IMode(){
    // Mode 5
    m_txBuf[0] = getHeader(lump_msg_type_t::LUMP_MSG_TYPE_DATA, 5, 10); // 0xdd
    m_txBuf[1] = this->m_sensorRGB_I[0] & 0xFF;           // Send LSB of red value
    m_txBuf[2] = (this->m_sensorRGB_I[0] >> 8) & 0xFF;    // Send MSB
    m_txBuf[3] = this->m_sensorRGB_I[1] & 0xFF;           // Send LSB of green value
    m_txBuf[4] = (this->m_sensorRGB_I[1] >> 8) & 0xFF;
    m_txBuf[5] = this->m_sensorRGB_I[2] & 0xFF;           // Send LSB of blue value
    m_txBuf[6] = (this->m_sensorRGB_I[2] >> 8) & 0xFF;
    m_txBuf[7] = 0;                                     // Unknown channel
    m_txBuf[8] = 0;                                     // Unknown channel
    sendUARTBuffer(8);
}


/**
 * @brief Mode 6 response (read): Send HSV array.
 *
 */
void ColorSensor::sensorHSVMode(){
    // Mode 6
    // Send data; payload size = 6, but total msg_size = 10
    DEBUG_PRINTLN(F("Mode 6"));

    // Send data
    m_txBuf[0] = getHeader(lump_msg_type_t::LUMP_MSG_TYPE_DATA, 6, 10); // header: 0xde
    m_txBuf[1] = this->m_sensorHSV[0] & 0xFF;           // Send LSB of hue value
    m_txBuf[2] = (this->m_sensorHSV[0] >> 8) & 0xFF;    // Send MSB
    m_txBuf[3] = this->m_sensorHSV[1] & 0xFF;           // Send LSB of saturation value
    m_txBuf[4] = (this->m_sensorHSV[1] >> 8) & 0xFF;
    m_txBuf[5] = this->m_sensorHSV[2] & 0xFF;           // Send LSB of value
    m_txBuf[6] = (this->m_sensorHSV[2] >> 8) & 0xFF;
    m_txBuf[7] = 0;                                     // Padding
    m_txBuf[8] = 0;                                     // Padding
    sendUARTBuffer(8);
}


/**
 * @brief Mode 8 response (read): Debug info
 *
 * @warning This mode exists but its implementation is **UNKNOWN**.
 *      Here is a custom implementation for debugging purposes.
 *      We basically send all responses to all modes.
 */
void ColorSensor::sensorDebugMode(){
    // Mode 8 - Test mode
    // extended mode info
    // We are in EXT_MODE_8 because of the Mode 8 command
    this->extendedModeInfoResponse();

    // Next modes are supposed to be sent with EXT_MODE_0
    this->m_currentExtMode = EXT_MODE_0;

    // Read modes
    this->sensorColorMode();
    this->sensorReflectedLightMode();
    this->sensorAmbientLight();
    this->sensorRGB_IMode();
    this->sensorHSVMode();

    // Write modes
    m_rxBuf[0] = 0xFF; // 255: 100%
    m_rxBuf[0] = 0x80; // 128: 50%
    m_rxBuf[0] = 0x40; // 64: 25%
    this->setLEDBrightnessesMode();
}
