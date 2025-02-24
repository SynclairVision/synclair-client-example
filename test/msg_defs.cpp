#include "msg_defs.hpp"

void pack_get_parameters(message &msg, MessageType type, uint16_t cam_id) {
    msg.message_type = type;
    msg.param_type = cam_id;
    // Lägg till ytterligare logik om nödvändigt
}
