/*
 * MyOwnBricks is a library for the emulation of PoweredUp sensors on microcontrollers
 * Copyright (C) 2021 Ysard - <ysard@users.noreply.github.com>
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

/**
 * @brief Discretize colors and set sensorColor variable
 *    Available colors: COLOR_NONE, COLOR_BLACK, COLOR_BLUE,
 *    COLOR_GREEN, COLOR_RED, COLOR_WHITE.
 *
 *    Generally speaking, stable measuring conditions are required, i.e.,
 *    a stable measuring distance not exceeding 4 cm, no interfering light
 *    reaching the side of the sensor. Think about matt black sensor shroud.
 *
 *    Metrics:
 *      - BASIC_RGB: Simple comparison between channels;
 *          Very fast but is likely to produce errors.
 *      - MANHATTAN: Sum of absolute values of distances.
 *          Quite heavy, but quite accurate if the reference values have been
 *          measured seriously and if the measurement environment is controlled
 *          (reproducible). Distance between the sensor and the object should be
 *          the same as during learning.
 *          https://fr.wikipedia.org/wiki/Distance_de_Manhattan
 *     - CANBERRA: A weighted version of Manhattan distance;
 *          Very heavy but brings a higher accuracy and more tolerance/stability to variations
 *          in the measurement environment.
 *          Note: The manipulation of decimal numbers should be avoided
 *          on microcontrollers... Does it worth it? Probably not.
 *          https://en.wikipedia.org/wiki/Canberra_distance
 */
//#define BASIC_RGB
//#define MANHATTAN
//#define CANBERRA
extern uint8_t  sensorColor;
extern uint16_t red, green, blue, clear, lux;


#ifdef BASIC_RGB
void detectColor() {
    if ((red > green) && (red > blue)) {
        sensorColor = COLOR_RED;
    } else if ((green > red) && (green > blue)) {
        sensorColor = COLOR_GREEN;
    } else if ((blue > red) && (blue > green)) {
        sensorColor = COLOR_BLUE;
    }
}
#endif


#if (defined(MANHATTAN) || defined(CANBERRA))
const uint16_t SAMPLES[][3] = {
    { 297,  83,  56 }, // RED_1
    {  43,  20,  17 }, // RED_3
    {  35, 142, 193 }, // BLUE_1
    {  35,  94, 116 }, // BLUE_3
    {  86, 257, 257 }, // CYAN_1
    {  36,  98,  97 }, // CYAN_3
    { 120, 141,  46 }, // YELLOW_1
    {  72,  73,  30 }, // YELLOW_3
    { 338, 373, 120 }, // YELLOW_PLQ_1
    { 159, 267, 201 }, // WHITE_1
    {  87, 126, 102 }, // WHITE_3
    {  89, 322, 163 }, // GREEN_1
    {  58, 106,  68 }, // GREEN_3
    { 103, 189,  57 }, // GREEN_LIGHT_1
    {  51,  77,  33 }, // GREEN_LIGHT_3
    {  26,  34,  28 }  // BLACK_1
};

const uint8_t SAMPLES_MAP[] = {
    COLOR_RED,    COLOR_RED,
    COLOR_BLUE,   COLOR_BLUE,
    COLOR_BLUE,   COLOR_BLUE,
    COLOR_YELLOW, COLOR_YELLOW,COLOR_YELLOW,
    COLOR_WHITE,  COLOR_WHITE,
    COLOR_GREEN,  COLOR_GREEN,
    COLOR_GREEN,  COLOR_GREEN,
    COLOR_BLACK
};

// Number of samples
const uint8_t samplesCount = sizeof(SAMPLES) / sizeof(SAMPLES[0]);

void detectColor() {
#ifdef MANHATTAN
    uint16_t minDist = 10000;
    uint16_t expDist;
#else
    float minDist = 3;
    float expDist;
#endif
    uint8_t bestSampleIndex = 0;

    for (uint8_t i = 0; i < samplesCount; i++) {
#ifdef MANHATTAN
        expDist = abs(static_cast<int16_t>(red - SAMPLES[i][0]))
                  + abs(static_cast<int16_t>(green - SAMPLES[i][1]))
                  + abs(static_cast<int16_t>(blue - SAMPLES[i][2]));
#else
        // Yeah it's ugly but abs() of Arduino is a macro different from the stl implementation
        // moreover the parameter must be explicitly signed.
        // The numerator or denominator must be a float.
        // https://www.best-microcontroller-projects.com/arduino-absolute-value.html
        // https://github.com/arduino/reference-en/issues/362
        expDist = (abs(static_cast<int16_t>(red - SAMPLES[i][0])) / static_cast<float>(red + SAMPLES[i][0]))
                  + (abs(static_cast<int16_t>(green - SAMPLES[i][1])) / static_cast<float>(green + SAMPLES[i][1]))
                  + (abs(static_cast<int16_t>(blue - SAMPLES[i][2])) / static_cast<float>(blue + SAMPLES[i][2]));
#endif
        if (expDist < minDist) {
            bestSampleIndex = i;
            minDist         = expDist;
        }
    }
    DEBUG_PRINTLN(expDist);

    // Arbitrary threshold to avoid erroneous
#ifdef MANHATTAN
    if (minDist > 100) {
#else
    if (minDist > 1.9) { // Red color is quite difficult to identify even with this high threashold
#endif
        // Matching is not acceptable
        sensorColor = COLOR_NONE;
        return;
    }
    // Get color value expected by the hub
    sensorColor = SAMPLES_MAP[bestSampleIndex];
}


#endif
