//
// Created by mauro on 4/7/21.
//
#pragma once

#define CPPHTTPLIB_OPENSSL_SUPPORT 1

#include <spdlog/spdlog.h>
#include <libwebsockets.h>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/writer.h>
#include <tbb/concurrent_hash_map.h>
#include "libs/misc/httplib.h"
#include "libs/misc/strings.h"
#include "libs/misc/sole.h"
#include "libs/ws_client/callback.h"
#include "libs/ws_client/websocket_client.h"

using namespace std;

struct Quotes {
    std::string id;
    std::string poolID;
    std::string protocol;
    std::string symbol;
    std::string name;
    double swap_fee;
    int decimals;
    int block_number;
    std::string transaction_hash;
    int processed_timestamp;
    std::string token0Symbol;
    std::string token1Symbol;
    std::string token0Address;
    std::string token1Address;
    int64_t token0decimals;
    int64_t token1decimals;
    double token0Price;
    double token1Price;
};

typedef tbb::concurrent_hash_map<string, Quotes> quotesTable;
typedef tbb::concurrent_hash_map<string, std::vector<Quotes>> connectionsTable;

class Streaming : public websocket_client, public client_callback_t {
private:
    bool system_debug_;

    std::unique_ptr<httplib::SSLClient> http_request_;

    quotesTable quotes_;
    connectionsTable connections_;

    void on_connected() override;

    void on_disconnected() override;

    void on_error(const char *msg, size_t len) override;

    void on_data(const char *data, size_t len, size_t remaining) override;

    bool load_active_pools();

    std::vector<double> getSpotPrice(const std::string &protocol, const std::vector<double> &weights, const std::vector<double> &balances);
public:
    Streaming() : websocket_client(this, "ws-feed.pro.coinbase.com") {
        lws_set_log_level(LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE, nullptr);
    }

    ~Streaming();

    void start();
};