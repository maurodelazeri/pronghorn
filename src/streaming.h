//
// Created by mauro on 4/7/21.
//
#pragma once

#include <spdlog/spdlog.h>
#include <libwebsockets.h>
#include "libs/ws_client/callback.h"
#include "libs/ws_client/websocket_client.h"

using namespace std;

class Streaming : public websocket_client, public client_callback_t {
private:
    void on_connected() override;

    void on_disconnected() override;

    void on_error(const char *msg, size_t len) override;

    void on_data(const char *data, size_t len, size_t remaining) override;

public:
    Streaming() : websocket_client(this, "ws-feed.pro.coinbase.com") {
        lws_set_log_level(LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE, nullptr);
    }

    ~Streaming();
};