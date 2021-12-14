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
#ifndef GLOBAL_H
#define GLOBAL_H

#if !defined(ARDUINO_AVR_PROMICRO) && !defined(ARDUINO_AVR_MICRO)
#include <cinttypes>
#endif

#if defined(ARDUINO_AVR_PROMICRO)
#define SerialTTL    Serial1
#define DbgSerial    Serial
#else
#define SerialTTL    Serial
#endif

/**
  * Static cast syntatic sugar
  */
#define _(type)    static_cast<type>


//#define DEBUG // Activate this for debug output on Serial CDC interface
//#define INFO

/**
 * Debug directives
 */
#if (defined(DEBUG) && defined(DbgSerial))
    #define DEBUG_PRINT(...)    DbgSerial.print(__VA_ARGS__)
    #define DEBUG_PRINTLN(...)  DbgSerial.println(__VA_ARGS__)
#else
    /**
     * If DEBUG, print the arguments, otherwise do nothing.
     */
    #define DEBUG_PRINT(...) void()
    /**
     * If DEBUG, print the arguments as a line, otherwise do nothing.
     */
    #define DEBUG_PRINTLN(...) void()
#endif

/**
 * Info directives
 */
#if (defined(INFO) && defined(DbgSerial))
    #define INFO_PRINT(...)    DbgSerial.print(__VA_ARGS__)
    #define INFO_PRINTLN(...)  DbgSerial.println(__VA_ARGS__)
#else
    /**
     * If INFO, print the arguments, otherwise do nothing.
     */
    #define INFO_PRINT(...) void()
    /**
     * If INFO, print the arguments as a line, otherwise do nothing.
     */
    #define INFO_PRINTLN(...) void()
#endif

#endif
