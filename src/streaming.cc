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
                quote.id = sole::uuid4().str();
                quote.poolID = data[i]["pool_id"].GetString();
                quote.protocol = data[i]["protocol"].GetString();
                quote.symbol = data[i]["symbol"].GetString();
                quote.name = data[i]["name"].GetString();
                quote.swap_fee = std::stod(data[i]["swap_fee"].GetString());
                quote.decimals = std::stoi(data[i]["decimals"].GetString());
                quote.block_number = data[i]["block_number"].GetInt64();
                quote.processed_timestamp = data[i]["processed_timestamp"].GetInt64();
                quote.transaction_hash = data[i]["transaction_hash"].GetString();

                if (document.HasMember("virtual_price")) {
                    quote.virtual_price = std::stod(data[i]["virtual_price"].GetString());
                }
                if (document.HasMember("virtual_price")) {
                    quote.amplification = std::stoi(data[i]["amplification"].GetString());
                }

                const rapidjson::Value &tokens = data[i]["tokens"];
                for (rapidjson::SizeType x = 0; x < tokens.Size(); x++) {
                    for (rapidjson::SizeType y = x + 1; y < tokens.Size(); y++) {
                        // Token 0
                        quote.token0Symbol = tokens[x]["symbol"].GetString();
                        quote.token0decimals = std::stoi(tokens[x]["decimals"].GetString());
                        quote.token0Address = tokens[x]["address"].GetString();
                        quote.token0Reserves = std::stod(tokens[x]["reserves"].GetString());
                        quote.token0Weight = std::stod(tokens[x]["weight"].GetString());

                        if (quote.protocol == "CURVEFI") {
                            quote.token0Price = quote.virtual_price;
                        } else {
                            quote.token0Price = calcSpotPrice(std::stod(tokens[x]["reserves"].GetString()),
                                                              std::stod(tokens[x]["weight"].GetString()),
                                                              std::stod(tokens[y]["reserves"].GetString()),
                                                              std::stod(tokens[y]["weight"].GetString()));

                        }
                        // Token 1
                        quote.token1Symbol = tokens[y]["symbol"].GetString();
                        quote.token1decimals = std::stoi(tokens[y]["decimals"].GetString());
                        quote.token1Address = tokens[y]["address"].GetString();
                        quote.token1Reserves = std::stod(tokens[y]["reserves"].GetString());
                        quote.token1Weight = std::stod(tokens[y]["weight"].GetString());

                        if (quote.protocol == "CURVEFI") {
                            quote.token1Price = quote.virtual_price;
                        } else {
                            quote.token1Price = calcSpotPrice(std::stod(tokens[y]["reserves"].GetString()),
                                                              std::stod(tokens[y]["weight"].GetString()),
                                                              std::stod(tokens[x]["reserves"].GetString()),
                                                              std::stod(tokens[x]["weight"].GetString()));
                        }
                    }
                }

                // Quotes
                quotesTable::accessor quotes_assessor;
                quotes_.find(quotes_assessor, quote.id);
                if (quotes_assessor.empty()) {
                    quotes_.insert(quotes_assessor, quote.id);
                }
                quotes_assessor->second = quote;

                // Connections
                connectionsTable::accessor connections_assessor;
                connections_.find(connections_assessor, quote.protocol + "-" + quote.token0Symbol);
                if (connections_assessor.empty()) {
                    connections_.insert(connections_assessor, quote.protocol + "-" + quote.token0Symbol);
                }
                connections_assessor->second.emplace_back(quote);

                connections_.find(connections_assessor, quote.protocol + "-" + quote.token1Symbol);
                if (connections_assessor.empty()) {
                    connections_.insert(connections_assessor, quote.protocol + "-" + quote.token1Symbol);
                }
                connections_assessor->second.emplace_back(quote);
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