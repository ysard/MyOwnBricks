/*
 * MyOwnBricks is a library for the emulation of PoweredUp sensors on microcontrollers
 * Copyright (C) 2021-2022 Ysard - <ysard@users.noreply.github.com>
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
extern uint8_t       reflectedLight;
extern uint8_t       ambientLight;
extern uint16_t      sensorRGB[3];
extern TCS34725      rgb_sensor;
extern volatile bool rgbSensorReady;
extern uint8_t getPercentage(const uint16_t, const float&, const float&);

#define LUX_TO_PERCENTAGE(val)                (getPercentage(val, 0.0105, -0.0843))
#define REFLECTED_LIGHT_TO_PERCENTAGE(val)    (getPercentage(val, 0.0017, -8))


/**
 * @brief Process raw values from sensor and convert them for the PoweredUp hub,
 *      if needed.
 */
void handleRGBSensorData() {
    if (!rgbSensorReady)
        return;

    // Data measurement
    boolean status = rgb_sensor.updateData(true);

    if (status) {
        // Ambient light (lux) computation
        rgb_sensor.updateLux();

        int16_t lux = lround(rgb_sensor.lux);

        // Sometimes lux values are below 0; this coincides with erroneous data
        // Moreover, we discard data taken below 40 lux.
        if ((lux >= 40) && (static_cast<uint16_t>(lux) <= rgb_sensor.maxlux)) {
            // Set ambient light (lux) - map 0-100
            //ambientLight = map(rgb_sensor.lux, 0, rgb_sensor.maxlux, 0, 100);
            ambientLight = LUX_TO_PERCENTAGE(lux); // cast ?

            // RGBC Channels are usable
            // Map values to max ~440;
            // Continuous values from 0-65535 (16bits) to 0-1023 (10bits)
            // Note: 440 gives ~28000 (which is the quasi maximum value observed in the channels)
            red   = rgb_sensor.r_comp >> 6,
            green = rgb_sensor.g_comp >> 6,
            blue  = rgb_sensor.b_comp >> 6,

            // Set clear channel as reflected light - map 0-100
            reflectedLight = REFLECTED_LIGHT_TO_PERCENTAGE(rgb_sensor.c_comp);

            // Set RGB channels
            sensorRGB[0] = red;
            sensorRGB[1] = green;
            sensorRGB[2] = blue;

            // Set detected color
            detectColor();
        } else {
            sensorColor = COLOR_NONE;
        }
#if (defined(INFO) || defined(DEBUG))
        clear = rgb_sensor.c_comp >> 6;
        /*
        Serial.print("Lux: "); Serial.print(rgb_sensor.lux, DEC);
        Serial.print("; max: "); Serial.print(rgb_sensor.maxlux);
        Serial.print("; R: "); Serial.print(red, DEC);
        Serial.print("; G: "); Serial.print(green, DEC);
        Serial.print("; B: "); Serial.print(blue, DEC);
        Serial.print("; C: "); Serial.println(clear, DEC);
        */

        Serial.print(rgb_sensor.lux, DEC); Serial.print(";");
        Serial.print(rgb_sensor.maxlux); Serial.print(";");
        Serial.print(red, DEC); Serial.print(";");
        Serial.print(green, DEC); Serial.print(";");
        Serial.print(blue, DEC); Serial.print(";");
        Serial.println(clear, DEC);
#endif
    } else {
        sensorColor = COLOR_NONE;
        INFO_PRINTLN(F("not valid data! wait next measure"));
    }
    // Interrupt tear down
    rgb_sensor.tcs.clearInterrupt();
    rgbSensorReady = false;
    PCIFR &= ~(1 << PCIF0); // clear PC interrupt flag in case of bounce
}
