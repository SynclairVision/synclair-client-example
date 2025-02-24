#pragma once

#include <cstdint>
#include <cstring>

// Definiera strukturen för message
struct message {
    uint16_t message_id;
    uint16_t version;
    uint16_t message_type;
    uint16_t param_type;
    uint8_t data[256];
};

constexpr uint16_t VERSION = 1;
constexpr uint16_t EMPTY = 0;
constexpr uint16_t QUIT = 1;

