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
    uint8_t *default_val = new uint8_t(0);
    uint16_t defaultRGB[3] = {0, 0, 0};

    // Sensor default values
    m_sensorColor = default_val;
    m_sensorDistance = default_val;
    m_LEDColor = new uint8_t(0);
    m_reflectedLight = default_val;
    m_ambientLight = default_val;
    m_sensorRGB = defaultRGB;
    m_IR_code = 0;
    m_pIRfunc = nullptr;
    m_pLEDColorfunc = nullptr;

    // UART protocol
    m_connSerialRX_pin = 0;
    m_connSerialTX_pin = 1;

    m_currentExtMode = 0;
    m_lastAckTick = 0;
    m_connected = false;
}

/**
 * @brief Constructor allowing to set detected color and distance measure.
 * @param pSensorColor Pointer to a discretized detected color. See m_LEDColor.
 * @param pSensorDistance Pointer to a discreztized distance measured to the
 *      the nearest object. Discretized values 0..10.
 */
LegoPupColorDistance::LegoPupColorDistance(uint8_t *pSensorColor, uint8_t *pSensorDistance){
    uint8_t *default_val = new uint8_t(0);
    uint16_t defaultRGB[3] = {0, 0, 0};

    // Set given values
    m_sensorColor = pSensorColor;
    m_sensorDistance = pSensorDistance;
    // Sensor default values
    m_LEDColor = new uint8_t(0);
    m_reflectedLight = default_val;
    m_ambientLight = default_val;
    m_sensorRGB = defaultRGB;
    m_IR_code = 0;
    m_pIRfunc = nullptr;
    m_pLEDColorfunc = nullptr;

    // UART protocol
    m_connSerialRX_pin = 0;
    m_connSerialTX_pin = 1;

    m_currentExtMode = 0;
    m_lastAckTick = 0;
    m_connected = false;
}

/**
 * @brief Setter for m_sensorColor
 * @param pData Pointer to a discretized detected color. See m_LEDColor.
 */
void LegoPupColorDistance::setSensorColor(uint8_t* pData){
    m_sensorColor = pData;
}

/**
 * @brief Setter for m_sensorDistance
 * @param pData Pointer to a discreztized distance measured to the
 *      the nearest object. Discretized values 0..10.
 */
void LegoPupColorDistance::setSensorDistance(uint8_t* pData){
    m_sensorDistance = pData;
}

/**
 * @brief Setter for m_sensorRGB; Raw values of Red Green Blue channels.
 * @param pData Expected value pointed is an array of uint16_t size 3.
 */
void LegoPupColorDistance::setSensorRGB(uint16_t* pData) {
    this->m_sensorRGB = pData;
}

/**
 * @brief Getter for m_IR_code
 * @return IR code
 */
uint16_t LegoPupColorDistance::getSensorIRCode() {
    return this->m_IR_code;
}

/**
 * @brief Set callback receiving m_IR_code when modified by the hub.
 */
void LegoPupColorDistance::setIRCallback(void (pfunc)(const uint16_t)) {
    this->m_pIRfunc = pfunc;
}

/**
 * @brief Setter for m_LEDColor
 * @param pData: Pointer to the current LED color.
 *      Available values:
 *      COLOR_BLACK, COLOR_BLUE, COLOR_GREEN, COLOR_RED, COLOR_WHITE.
 */
void LegoPupColorDistance::setSensorLEDColor(uint8_t* pData) {
    // Free constructor's value
    delete this->m_LEDColor;
    this->m_LEDColor = pData;
}

/**
 * @brief Set callback receiving m_LEDColor when modified by the hub.
 */
void LegoPupColorDistance::setLEDColorCallback(void (pfunc)(const uint8_t)) {
    this->m_pLEDColorfunc = pfunc;
}

/**
 * @brief Setter for m_reflectedLight
 * @param pData Pointer to reflected light (from clear channel value or
 *      calculations based on rgb channels). Discretized values 0..5F.
 */
void LegoPupColorDistance::setSensorReflectedLight(uint8_t* pData) {
    this->m_reflectedLight = pData;
}

/**
 * @brief Setter for m_ambientLight
 * @param pData Pointer to ambient light based on lux value.
 */
void LegoPupColorDistance::setSensorAmbientLight(uint8_t* pData) {
    this->m_ambientLight = pData;
}

/**
 * @brief Get status of connection with the hub.
 * @return bool
 */
bool LegoPupColorDistance::isConnected(void){
    return m_connected;
}

/**
 * @brief Get checksum for the given message
 * @param pData Message array: Header + Payload
 * @param length Length of the message
 * @return Checksum byte
 */
uint8_t LegoPupColorDistance::calcChecksum(uint8_t* pData, int length){
    uint8_t lRet, i;
    lRet = 0xFF;
    for(i=0; i<length; i++){
        lRet ^= pData[i];
    }
    return lRet;
}

/**
 * @brief Wait until the hub is available on the serial RX line.
 *      Then, assert the TX line.
 */
void LegoPupColorDistance::commWaitForHubIdle(void){
    // Disable uart: manual control TX and RX pins
    SerialTTL.end();

    unsigned long idletick;

    pinMode(m_connSerialTX_pin, OUTPUT);
    digitalWrite(m_connSerialTX_pin, LOW);

    pinMode(m_connSerialRX_pin, INPUT);
    idletick = millis();
    while(1){
        if(digitalRead(m_connSerialRX_pin) == LOW){
            idletick = millis();
        }
        if(millis()-idletick > 100){
            break;
        }
    }

    digitalWrite(m_connSerialTX_pin, HIGH);
    delay(100);
    digitalWrite(m_connSerialTX_pin, LOW);
    delay(100);
}

/**
 * @brief Send initialization sequences for the current Color & Distance sensor.
 * @see https://github.com/pybricks/pybricks-micropython/lib/pbio/test/src/uartdev.c
 */
void LegoPupColorDistance::commSendInitSequence(void){
    // TODO: put all this strings into flash via PROGMEM
    // Initialize uart
    SerialTTL.begin(115200);

    SerialTTL.write("\x00",1);
    SerialTTL.flush();
    delay(10);
    SerialTTL.write("\x40\x25\x9A",3);
    SerialTTL.write("\x51\x07\x07\x0A\x07\xA3",6);
    SerialTTL.write("\x52\x00\xC2\x01\x00\x6E",6);
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
 */
void LegoPupColorDistance::Process(void){

    if(!this->m_connected){
        #ifdef DbgSerial
        DbgSerial.println("INIT SENSOR");
        #endif

        // Wait for HUB to idle it's TX pin (idle = High)
        // TODO: ces bidouilles émettent b'\x00\x00' avant tout choses sur la ligne série !!
        commWaitForHubIdle();

        // Starting initialization sequence
        //unsigned long starSequence = millis();
        commSendInitSequence();
        unsigned long starttime = millis();
        //DbgSerial.println(starttime-starSequence); // time requested for initialization sequence

        // Check if the hub send a ACK
        unsigned long currenttime = starttime;
        while((currenttime-starttime) < 2000){
            if (SerialTTL.available() > 0) {
                // read the incoming byte
                unsigned char dat = SerialTTL.read();
                if (dat == 0x04){ // ACK
                    //DbgSerial.println("Connection Espablished !");
                    SerialTTL.begin(115200);
                    m_connected = true;
                    m_lastAckTick = millis();
                    break;
                }
            }
            currenttime = millis();
            delay(10);
        }
    }
    else{
        // Connection established
        if (SerialTTL.available() > 0) {
            unsigned char header;
            unsigned char mode;
            header = SerialTTL.read();

            #ifdef DbgSerial
            DbgSerial.print(F("<\tHeader "));
            DbgSerial.println(header, HEX);
            #endif

            if(header == 0x02){ // NACK
                m_lastAckTick = millis();
                // Here we can send mode 0 or mode 8 according to the value of ExtMode
                // And send extendedModeInfoResponse before any data response.
                // Usually we go into mode 8, which automatically sends extendedModeInfoResponse
                // Note: In practice the default mode is always the lowest (0).
                this->m_currentExtMode = EXT_MODE_8;
                this->sensorSpec1Mode();
                delay(50);
            }
            else if (header == 0x43) {
                // Get values commands (3 bytes message)
                mode = SerialTTL.read();

                #ifdef DbgSerial
                DbgSerial.print(F("<\tAsked mode "));
                DbgSerial.println(mode);
                #endif

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
                        break;
                }
                // Discard the last byte of data (checksum)
                SerialTTL.read();
            }
            else if(header == 0x46){
                // Set values commands
                // The message has 2 parts (each with header and checksum):
                // - The EXT_MODE status directly given
                // - The LUMP_MSG_TYPE_DATA itself with its data
                this->m_currentExtMode = SerialTTL.read();

                // Discard the last byte of data (checksum) and get the header of the next message
                size_t ret = SerialTTL.readBytes(m_rxBuf, 2);
                if (!ret)
                    return;

                // Get mode and size of the message from the header
                uint8_t msg_size;
                parseHeader(m_rxBuf[1], mode, msg_size);

                // Read the remaining bytes after the header
                // Data will be in the indexes [0;msg_size-2]
                ret = SerialTTL.readBytes(m_rxBuf, msg_size - 1);
                if (!ret)
                    return;

                switch(mode) {
                    case LegoPupColorDistance::PBIO_IODEV_MODE_PUP_COLOR_DISTANCE_SENSOR__COL_O:
                        this->setLEDColorMode();
                        break;
                    case LegoPupColorDistance::PBIO_IODEV_MODE_PUP_COLOR_DISTANCE_SENSOR__IR_TX:
                        this->setIRTXMode();
                        break;
                    default:
                        break;
                }
            }
        }

        // TODO: possible that the processing of responses takes longer than 200ms...
        // it could end to a systematic disconnection here...

        // Check disconnection from the Hub and go in reset/init mode if needed
        if(millis()- m_lastAckTick > 200){
            // DbgSerial.println("Disconnect; Too much time since last NACK");
            m_connected = false;
        }
    }
}

/**
 * @brief Get header from the given message type, mode and size
 * @param msg_type Basically lump_msg_type_t::LUMP_MSG_TYPE_DATA for emitted messages.
 * @param mode Mode number.
 * @param msg_size Size of the message WITHOUT header & checksum!
 *
 * @warning: msg_size is multiplied by 3 in order the given params fits in authorized headers
 *      (There are restrictions due to the masks used)
 * @return
 */
uint8_t LegoPupColorDistance::getHeader(const lump_msg_type_t &msg_type, const uint8_t &mode, const uint8_t &msg_size) {
    return (msg_type & LUMP_MSG_TYPE_MASK) | (mode & LUMP_MSG_CMD_MASK) | ((msg_size*3) & LUMP_MSG_SIZE_MASK);
}

/**
 * @brief Get mode and message size from the given header.
 *      Currently used to parse 2nd part of write queries (1st part has the known header 0x46).
 * @param header Header of a received/transmitted message.
 * @param mode Reference See LegoPupColorDistance class enumeration of mode.
 * @param msg_size Reference to message size.
 */
void LegoPupColorDistance::parseHeader(const uint8_t &header, uint8_t &mode, uint8_t &msg_size) {
    // Type is known to be LUMP_MSG_TYPE_DATA because of 0x46 header
    // msg_type = header & LUMP_MSG_TYPE_MASK;
    mode = header & LUMP_MSG_CMD_MASK;
    msg_size = getMsgSize(header);
}

/**
 * @brief Get size of a message from the given header. Used by parseHeader().
 *
 * @warning /!\ This is a simplified version that works on messages of type LUMP_MSG_TYPE_DATA.
 *      DO NOT use on LUMP_MSG_TYPE_SYS, LUMP_MSG_TYPE_INFO messages.
 * @param header
 * @return Expected size
 */
uint8_t LegoPupColorDistance::getMsgSize(const uint8_t &header) {
    // Simplified version that implicitly asserts that msg_type is LUMP_MSG_TYPE_DATA
    return _(uint8_t) ((1 << (((header) >> 3) & 0x7)) + 2);
}

/**
 * @brief Send the TX buffer content to the hub
 *      Also add the checksum of the message.
 * @param msg_size Size of the message WITHOUT header & checksum.
 */
void LegoPupColorDistance::sendUARTBuffer(uint8_t msg_size) {
    // Add checksum to the last index
    m_txBuf[msg_size + 1] = calcChecksum(this->m_txBuf, msg_size + 1);
    // Send data
    SerialTTL.write((char*)this->m_txBuf, msg_size + 2);
    SerialTTL.flush();
}

/**
 * @brief Send EXT_MODE status to the hub: extended mode info message
 *      Should be used as a first response after a NACK, and before every responses for
 *      modes >= 8.
 */
void LegoPupColorDistance::extendedModeInfoResponse() {
    // extended mode info
    m_txBuf[0] = 0x46;                      // header type LUMP_MSG_TYPE_CMD, cmd LUMP_CMD_EXT_MODE, size 3
    m_txBuf[1] = this->m_currentExtMode;    // current EXT_MODE
    sendUARTBuffer(1);
}

/**
 * @brief Mode 5 response (write)
 *      Also call LEDColor callback if defined. See m_pLEDColorfunc.
 */
void LegoPupColorDistance::setLEDColorMode() {
    // Mode 5 (write mode)
    // Expect LED color index (1 int8_t)
    *this->m_LEDColor = m_rxBuf[0];

    #ifdef DbgSerial
    DbgSerial.print(F("LEDcolor set: "));
    DbgSerial.println(*m_LEDColor, HEX);
    #endif

    if (this->m_pLEDColorfunc != nullptr)
        this->m_pLEDColorfunc(*this->m_LEDColor);
}

/**
 * @brief Mode 7 response (write)
 *      Also call IR callback if defined. See m_pIRfunc.
 */
void LegoPupColorDistance::setIRTXMode() {
    // Mode 7 (write mode)
    // Expect IR code on (1 int16_t)
    // From Little-Endian (LSB first in the array, then the MSB)
    //this->m_IR_code = *((uint16_t *) &m_rxBuf[0]);
    this->m_IR_code = (_(uint16_t) (m_rxBuf[1] << 8)) | m_rxBuf[0];

    #ifdef DbgSerial
    DbgSerial.print(F("IR data set: "));
    DbgSerial.println(this->m_IR_code, HEX);
    #endif

    if (this->m_pIRfunc != nullptr)
        this->m_pIRfunc(this->m_IR_code);
}

/**
 * @brief Mode 0 response (read): Send current LED color.
 */
void LegoPupColorDistance::LEDColorMode() {
    // Mode 0
    m_txBuf[0] = 0xC0;                      // header
    m_txBuf[1] = *m_LEDColor;               // LED current color [0, 3, 5, 9, 0x0A]
    sendUARTBuffer(1);
}

/**
 * @brief Mode 1 response (read): Send distance measure.
 */
void LegoPupColorDistance::sensorDistanceMode() {
    // Mode 1
    m_txBuf[0] = 0xC1;                      // header
    m_txBuf[1] = *m_sensorDistance;         // distance [0..10]
    sendUARTBuffer(1);
}

/**
 * @brief Mode 3 response (read): Send reflected light measure.
 */
void LegoPupColorDistance::sensorReflectedLightMode() {
    // Mode 3
    m_txBuf[0] = 0xC3;                      // header
    m_txBuf[1] = *this->m_reflectedLight;   // 0..5F
    sendUARTBuffer(1);
}

/**
 * @brief Mode 4 response (read): Send lux measure.
 */
void LegoPupColorDistance::sensorAmbientLight() {
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
void LegoPupColorDistance::sensorSpec1Mode() {
    // Mode 8
    #ifdef DbgSerial
    DbgSerial.println(F("Mode 8"));
    #endif

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
void LegoPupColorDistance::sensorDebugMode() {
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
/*10200 octets (35%) de l'espace de stockage de programmes. Le maximum est de 28672 octets.
Les variables globales utilisent 1571 octets (61%)

*/
