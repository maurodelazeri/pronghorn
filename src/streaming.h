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
#include "libs/misc/system.h"
#include "libs/misc/sole.h"
#include "libs/misc/elapsed.h"
#include "libs/misc/md5.h"
#include "libs/ws_client/callback.h"
#include "libs/ws_client/websocket_client.h"
#include "libs/match.h"

#include "libs/graph/directed_edge.h"
#include "libs/graph/edge_weighted_digraph.h"
#include "libs/graph/bellman_ford_sp.h"

using namespace std;

struct Quotes {
    std::string id;
    std::string poolID;
    std::string chain;
    std::string protocol;
    std::string symbol;
    std::string name;
    double swap_fee;
    double virtual_price;
    int64_t amplification;
    int64_t decimals;
    int64_t block_number;
    std::string transaction_hash;
    int64_t processed_timestamp;
    std::string token0Symbol;
    std::string token1Symbol;
    std::string token0Address;
    std::string token1Address;
    int64_t token0decimals;
    int64_t token1decimals;
    double token0Reserves;
    double token1Reserves;
    double token0Weight;
    double token1Weight;
    double token0Price;
    double token1Price;
};

struct Arbitrage {
    std::vector<std::string> addr;
    std::vector<std::string> exchange;
    std::vector<std::string> pool;
    std::string output;
};

typedef tbb::concurrent_hash_map<string, Quotes> quotesTable;
typedef tbb::concurrent_hash_map<string, std::vector<Quotes>> connectionsTable;

class Streaming : public websocket_client, public client_callback_t {
private:
    bool system_debug_;

    double initial_volume_ = 1;

    httplib::Server server_;
    std::unique_ptr<httplib::Client> nodeRequest_;

    std::unique_ptr<httplib::SSLClient> http_request_;

    quotesTable quotes_;
    connectionsTable connections_;

    void on_connected() override;

    void on_disconnected() override;

    void on_error(const char *msg, size_t len) override;

    void on_data(const char *data, size_t len, size_t remaining) override;

    bool load_active_pools();

    void runCycle();

    void buildEdgeWeightedDigraph(std::vector<DirectedEdge *> &directedEdge,
                                  std::unordered_map<std::string, int> &seq_mapping);

    void simulateArbitrage(const std::vector<Arbitrage> &arbitrages);

    void executeArbitrage(const Arbitrage &arbitrage, const std::string &execution_json);

public:
    Streaming();

    ~Streaming();

    void start();

    void rungWebServer();
};