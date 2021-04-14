//
// Created by mauro on 4/7/21.
//

#include "streaming.h"

Streaming::Streaming() : websocket_client(this, "wss.zinnion.com") {
    lws_set_log_level(LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE, nullptr);

    // NODE API
    nodeRequest_ = std::make_unique<httplib::Client>(
            "bsc_swapper", 3000
    );
    nodeRequest_->set_connection_timeout(120);
}

Streaming::~Streaming() {}

void Streaming::on_connected() {
    lwsl_user("client connected\n");
    std::string msg = R"({ "type": "subscribe", "channels": ["TICKERS_UNISWAPV2","TICKERS_SUSHISWAP","TICKERS_BALANCER","TICKERS_BANCOR","TICKERS_BALANCER","TICKERS_CURVEFI"] })";
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
    if (system_debug_) {
        lwsl_user("data from server: %s\n", msg.c_str());
    }
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

    // Websocket
    //connect();
//    load_active_pools();
//    rungWebServer();

    while (true) {
        auto elapsed = make_unique<Elapsed>("Arb Cycle");
        quotes_.clear();
        connections_.clear();
        load_active_pools();
        spdlog::info("{} quotes located", quotes_.size());
        runCycle();
    }

    rungWebServer();
}

void Streaming::rungWebServer() {
    try {
        server_.Get("/", [](const httplib::Request &req, httplib::Response &res) {
            std::string content = "Hi :)";
            res.set_content(content, "text/html");
        });

        server_.Get("/connections", [this](const httplib::Request &req, httplib::Response &res) {
            // Logic
            std::unordered_map<std::string, int> seq_mapping;
            std::vector<std::string> result;

            // Map unique symbols across all platforms
            int position = 0;
            {
                for (auto const &[key, _] : connections_) {
                    // remove the exchange, we want only the symbol
                    std::string addr = key;
                    size_t pos = addr.find('_');
                    addr.erase(0, pos + 1);
                    if (seq_mapping.count(addr) == 0) {
                        seq_mapping[addr] = position;
                        position++;
                    }
                }
            }

            // Build the direct edges
            std::vector<DirectedEdge *> directedEdge;
            buildEdgeWeightedDigraph(directedEdge, seq_mapping);
            EdgeWeightedDigraph G(position);

            // backwards loop to maintain the mapping of edge with asset
            for (auto x = directedEdge.size(); x-- > 0;) {
                G.addEdge(directedEdge[x]);
            }

            auto uuid_v4 = sole::uuid4().str();

            std::ofstream outfile("/tmp/connections-" + uuid_v4 + ".dot");
            outfile << G.getGraphviz() << std::endl;
            outfile.close();

            std::string command =
                    "dot /tmp/connections-" + uuid_v4 + ".dot -Tsvg > /tmp/connections-" + uuid_v4 + ".svg";
            utils::exec(command.c_str());

            std::ifstream ifs("/tmp/connections-" + uuid_v4 + ".svg");
            std::string content((std::istreambuf_iterator<char>(ifs)),
                                (std::istreambuf_iterator<char>()));
            res.set_content(content, "image/svg+xml");
            command = "rm /tmp/connections-" + uuid_v4 + ".dot /tmp/connections-" + uuid_v4 + ".svg";
            utils::exec(command.c_str());
        });

        server_.listen("0.0.0.0", 8181);
    } catch (std::exception &e) {
        spdlog::error("Webserver error: {}", e.what());
    }
}

void Streaming::buildEdgeWeightedDigraph(std::vector<DirectedEdge *> &directedEdge,
                                         std::unordered_map<std::string, int> &seq_mapping) {

    try {
        std::unordered_map<std::string, bool> connections_mapping;

        for (auto const &[_, data] : quotes_) {
            {
                connectionsTable::accessor connections_assessor1;
                connections_.find(connections_assessor1, data.protocol + "_" + data.token0Address);

                for (auto const &x : connections_assessor1->second) {
                    std::string key;
                    key.append(x.token0Address).append("_").append(x.token1Address).append("_").append(x.id);
                    if (connections_mapping.count(key) == 0 && connections_mapping.count(key) == 0) {

                        Asset asset_0;
                        asset_0.quoteId = x.id;
                        asset_0.symbol = x.token0Symbol;
                        asset_0.address = x.token0Address;
                        asset_0.exchange = x.protocol;
                        asset_0.poolID = x.poolID;
                        asset_0.decimals = x.token0decimals;

                        Asset asset_1;
                        asset_1.quoteId = x.id;
                        asset_1.symbol = x.token1Symbol;
                        asset_1.address = x.token1Address;
                        asset_1.exchange = x.protocol;
                        asset_1.poolID = x.poolID;
                        asset_1.decimals = x.token1decimals;

                        connections_mapping[key] = true;

                        auto *e = new DirectedEdge(seq_mapping[x.token1Address],
                                                   seq_mapping[x.token0Address],
//                                               x.token0Price, asset_1, asset_0);
                                                   -std::log(x.token0Price), asset_1, asset_0);
                        directedEdge.emplace_back(e);
                    }
                }
            }

            {
                connectionsTable::accessor connections_assessor2;
                connections_.find(connections_assessor2, data.protocol + "_" + data.token1Address);
                for (auto const &x : connections_assessor2->second) {
                    std::string key;
                    key.append(x.token1Address).append("_").append(x.token0Address).append("_").append(x.id);
                    if (connections_mapping.count(key) == 0 && connections_mapping.count(key) == 0) {

                        Asset asset_0;
                        asset_0.quoteId = x.id;
                        asset_0.symbol = x.token0Symbol;
                        asset_0.address = x.token0Address;
                        asset_0.exchange = x.protocol;
                        asset_0.poolID = x.poolID;
                        asset_0.decimals = x.token0decimals;

                        Asset asset_1;
                        asset_1.quoteId = x.id;
                        asset_1.symbol = x.token1Symbol;
                        asset_1.address = x.token1Address;
                        asset_1.exchange = x.protocol;
                        asset_1.poolID = x.poolID;
                        asset_1.decimals = x.token1decimals;

                        connections_mapping[key] = true;

                        auto *e = new DirectedEdge(seq_mapping[x.token0Address],
                                                   seq_mapping[x.token1Address],
//                                               x.token1Price, asset_0, asset_1);
                                                   -std::log(x.token1Price), asset_0, asset_1);
                        directedEdge.emplace_back(e);
                    }
                }
            }
        }
    } catch (std::exception &e) {
        spdlog::error("buildEdgeWeightedDigraph error: {}", e.what());
    }
}

void Streaming::runCycle() {
    // Logic
    std::unordered_map<std::string, int> seq_mapping;
    std::vector<std::string> result;

    // Map unique symbols across all platforms
    int position = 0;
    {
        for (auto &connection : connections_) {
            // remove the exchange, we want only the symbol
            std::string addr = connection.first;
            size_t pos = addr.find('_');
            addr.erase(0, pos + 1);
            if (seq_mapping.count(addr) == 0) {
                seq_mapping[addr] = position;
                position++;
            }
        }
    }

    // Build the direct edges
    spdlog::info("Building directed edges");
    std::vector<DirectedEdge *> directedEdge;
    buildEdgeWeightedDigraph(directedEdge, seq_mapping);
    EdgeWeightedDigraph G(position);

    // Backwards loop to maintain the mapping of edge with asset with the right position
    for (auto x = directedEdge.size(); x-- > 0;) {
        G.addEdge(directedEdge[x]);
    }

    std::vector<Arbitrage> arbitrages;

    spdlog::info("Checking arbitrage opportunities");
    std::unordered_map<std::string, bool> hash;
    for (int i = 0; i < position; i++) {
        // find negative cycle
        BellmanFordSP spt(G, i);

        if (spt.hasNegativeCycle()) {
            stack<DirectedEdge *> edges(spt.negativeCycle());
            std::string output;
            double stake = 1;
            double final_stake = stake;

            Arbitrage arbitrage;
            while (!edges.empty()) {
                char *m1 = nullptr;
                asprintf(&m1, "%10.5f %s-%s-%s ", final_stake, edges.top()->asset_from().exchange.c_str(),
                         edges.top()->asset_from().symbol.c_str(), edges.top()->asset_from().address.c_str());
                output.append(m1);
                free(m1);

                final_stake *= std::exp(-edges.top()->weight());

                char *m2 = nullptr;
                asprintf(&m2, "= %10.5f %s-%s-%s\n", final_stake, edges.top()->asset_to().exchange.c_str(),
                         edges.top()->asset_to().symbol.c_str(), edges.top()->asset_to().address.c_str());
                output.append(m2);
                free(m2);

                arbitrage.addr.emplace_back(edges.top()->asset_from().address);
                arbitrage.addr.emplace_back(edges.top()->asset_to().address);
                arbitrage.exchange.emplace_back(edges.top()->asset_to().exchange);
                arbitrage.pool.emplace_back(edges.top()->asset_to().poolID);

                edges.pop();
            }

            const std::string executionhash = md5_from_file(output);
            if (hash.count(executionhash)) {
                // if the hash already exist, we dont need to add it again
                continue;
            }

            hash[executionhash] = true;
            arbitrage.output = output;

            // Only if starts with WBNB
            if (arbitrage.addr[0] == "0xbb4cdb9cbd36b01bd1cbaebf2de08d9173bc095c") {
                arbitrages.emplace_back(arbitrage);
            }

            //cout << output << endl;
        } else {
            // cout << "No negative cycle" << endl;
        }
    }
    // Send for execution
    simulateArbitrage(arbitrages);
    sleep(10);
}

void Streaming::simulateArbitrage(const std::vector<Arbitrage> &arbitrages) {
    try {
        spdlog::info("{} Opportunities found, sending it over to further check", arbitrages.size());

        double final_profit = 0.0;
        int current_index = 0;
        int execution_index = 0;
        std::string execution_json;

        for (auto const &arb : arbitrages) {
            rapidjson::Document request_document;
            rapidjson::Document::AllocatorType &allocator = request_document.GetAllocator();
            request_document.SetObject();

            rapidjson::Value obj(rapidjson::kObjectType);
            rapidjson::Value val(rapidjson::kObjectType);
            rapidjson::Value exchangeArray(rapidjson::kArrayType);
            rapidjson::Value addrArray(rapidjson::kArrayType);
            rapidjson::Value poolArray(rapidjson::kArrayType);

            for (auto const &x : arb.exchange) {
                val.SetString(x.c_str(), static_cast<rapidjson::SizeType>(x.length()),
                              allocator);
                exchangeArray.PushBack(val, allocator);
            }

            for (auto const &x : arb.addr) {
                val.SetString(x.c_str(), static_cast<rapidjson::SizeType>(x.length()),
                              allocator);
                addrArray.PushBack(val, allocator);
            }

            for (auto const &x : arb.addr) {
                val.SetString(x.c_str(), static_cast<rapidjson::SizeType>(x.length()),
                              allocator);
                poolArray.PushBack(val, allocator);
            }

            val.SetString(arb.output.c_str(),
                          static_cast<rapidjson::SizeType>(arb.output.length()),
                          allocator);
            request_document.AddMember("output", val, allocator);
            request_document.AddMember("exchange", exchangeArray, allocator);
            request_document.AddMember("addr", addrArray, allocator);
            request_document.AddMember("pool", poolArray, allocator);

            val.SetDouble(initial_volume_);
            request_document.AddMember("starting_volume", val, allocator);

            rapidjson::StringBuffer sb;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
            request_document.Accept(writer);

            std::string url = "/simulation";
            auto res = nodeRequest_->Post(url.c_str(), sb.GetString(), "application/json");
            if (res == nullptr) {
                spdlog::error("Node api error 1: {}", "nullptr");
                return;
            }

            if (res.error()) {
                spdlog::error("Node api error 2: {}", res.error());
                return;
            }

            rapidjson::Document document;

            // Parse the JSON
            if (document.Parse(res->body.c_str()).HasParseError()) {
                spdlog::error("Node api document parse error: {}", res->body.c_str());
                return;
            }

            if (!document.IsObject()) {
                spdlog::error("Node api  error: {}", "No data");
                return;
            }

            if (document.HasMember("error")) {
                const rapidjson::Value &error = document["error"];
                if (error.GetBool()) {
                    const rapidjson::Value &message = document["message"];
                    spdlog::error("Node api: {}", message.GetString());
                }
            } else {
                spdlog::error("Node api return does not contain a error status");
                return;
            }
            if (document.HasMember("profit")) {
                const rapidjson::Value &profit = document["profit"];
                //spdlog::info("Simulation of profit: {}", profit.GetDouble());
                if (final_profit < profit.GetDouble()) {
                    final_profit = profit.GetDouble();
                    execution_json = sb.GetString();
                    execution_index = current_index;
                }
            }
            current_index++;
        }

        if (final_profit > 0) {
            spdlog::info("Profitable operation found {}", final_profit);

            if (arbitrages[execution_index].exchange.size() == 2) {
                final_profit -= 0.00182082;
                final_profit -= initial_volume_ * 0.003;
                final_profit -= initial_volume_ * 0.003;
            } else if (arbitrages[execution_index].exchange.size() == 3) {
                final_profit -= 0.00313629;
                final_profit -= initial_volume_ * 0.003;
                final_profit -= initial_volume_ * 0.003;
                final_profit -= initial_volume_ * 0.003;
            } else if (arbitrages[execution_index].exchange.size() == 4) {
                final_profit -= 0.00399482;
                final_profit -= initial_volume_ * 0.003;
                final_profit -= initial_volume_ * 0.003;
                final_profit -= initial_volume_ * 0.003;
                final_profit -= initial_volume_ * 0.003;
            } else if (arbitrages[execution_index].exchange.size() == 5) {
                final_profit -= 0.0047071;
                final_profit -= initial_volume_ * 0.003;
                final_profit -= initial_volume_ * 0.003;
                final_profit -= initial_volume_ * 0.003;
                final_profit -= initial_volume_ * 0.003;
                final_profit -= initial_volume_ * 0.003;
            } else if (arbitrages[execution_index].exchange.size() == 6) {
                final_profit -= 0.00525459;
                final_profit -= initial_volume_ * 0.003;
                final_profit -= initial_volume_ * 0.003;
                final_profit -= initial_volume_ * 0.003;
                final_profit -= initial_volume_ * 0.003;
                final_profit -= initial_volume_ * 0.003;
                final_profit -= initial_volume_ * 0.003;
            } else {
                spdlog::info("Execution has more than 6 hoops, we are not handling it");
                return;
            }

            if (final_profit > 0) {
                spdlog::info("Operation payload {}", arbitrages[execution_index].output);
                spdlog::info("Sending execution expecting {} BNB in returns.", final_profit);
                executeArbitrage(arbitrages[execution_index], execution_json);
            } else {
                spdlog::info("No Profitable profits profits after fees");
            }
        } else {
            spdlog::info("Nothing to execute");
        }
    } catch (std::exception &e) {
        spdlog::error("simulateArbitrage error: {}", e.what());
    }
}

void Streaming::executeArbitrage(const Arbitrage &arbitrage, const std::string &execution_json) {
    try {
        std::string url = "/trade";
        auto res = nodeRequest_->Post(url.c_str(), execution_json, "application/json");
        if (res == nullptr) {
            spdlog::error("Node api error 1: {}", "nullptr");
            return;
        }

        if (res.error()) {
            spdlog::error("Node api error 2: {}", res.error());
            return;
        }

        rapidjson::Document document;

        // Parse the JSON
        if (document.Parse(res->body.c_str()).HasParseError()) {
            spdlog::error("Node api document parse error: {}", res->body.c_str());
            return;
        }

        if (!document.IsObject()) {
            spdlog::error("Node api  error: {}", "No data");
            return;
        }

        if (document.HasMember("error")) {
            const rapidjson::Value &error = document["error"];
            if (error.GetBool()) {
                const rapidjson::Value &message = document["message"];
                spdlog::error("Node api: {}", message.GetString());
            }
        } else {
            spdlog::error("Node api return does not contain a error status");
            return;
        }

        if (document.HasMember("executed")) {
            const rapidjson::Value &executed = document["executed"];
            if (executed.GetBool()) {
                if (document.HasMember("transactionHash")) {
                    const rapidjson::Value &transactionHash = document["transactionHash"];
                    const rapidjson::Value &profit = document["profit"];
                    spdlog::info("Trade executed: transactionHash: {} Profit: {}", transactionHash.GetString(),
                                 profit.GetDouble());

                    std::ofstream out;
                    auto givemetime = chrono::system_clock::to_time_t(chrono::system_clock::now());
                    out.open("/opt/executions.log", std::ios::app);
                    out << "-----------------------------------\n";
                    out << ctime(&givemetime) << "Transaction Hash: " << transactionHash.GetString() << "\n"
                        << "Profit: " << profit.GetDouble();
                }
            }
        }
    } catch (std::exception &e) {
        spdlog::error("simulateArbitrage error: {}", e.what());
    }
}

bool Streaming::load_active_pools() {
    try {
        int64_t cursor = -1;
        while (cursor != 0) {
            if (cursor == -1) {
                cursor = 0;
            }
            cout << "Loading data, cursor at:" << cursor << endl;

            rapidjson::Document document;
            std::string url = "/v1/active-pools/bsc/*/" + to_string(cursor) + "/1000";
            auto res = http_request_->Get(url.c_str());
            if (res == nullptr) {
                spdlog::error("Error: {}", "nullptr");
                break;
            }

            if (res.error()) {
                spdlog::error("Error: {}", res.error());
                break;
            }

            // Parse the JSON
            if (document.Parse(res->body.c_str()).HasParseError()) {
                spdlog::error("Document parse error: {}", res->body.c_str());
                break;
            }

            if (!document.IsObject()) {
                spdlog::error("Error: {}", "No data");
                break;
            }

            if (document.HasMember("data")) {
                cursor = document["next_cursor"].GetInt64();

                const rapidjson::Value &data = document["data"];
                for (rapidjson::SizeType i = 0; i < data.Size(); i++) {
                    Quotes quote;
                    quote.id = sole::uuid4().str();
                    quote.poolID = data[i]["pool_id"].GetString();
                    quote.chain = data[i]["chain"].GetString();
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
                    if (document.HasMember("amplification")) {
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

                    if (quote.chain != "BSC") {
                        continue;
                    }

                    // Quotes
                    {
                        quotesTable::accessor quotes_assessor;
                        quotes_.find(quotes_assessor, quote.id);
                        if (quotes_assessor.empty()) {
                            quotes_.insert(quotes_assessor, quote.id);
                        }
                        quotes_assessor->second = quote;
                    }
                    // Connections
                    {
                        connectionsTable::accessor connections_assessor;
                        connections_.find(connections_assessor, quote.protocol + "_" + quote.token0Address);
                        if (connections_assessor.empty()) {
                            connections_.insert(connections_assessor, quote.protocol + "_" + quote.token0Address);
                        }
                        connections_assessor->second.emplace_back(quote);
                    }
                    {
                        connectionsTable::accessor connections_assessor;
                        connections_.find(connections_assessor, quote.protocol + "_" + quote.token1Address);
                        if (connections_assessor.empty()) {
                            connections_.insert(connections_assessor, quote.protocol + "_" + quote.token1Address);
                        }
                        connections_assessor->second.emplace_back(quote);
                    }
                }
            } else {
                break;
            }
        }
        return true;
    } catch (std::exception &e) {
        spdlog::error("Parse error: {}", e.what());
    }
    return false;
}