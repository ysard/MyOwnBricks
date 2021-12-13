/*
 * MyOwnBricks is a library for the emulation of PoweredUp sensors on microcontrollers
 * Copyright (C) 2021 Ysard - <ysard@users.noreply.github.com>
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
 *          Very heavy but brings a high accuracy and more tolerance to variations 
 *          in the measurement environment.
 *          Note: The manipulation of decimal numbers should be avoided 
 *          on microcontrollers...
 *          https://en.wikipedia.org/wiki/Canberra_distance
 */
//#define BASIC_RGB
//#define MANHATTAN
//#define CANBERRA
extern uint8_t sensorColor;
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
  {19044, 5329, 3600}, // RED_1
  {2778, 1343, 1103}, // RED_3
  {2270, 9116, 12358}, // BLUE_1
  {2268, 6069, 7432}, // BLUE_3
  {5564, 16485, 16511}, // CYAN_1
  {2352, 6315, 6263}, // CYAN_3
  {7736, 9071, 2948}, // YELLOW_1
  {4665, 4722, 1935}, // YELLOW_3
  {21646, 23873, 7706}, // YELLOW_PLQ_1
  {10227, 17135, 12897}, // WHITE_1
  {5625, 8099, 6578}, // WHITE_3
  {5703, 20655, 10476}, // GREEN_1
  {3763, 6829, 4375}, // GREEN_3
  {6645, 12134, 3710}, // GREEN_LIGHT_1
  {3323, 4978, 2145}, // GREEN_LIGHT_3
  {1688, 2228, 1806}, // BLACK_1
};

const uint8_t SAMPLES_MAP[] = {
  COLOR_RED, COLOR_RED, 
  COLOR_BLUE, COLOR_BLUE, 
  COLOR_BLUE, COLOR_BLUE,
  COLOR_YELLOW, COLOR_YELLOW, COLOR_YELLOW,
  COLOR_WHITE, COLOR_WHITE,
  COLOR_GREEN, COLOR_GREEN,
  COLOR_GREEN, COLOR_GREEN,
  COLOR_BLACK
};

// Number of samples
const byte samplesCount = sizeof(SAMPLES) / sizeof(SAMPLES[0]);

void detectColor() {

  uint16_t minDist = 10000;
  uint16_t expDist;
  uint8_t bestSampleIndex;

  for (uint8_t i = 0; i < samplesCount; i++) {
    #ifdef MANHATTAN
    expDist = abs(red - SAMPLES[i][0]) + abs(green - SAMPLES[i][1]) + abs(blue - SAMPLES[i][2]);
    #else
    expDist = (abs(red - SAMPLES[i][0]) / (red + SAMPLES[i][0])) 
      + (abs(green - SAMPLES[i][1]) / (green + SAMPLES[i][1])) 
      + (abs(blue - SAMPLES[i][2]) / (blue + SAMPLES[i][2]));
    #endif
    if (expDist < minDist) {
      bestSampleIndex = i;
      minDist = expDist;
    }
  }

  if (minDist > 6000) {
    // Matching is not acceptable
    sensorColor = COLOR_NONE;
    return;
  }
  // Get color value expected by the hub
  sensorColor = SAMPLES_MAP[bestSampleIndex];
}
#endif
