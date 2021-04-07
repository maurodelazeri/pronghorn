//
// Created by mauro on 4/7/21.
//

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

void Streaming::start() {
    system_debug_ = (strcasecmp("true", utils::getEnvVar("DEBUG").c_str()) == 0);
    if (system_debug_) {
        spdlog::info("DEBUG MODE IS ENABLED");
    }

    http_request_ = std::make_unique<httplib::SSLClient>(
            "api.zinnion.com", 443
    );
    http_request_->set_connection_timeout(15);

    load_active_pools();

    // Websocket
    connect();
}

std::vector<double> Streaming::getSpotPrice(const std::string &protocol, const std::vector<double> &weights,
                                            const std::vector<double> &balances) {
    std::vector<double> spot_prices;

    return spot_prices;
}

bool Streaming::load_active_pools() {
    try {
        rapidjson::Document document;

        std::string url = "/v1/active-pools/all/0/1000000000";
        auto res = http_request_->Get(url.c_str());
        if (res == nullptr) {
            spdlog::error("Error: {}", "nullptr");
            return false;
        }

        if (res.error()) {
            spdlog::error("Error: {}", res.error());
            return false;
        }

        // Parse the JSON
        if (document.Parse(res->body.c_str()).HasParseError()) {
            spdlog::error("Document parse error: {}", res->body.c_str());
            return false;
        }

        if (!document.IsObject()) {
            spdlog::error("Error: {}", "No data");
            return false;
        }

        if (document.HasMember("data")) {
            const rapidjson::Value &data = document["data"];
            for (rapidjson::SizeType i = 0; i < data.Size(); i++) {
                Quotes quote;
                quotesTable::accessor quotes_assessor;
                quotes_.find(quotes_assessor, data[i]["pool_id"].GetString());
                if (quotes_assessor.empty()) {
                    quotes_.insert(quotes_assessor, data[i]["pool_id"].GetString());
                }

                quote.poolID = data[i]["pool_id"].GetString();
                quote.protocol = data[i]["protocol"].GetString();
                quote.symbol = data[i]["symbol"].GetString();
                quote.name = data[i]["name"].GetString();
                quote.swap_fee = std::stod(data[i]["swap_fee"].GetString());
                quote.decimals = std::stoi(data[i]["decimals"].GetString());
                quote.block_number = std::stoi(data[i]["block_number"].GetString());
                quote.processed_timestamp = std::stoi(data[i]["processed_timestamp"].GetString());
                quote.transaction_hash = data[i]["transaction_hash  "].GetString();

                std::vector<double> spot_price = getSpotPrice(quote.protocol, std::vector<double>{
                                                                      std::stod(data[i][0]["reserves"].GetString()), std::stod(data[i][1]["reserves"].GetString())},
                                                              std::vector<double>{
                                                                      std::stod(data[i][0]["weight"].GetString()),
                                                                      std::stod(data[i][0]["weight"].GetString())});

                // Token 0
                quote.token0Symbol = data[i][0]["symbol"].GetString();
                quote.token0decimals = std::stoi(data[i]["decimals"].GetString());
                quote.token0Address = data[i][0]["address"].GetString();
                quote.token0Price = spot_price[0];

                // Token 1
                quote.token1Symbol = data[i][0]["symbol"].GetString();
                quote.token1decimals = std::stoi(data[i]["decimals"].GetString());
                quote.token1Address = data[i][0]["address"].GetString();
                quote.token1Price = spot_price[0];

                quotes_assessor->second = quote;
            }
        } else {
            return false;
        }
        return true;
    } catch (std::exception &e) {
        spdlog::error("Parse error: {}", e.what());
    }
    return false;
}