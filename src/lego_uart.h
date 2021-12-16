// SPDX-License-Identifier: MIT
// Copyright (c) 2019-2020 The Pybricks Authors

// LEGO UART Message Protocol (LUMP) for EV3 and Powered Up I/O devices
//
// This file contains bytecode definitions for interperting messages send to
// and from LEGO MINDSTORMS EV3 and Powered Up UART devices.
//
// References:
// - http://ev3.fantastic.computer/doxygen/UartProtocol.html
// - https://github.com/mindboards/ev3sources/blob/master/lms2012/d_uart/Linuxmod_AM1808/d_uart_mod.c
// - https://github.com/ev3dev/lego-linux-drivers/blob/ev3dev-buster/sensors/ev3_uart_sensor_ld.c
// - https://sourceforge.net/p/lejos/wiki/UART%20Sensor%20Protocol/
// - https://lego.github.io/lego-ble-wireless-protocol-docs/index.html

// Note: This file is a partial copy of
// https://github.com/pybricks/pybricks-micropython/blob/master/lib/lego/lego_uart.h
#ifndef LEGO_UART_H
#define LEGO_UART_H

// Header byte

/** Bit mask for ::lump_msg_type_t */
#define LUMP_MSG_TYPE_MASK 0xC0

/**
 * Message type.
 */
typedef enum {
    /**
     * System message type.
     *
     * These messages don't have a payload or a checksum, so only consist of the
     * single header byte.
     *
     * The ::LUMP_MSG_SIZE_MASK bits should be set to ::LUMP_MSG_SIZE_1.
     *
     * The ::LUMP_MSG_CMD_MASK bits must be one of ::lump_sys_t.
     */
    LUMP_MSG_TYPE_SYS = 0 << 6,

    /**
     * Command message type.
     *
     * The ::LUMP_MSG_SIZE_MASK bits must be set to the size of the
     * payload.
     *
     * The ::LUMP_MSG_CMD_MASK bits must be one of ::lump_cmd_t.
     */
    LUMP_MSG_TYPE_CMD = 1 << 6,

    /**
     * Info message type.
     *
     * The ::LUMP_MSG_SIZE_MASK bits must be set to the size of the
     * payload.
     *
     * The ::LUMP_MSG_CMD_MASK bits must be set to the mode index number.
     *
     * The header byte will be followed by a ::lump_info_t byte.
     */
    LUMP_MSG_TYPE_INFO = 2 << 6,

    /**
     * Data message type.
     *
     * The ::LUMP_MSG_SIZE_MASK bits must be set to the size of the
     * payload.
     *
     * The ::LUMP_MSG_CMD_MASK bits must be set to the mode index number.
     */
    LUMP_MSG_TYPE_DATA = 3 << 6,
} lump_msg_type_t;

/** Bit mask for ::lump_msg_size_t. */
#define LUMP_MSG_SIZE_MASK 0x38

/**
 * Macro to convert ::lump_msg_size_t to the size in bytes.
 *
 * @param [in]  s   ::lump_msg_size_t.
 * @return          The size in bytes.
 */
#define LUMP_MSG_SIZE(s) (1 << (((s) >> 3) & 0x7))

/**
 * Encoded message payload size.
 */
typedef enum {
    /** Payload is 1 byte. */
    LUMP_MSG_SIZE_1 = 0 << 3,

    /** Payload is 2 bytes. */
    LUMP_MSG_SIZE_2 = 1 << 3,

    /** Payload is 4 byte. */
    LUMP_MSG_SIZE_4 = 2 << 3,

    /** Payload is 8 byte. */
    LUMP_MSG_SIZE_8 = 3 << 3,

    /** Payload is 16 byte. */
    LUMP_MSG_SIZE_16 = 4 << 3,

    /** Payload is 32 byte. */
    LUMP_MSG_SIZE_32 = 5 << 3,
} lump_msg_size_t;



/**
 * The message command or mode number mask.
 *
 * The meaning of the header value in this position depends on the
 * ::lump_msg_type_t of the header.
 */
#define LUMP_MSG_CMD_MASK 0x07




/**
 * Command types.
 *
 * This value is encoded at ::LUMP_MSG_CMD_MASK when ::lump_msg_type_t is
 * ::LUMP_MSG_TYPE_CMD.
 */
typedef enum {
    /**
     * Write command.
     *
     * This message is sent in either directino after syncronization to select
     * the mode.
     *
     * The payload size and data depend on the ::lump_type_id_t of the I/O
     * device.
     *
     * Known uses of this command included resetting the angle on
     * ::LUMP_TYPE_ID_EV3_GYRO_SENSOR and selecting mode combinations on
     * Powered Up devices.
     */
    LUMP_CMD_WRITE = 0x4,


} lump_cmd_t;

#endif // LEGO_UART_H
