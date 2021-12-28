
COLOR_MAP = {
    0xFF: "NONE",
    0: "BLACK",
    1: "PINK",
    2: "PURPLE",
    3: "BLUE",
    4: "LIGHTBLUE",
    5: "CYAN",
    6: "GREEN",
    7: "YELLOW",
    8: "ORANGE",
    9: "RED",
    10: "WHITE",
}

# Time interval during which the loop is blocked waiting for the reception of a packet.
# The smaller this interval, the greater the number of requests that can be sent.
# The interval should not exceed 200ms otherwise the device will disconnect.
LOOP_TIMEOUT = 0.1
