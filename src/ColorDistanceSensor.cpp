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
#include "ColorDistanceSensor.h"

/**
 * @brief Default constructor
 */
LegoPupColorDistance::LegoPupColorDistance(){
    m_defaultIntVal  = new uint8_t(0);
    uint16_t defaultRGB[3] = {0, 0, 0};

    // Sensor default values
    m_sensorColor    = m_defaultIntVal;
    m_sensorDistance = m_defaultIntVal;
    m_LEDColor       = new uint8_t(0);
    m_reflectedLight = m_defaultIntVal;
    m_ambientLight   = m_defaultIntVal;
    m_sensorRGB      = defaultRGB;
    m_IR_code        = 0;
    m_pIRfunc        = nullptr;
    m_pLEDColorfunc  = nullptr;

    // UART protocol
    m_currentExtMode = 0;
}


/**
 * @brief Constructor allowing to set detected color and distance measure.
 * @overload
 * @param pSensorColor Pointer to a discretized detected color. See m_LEDColor.
 * @param pSensorDistance Pointer to a discreztized distance measured to the
 *      the nearest object. Discretized values 0..10.
 */
LegoPupColorDistance::LegoPupColorDistance(uint8_t *pSensorColor, uint8_t *pSensorDistance){
    m_defaultIntVal  = new uint8_t(0);
    uint16_t defaultRGB[3] = {0, 0, 0};

    // Set given values
    m_sensorColor    = pSensorColor;
    m_sensorDistance = pSensorDistance;
    // Sensor default values
    m_LEDColor       = new uint8_t(0);
    m_reflectedLight = m_defaultIntVal;
    m_ambientLight   = m_defaultIntVal;
    m_sensorRGB      = defaultRGB;
    m_IR_code        = 0;
    m_pIRfunc        = nullptr;
    m_pLEDColorfunc  = nullptr;

    // UART protocol
    m_currentExtMode = 0;
}


/**
 * @brief LegoPupColorDistance::~LegoPupColorDistance
 */
LegoPupColorDistance::~LegoPupColorDistance() {
    delete m_defaultIntVal;
}


/**
 * @brief Setter for m_sensorColor
 * @param pData Pointer to a discretized detected color. See m_LEDColor.
 */
void LegoPupColorDistance::setSensorColor(uint8_t *pData){
    m_sensorColor = pData;
}


/**
 * @brief Setter for m_sensorDistance
 * @param pData Pointer to a discreztized distance measured to the
 *      the nearest object. Discretized values 0..10.
 */
void LegoPupColorDistance::setSensorDistance(uint8_t *pData){
    m_sensorDistance = pData;
}


/**
 * @brief Setter for m_sensorRGB; Raw values of Red Green Blue channels.
 * @param pData Expected value pointed is an array of uint16_t size 3.
 */
void LegoPupColorDistance::setSensorRGB(uint16_t *pData){
    this->m_sensorRGB = pData;
}


/**
 * @brief Getter for m_IR_code
 * @return IR code
 */
uint16_t LegoPupColorDistance::getSensorIRCode(){
    return this->m_IR_code;
}


/**
 * @brief Set callback receiving m_IR_code when modified by the hub.
 */
void LegoPupColorDistance::setIRCallback(void(pfunc)(const uint16_t)){
    this->m_pIRfunc = pfunc;
}


/**
 * @brief Setter for m_LEDColor
 * @param pData: Pointer to the current LED color.
 *      Available values:
 *      COLOR_BLACK, COLOR_BLUE, COLOR_GREEN, COLOR_RED, COLOR_WHITE.
 */
void LegoPupColorDistance::setSensorLEDColor(uint8_t *pData){
    // Free constructor's value
    delete this->m_LEDColor;
    this->m_LEDColor = pData;
}


/**
 * @brief Set callback receiving m_LEDColor when modified by the hub.
 */
void LegoPupColorDistance::setLEDColorCallback(void(pfunc)(const uint8_t)){
    this->m_pLEDColorfunc = pfunc;
}


/**
 * @brief Setter for m_reflectedLight
 * @param pData Pointer to reflected light (from clear channel value or
 *      calculations based on rgb channels). Discretized values 0..5F.
 */
void LegoPupColorDistance::setSensorReflectedLight(uint8_t *pData){
    this->m_reflectedLight = pData;
}


/**
 * @brief Setter for m_ambientLight
 * @param pData Pointer to ambient light based on lux value.
 */
void LegoPupColorDistance::setSensorAmbientLight(uint8_t *pData){
    this->m_ambientLight = pData;
}

/**
 * @brief Send initialization sequences for the current Color & Distance sensor.
 * @see https://github.com/pybricks/pybricks-micropython/lib/pbio/test/src/uartdev.c
 */
void LegoPupColorDistance::commSendInitSequence(){
    // TODO: put all this strings into flash via PROGMEM
    // Initialize uart
    SerialTTL.begin(2400);

    SerialTTL.write("\x40\x25\x9A",3);
    SerialTTL.write("\x51\x07\x07\x0A\x07\xA3",6);
    SerialTTL.write("\x52\x00\xC2\x01\x00\x6E",6); // SPEED 115200
    SerialTTL.write("\x5F\x00\x00\x00\x10\x00\x00\x00\x10\xA0",10);
    SerialTTL.flush();
    delay(10);
    SerialTTL.write("\x9A\x20\x43\x41\x4C\x49\x42\x00\x00\x00\x00",11);
    SerialTTL.write("\x9A\x21\x00\x00\x00\x00\x00\xFF\x7F\x47\x83",11);
    SerialTTL.write("\x9A\x22\x00\x00\x00\x00\x00\x00\xC8\x42\xCD",11);
    SerialTTL.write("\x9A\x23\x00\x00\x00\x00\x00\xFF\x7F\x47\x81",11);
    SerialTTL.write("\x92\x24\x4E\x2F\x41\x00\x69",7);
    SerialTTL.write("\x8A\x25\x10\x00\x40",5);
    SerialTTL.write("\x92\xA0\x08\x01\x05\x00\xC1",7);
    SerialTTL.flush();
    delay(10);
    SerialTTL.write("\x99\x20\x44\x45\x42\x55\x47\x00\x00\x00\x17",11);
    SerialTTL.write("\x99\x21\x00\x00\x00\x00\x00\xC0\x7F\x44\xBC",11);
    SerialTTL.write("\x99\x22\x00\x00\x00\x00\x00\x00\xC8\x42\xCE",11);
    SerialTTL.write("\x99\x23\x00\x00\x00\x00\x00\x00\x20\x41\x24",11);
    SerialTTL.write("\x91\x24\x4E\x2F\x41\x00\x6A",7);
    SerialTTL.write("\x89\x25\x10\x00\x43",5);
    SerialTTL.write("\x91\xA0\x02\x01\x05\x00\xC8",7);
    SerialTTL.flush();
    delay(10);
    SerialTTL.write("\x98\x20\x53\x50\x45\x43\x20\x31\x00\x00\x53",11);
    SerialTTL.write("\x98\x21\x00\x00\x00\x00\x00\x00\x7F\x43\x7A",11);
    SerialTTL.write("\x98\x22\x00\x00\x00\x00\x00\x00\xC8\x42\xCF",11);
    SerialTTL.write("\x98\x23\x00\x00\x00\x00\x00\x00\x7F\x43\x78",11);
    SerialTTL.write("\x90\x24\x4E\x2F\x41\x00\x6B",7);
    SerialTTL.write("\x88\x25\x00\x00\x52",5);
    SerialTTL.write("\x90\xA0\x04\x00\x03\x00\xC8",7);
    SerialTTL.flush();
    delay(10);
    SerialTTL.write("\x9F\x00\x49\x52\x20\x54\x78\x00\x00\x00\x77",11);
    SerialTTL.write("\x9F\x01\x00\x00\x00\x00\x00\xFF\x7F\x47\xA6",11);
    SerialTTL.write("\x9F\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xE8",11);
    SerialTTL.write("\x9F\x03\x00\x00\x00\x00\x00\xFF\x7F\x47\xA4",11);
    SerialTTL.write("\x97\x04\x4E\x2F\x41\x00\x4C",7);
    SerialTTL.write("\x8F\x05\x00\x04\x71",5);
    SerialTTL.write("\x97\x80\x01\x01\x05\x00\xED",7);
    SerialTTL.flush();
    delay(10);
    SerialTTL.write("\x9E\x00\x52\x47\x42\x20\x49\x00\x00\x00\x5F",11);
    SerialTTL.write("\x9E\x01\x00\x00\x00\x00\x00\xC0\x7F\x44\x9B",11);
    SerialTTL.write("\x9E\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xE9",11);
    SerialTTL.write("\x9E\x03\x00\x00\x00\x00\x00\xc0\x7F\x44\x99",11);
    SerialTTL.write("\x96\x04\x52\x41\x57\x00\x29",7);
    SerialTTL.write("\x8E\x05\x10\x00\x64",5);
    SerialTTL.write("\x96\x80\x03\x01\x05\x00\xEE",7);
    SerialTTL.flush();
    delay(10);
    SerialTTL.write("\x9D\x00\x43\x4F\x4C\x20\x4F\x00\x00\x00\x4D",11);
    SerialTTL.write("\x9D\x01\x00\x00\x00\x00\x00\x00\x20\x41\x02",11);
    SerialTTL.write("\x9D\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xEA",11);
    SerialTTL.write("\x9D\x03\x00\x00\x00\x00\x00\x00\x20\x41\x00",11);
    SerialTTL.write("\x95\x04\x49\x44\x58\x00\x3B",7);
    SerialTTL.write("\x8D\x05\x00\x04\x73",5);
    SerialTTL.write("\x95\x80\x01\x00\x03\x00\xE8",7);
    SerialTTL.flush();
    delay(10);
    SerialTTL.write("\x94\x00\x41\x4D\x42\x49\x6C",7);
    SerialTTL.write("\x9C\x01\x00\x00\x00\x00\x00\x00\xC8\x42\xE8",11);
    SerialTTL.write("\x9C\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xEB",11);
    SerialTTL.write("\x9C\x03\x00\x00\x00\x00\x00\x00\xC8\x42\xEA",11);
    SerialTTL.write("\x94\x04\x50\x43\x54\x00\x28",7);
    SerialTTL.write("\x8C\x05\x10\x00\x66",5);
    SerialTTL.write("\x94\x80\x01\x00\x03\x00\xE9",7);
    SerialTTL.flush();
    delay(10);
    SerialTTL.write("\x9B\x00\x52\x45\x46\x4C\x54\x00\x00\x00\x2D",11);
    SerialTTL.write("\x9B\x01\x00\x00\x00\x00\x00\x00\xC8\x42\xEF",11);
    SerialTTL.write("\x9B\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xEC",11);
    SerialTTL.write("\x9B\x03\x00\x00\x00\x00\x00\x00\xC8\x42\xED",11);
    SerialTTL.write("\x93\x04\x50\x43\x54\x00\x2F",7);
    SerialTTL.write("\x8B\x05\x10\x00\x61",5);
    SerialTTL.write("\x93\x80\x01\x00\x03\x00\xEE",7);
    SerialTTL.flush();
    delay(10);
    SerialTTL.write("\x9A\x00\x43\x4F\x55\x4E\x54\x00\x00\x00\x26",11);
    SerialTTL.write("\x9A\x01\x00\x00\x00\x00\x00\x00\xC8\x42\xEE",11);
    SerialTTL.write("\x9A\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xED",11);
    SerialTTL.write("\x9A\x03\x00\x00\x00\x00\x00\x00\xC8\x42\xEC",11);
    SerialTTL.write("\x92\x04\x43\x4E\x54\x00\x30",7);
    SerialTTL.write("\x8A\x05\x08\x00\x78",5);
    SerialTTL.write("\x92\x80\x01\x02\x04\x00\xEA",7);
    SerialTTL.flush();
    delay(10);
    SerialTTL.write("\x91\x00\x50\x52\x4F\x58\x7B",7);
    SerialTTL.write("\x99\x01\x00\x00\x00\x00\x00\x00\x20\x41\x06",11);
    SerialTTL.write("\x99\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xEE",11);
    SerialTTL.write("\x99\x03\x00\x00\x00\x00\x00\x00\x20\x41\x04",11);
    SerialTTL.write("\x91\x04\x44\x49\x53\x00\x34",7);
    SerialTTL.write("\x89\x05\x50\x00\x23",5);
    SerialTTL.write("\x91\x80\x01\x00\x03\x00\xEC",7);
    SerialTTL.flush();
    delay(10);
    SerialTTL.write("\x98\x00\x43\x4F\x4C\x4F\x52\x00\x00\x00\x3A",11);
    SerialTTL.write("\x98\x01\x00\x00\x00\x00\x00\x00\x20\x41\x07",11);
    SerialTTL.write("\x98\x02\x00\x00\x00\x00\x00\x00\xC8\x42\xEF",11);
    SerialTTL.write("\x98\x03\x00\x00\x00\x00\x00\x00\x20\x41\x05",11);
    SerialTTL.write("\x90\x04\x49\x44\x58\x00\x3E",7);
    SerialTTL.write("\x88\x05\xC4\x00\xB6",5);
    SerialTTL.write("\x90\x80\x01\x00\x03\x00\xED",7);
    SerialTTL.write("\x88\x06\x4F\x00\x3E",5);
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
void LegoPupColorDistance::process(){
    if(!this->m_connected){
        connectToHub();
    } else {
        // Connection established
        if (SerialTTL.available() > 0) {
            unsigned char header;
            unsigned char mode;
            header = SerialTTL.read();

            DEBUG_PRINT(F("<\tHeader "));
            DEBUG_PRINTLN(header, HEX);

            if (header == 0x02) { // NACK
                m_lastAckTick = millis();
                // Here we can send mode 0 or mode 8 according to the value of ExtMode
                // And send extendedModeInfoResponse before any data response.
                // Usually we go into mode 8, which automatically sends extendedModeInfoResponse
                // Note: In practice the default mode is always the lowest (0).
                this->m_currentExtMode = EXT_MODE_8;
                this->sensorSpec1Mode();
            } else if (header == 0x43) {
                // "Get value" commands (3 bytes message: header, mode, checksum)
                size_t ret = SerialTTL.readBytes(m_rxBuf, 2);
                if (ret < 2) {
                    // check if all expected bytes are received without timeout
                    DEBUG_PRINT("incomplete 0x43 message");
                    return;
                }
                mode = m_rxBuf[0];
                DEBUG_PRINT(F("<\tAsked mode "));
                DEBUG_PRINTLN(mode);

                this->m_currentExtMode = (mode < 8) ? EXT_MODE_0 : EXT_MODE_8;

                switch (mode) {
                    case LegoPupColorDistance::PBIO_IODEV_MODE_PUP_COLOR_DISTANCE_SENSOR__COLOR:
                        this->LEDColorMode();
                        break;
                    case LegoPupColorDistance::PBIO_IODEV_MODE_PUP_COLOR_DISTANCE_SENSOR__PROX:
                        this->sensorDistanceMode();
                        break;
                    case LegoPupColorDistance::PBIO_IODEV_MODE_PUP_COLOR_DISTANCE_SENSOR__REFLT:
                        this->sensorReflectedLightMode();
                        break;
                    case LegoPupColorDistance::PBIO_IODEV_MODE_PUP_COLOR_DISTANCE_SENSOR__AMBI:
                        this->sensorAmbientLight();
                        break;
                    case LegoPupColorDistance::PBIO_IODEV_MODE_PUP_COLOR_DISTANCE_SENSOR__RGB_I:
                        this->sensorRGBIMode();
                        break;
                    case LegoPupColorDistance::PBIO_IODEV_MODE_PUP_COLOR_DISTANCE_SENSOR__SPEC1:
                        this->sensorSpec1Mode();
                        break;
                    case LegoPupColorDistance::PBIO_IODEV_MODE_PUP_COLOR_DISTANCE_SENSOR__DEBUG:
                        this->sensorDebugMode();
                        break;
                    default:
                        INFO_PRINT("unknown R mode: ");
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

                // Read the remaining bytes after the header (cheksum included)
                // Data will be in the indexes [0;msg_size-2]
                ret = SerialTTL.readBytes(m_rxBuf, msg_size - 1);
                if (ret < msg_size - 1)
                    return;

                switch(mode) {
                    case LegoPupColorDistance::PBIO_IODEV_MODE_PUP_COLOR_DISTANCE_SENSOR__COL_O:
                        this->setLEDColorMode();
                        break;
                    case LegoPupColorDistance::PBIO_IODEV_MODE_PUP_COLOR_DISTANCE_SENSOR__IR_TX:
                        this->setIRTXMode();
                        break;
                    default:
                        INFO_PRINT("unknown W mode: ");
                        INFO_PRINTLN(mode, HEX);
                        break;
                }
            }
        }

        // Check disconnection from the Hub and go in reset/init mode if needed
        if (millis() - m_lastAckTick > 200) {
            INFO_PRINT("Disconnect; Too much time since last NACK - ");
            INFO_PRINTLN(millis() - m_lastAckTick);
            m_connected = false;
        }
    }
}


/**
 * @brief Send EXT_MODE status to the hub: extended mode info message
 *      Should be used as a first response after a NACK, and before every responses for
 *      modes >= 8.
 */
void LegoPupColorDistance::extendedModeInfoResponse(){
    // extended mode info
    m_txBuf[0] = 0x46;                      // header type LUMP_MSG_TYPE_CMD, cmd LUMP_CMD_EXT_MODE, size 3
    m_txBuf[1] = this->m_currentExtMode;    // current EXT_MODE
    sendUARTBuffer(1);
}


/**
 * @brief Mode 5 response (write)
 *      Also call LEDColor callback if defined. See m_pLEDColorfunc.
 */
void LegoPupColorDistance::setLEDColorMode(){
    // Mode 5 (write mode)
    // Expect LED color index (1 int8_t)
    *this->m_LEDColor = m_rxBuf[0];

    DEBUG_PRINT(F("LEDcolor set: "));
    DEBUG_PRINTLN(*m_LEDColor, HEX);

    if (this->m_pLEDColorfunc != nullptr)
        this->m_pLEDColorfunc(*this->m_LEDColor);
}


/**
 * @brief Mode 7 response (write)
 *      Also call IR callback if defined. See m_pIRfunc.
 * @todo
 *      Repeated pulses: count 5 IR codes in x loops: multi callback call
 *      Because we maybe can't block the loop for 5 consecutive transmissions
 *      (a loop can't take more than 200ms).
 *      LEGO protocol needs 5 repetitions, the delay between 2 repetitions is
 *      channel dependent, the length of a message is 16ms (doc), ~11ms in IRremote.
 *      channel delays: 1: 110ms, 2: 148ms, 3: 189ms, 4: 230ms (100 to 200ms in IRremote)
 *
 *      See:
 *      https://github.com/Arduino-IRremote/Arduino-IRremote/blob/e06b594fbefac384d7e1c12aa3e014fca9ee0e6b/src/ir_Lego.hpp#L123
 *      https://web.archive.org/web/20190711083546/http://www.hackvandedam.nl/blog/?page_id=559
 */
void LegoPupColorDistance::setIRTXMode(){
    // Mode 7 (write mode)
    // Expect IR code on (1 int16_t)
    // From Little-Endian (LSB first in the array, then the MSB)
    // Don't do this: prefer explicit endianness handling
    //this->m_IR_code = *((uint16_t *) &m_rxBuf[0]);
    this->m_IR_code = (_(uint16_t) (m_rxBuf[1] << 8)) | m_rxBuf[0];

    DEBUG_PRINT(F("IR data set: "));
    DEBUG_PRINTLN(this->m_IR_code, HEX);

    if (this->m_pIRfunc != nullptr)
        this->m_pIRfunc(this->m_IR_code);
}


/**
 * @brief Mode 0 response (read): Send current LED color.
 */
void LegoPupColorDistance::LEDColorMode(){
    // Mode 0
    m_txBuf[0] = 0xC0;                      // header
    m_txBuf[1] = *m_LEDColor;               // LED current color [0, 3, 5, 9, 0x0A]
    sendUARTBuffer(1);
}


/**
 * @brief Mode 1 response (read): Send distance measure.
 */
void LegoPupColorDistance::sensorDistanceMode(){
    // Mode 1
    m_txBuf[0] = 0xC1;                      // header
    m_txBuf[1] = *m_sensorDistance;         // distance [0..10]
    sendUARTBuffer(1);
}


/**
 * @brief Mode 3 response (read): Send reflected light measure.
 */
void LegoPupColorDistance::sensorReflectedLightMode(){
    // Mode 3
    m_txBuf[0] = 0xC3;                      // header
    m_txBuf[1] = *this->m_reflectedLight;   // 0..5F
    sendUARTBuffer(1);
}


/**
 * @brief Mode 4 response (read): Send lux measure.
 */
void LegoPupColorDistance::sensorAmbientLight(){
    // Mode 4
    m_txBuf[0] = 0xC4;                      // header
    m_txBuf[1] = *this->m_ambientLight;
    sendUARTBuffer(1);
}


/**
 * @brief Mode 6 response (read): Send RGB array.
 * @warning The message should be size 6, but for constraints reasons due to masks,
 *      we must stick to a size of 10 bytes.
 */
void LegoPupColorDistance::sensorRGBIMode() {
    // Mode 6
    // Max observed value is ~440
    // Send data; payload size = 6, but total msg_size = 10
    // TODO: we send: device header: 0xde => type LUMP_MSG_TYPE_DATA mode 6 tot size 10
    // size = 10 !! 8 bytes useful / 10
    m_txBuf[0] = getHeader(lump_msg_type_t::LUMP_MSG_TYPE_DATA, 6, 10); // 0xde
    m_txBuf[1] = this->m_sensorRGB[0] & 0xFF;           // Send LSB
    m_txBuf[2] = (this->m_sensorRGB[0] >> 8) & 0xFF;    // Send MSB
    m_txBuf[3] = this->m_sensorRGB[1] & 0xFF;
    m_txBuf[4] = (this->m_sensorRGB[1] >> 8) & 0xFF;
    m_txBuf[5] = this->m_sensorRGB[2] & 0xFF;
    m_txBuf[6] = (this->m_sensorRGB[2] >> 8) & 0xFF;
    m_txBuf[7] = 0;                                     // Padding
    m_txBuf[8] = 0;                                     // Padding
    sendUARTBuffer(8);
}


/**
 * @brief Mode 8 response (read): Default response after NACK.
 *      Send detected color, distance, current LED color, reflected light data.
 */
void LegoPupColorDistance::sensorSpec1Mode(){
    // Mode 8
    DEBUG_PRINTLN(F("Mode 8"));

    // extended mode info
    this->extendedModeInfoResponse();

    // Send data
    m_txBuf[0] = 0xD0;               // header
    m_txBuf[1] = *m_sensorColor;     // color    [0, 3, 5, 9, 0x0A, 0xFF]
    m_txBuf[2] = *m_sensorDistance;  // distance [0..10]
    m_txBuf[3] = *m_LEDColor;        // LED current color [0, 3, 5, 9, 0x0A]
    m_txBuf[4] = *m_reflectedLight;  // reflected light [0..5F]
    sendUARTBuffer(4);
}


/**
 * @brief Mode 9 response (read): Debug info
 *
 * @warning This mode exists but its implementation is **UNKNOWN**.
 *      Here is a custom implementation for debugging purposes.
 *      We basically send all responses to all modes.
 */
void LegoPupColorDistance::sensorDebugMode(){
    // Mode 9 - Test mode
    // extended mode info
    // We are already in EXT_MODE_8 because of the Mode 9 command
    this->extendedModeInfoResponse();
    this->sensorSpec1Mode();

    // Next modes are supposed to be sent with EXT_MODE_0
    this->m_currentExtMode = EXT_MODE_0;

    // Read modes
    this->LEDColorMode();
    this->sensorDistanceMode();
    this->sensorReflectedLightMode();
    this->sensorAmbientLight();
    this->sensorRGBIMode();

    // Write modes
    m_rxBuf[0] = 0xFF;
    this->setLEDColorMode();

    // 0x4142
    m_rxBuf[0] = 0x42; // LSB
    m_rxBuf[1] = 0x41; // MSB
    this->setIRTXMode();
}
