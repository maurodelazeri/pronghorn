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
#include "libs/match.h"
#include "libs/graph/directed_edge.h"
#include "libs/graph/edge_weighted_digraph.h"
#include "libs/graph/bellman_ford_sp.h"

using namespace std;

struct Quotes {
    std::string id;
    std::string protocol;
    std::string poolID;
    std::string token0Symbol;
    std::string token1Symbol;
    std::string token0Address;
    std::string token1Address;
    int64_t token0decimals;
    int64_t token1decimals;
    double token0Price;
    double token1Price;
    double token0derivedBNB;
    double token1derivedBNB;
};

struct Arbitrage {
    std::vector<std::string> addr;
    std::vector<std::string> exchange;
    std::vector<std::string> pool;
    std::string output;
};

class Streaming {
private:
    bool system_debug_;

    double initial_volume_ = 0.005;

    httplib::Server server_;
    std::unique_ptr<httplib::Client> nodeRequest_;
    std::unique_ptr<httplib::Client> graphRequest_;

    bool loadPancakeSwapPrices(std::unordered_map<std::string, Quotes> &quotes,std::unordered_map<std::string, std::vector<Quotes>> &connections);

    void runCycle();

    void buildEdgeWeightedDigraph(std::vector<DirectedEdge *> &directedEdge,
                                  std::unordered_map<std::string, Quotes> &quotes,
                                  std::unordered_map<std::string, std::vector<Quotes>> &connections,
                                  std::unordered_map<std::string, int> &seq_mapping);

    void simulateArbitrage(const std::vector<Arbitrage> &arbitrages);

    void executeArbitrage(const Arbitrage &arbitrage, const std::string &execution_json);

    std::unordered_map<int,std::string> mauro;
public:
    Streaming();

    ~Streaming();

    [[noreturn]] void start();

    void rungWebServer();
};