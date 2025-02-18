#ifndef NETWORK_INTERFACE_HPP
#define NETWORK_INTERFACE_HPP

#include <string>
#include "message-definitions/msg_defs.hpp"

bool try_connect(std::string ip);
void close_connection();
void send_message(const message &msg);
message read_message();

#endif // NETWORK_INTERFACE_HPP
