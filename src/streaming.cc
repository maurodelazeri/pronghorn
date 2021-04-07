//
// Created by mauro on 4/7/21.
//

#include "streaming.h"

#include "streaming.h"

Streaming::~Streaming() {}

void Streaming::on_connected() {
    lwsl_user("client connected\n");
    std::string msg = R"({ "type": "subscribe", "product_ids": [ "ETH-USD", "ETH-EUR" ], "channels": [ "level2", "heartbeat", { "name": "ticker", "product_ids": [ "ETH-BTC", "ETH-USD" ] } ] })";
    send(msg.data(), msg.size());
}

void Streaming::on_disconnected() {
    lwsl_user("client disconnected\n");
}

void Streaming::on_error(const char *msg, size_t len) {
    lwsl_user("client error\n");
}

void Streaming::on_data(const char *data, size_t len, size_t remaining) {
    std::string msg(data, len);
    lwsl_user("data from server: %s\n", msg.c_str());
}