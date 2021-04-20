//
// Created by mauro on 4/7/21.
//

#include "streaming.h"

Streaming::Streaming() {
    // Node API
    nodeRequest_ = std::make_unique<httplib::Client>(
            "bsc_swapper", 3000
    );
    nodeRequest_->set_connection_timeout(120);

    // The graph
    graphRequest_ = std::make_unique<httplib::Client>(
            "graph_bsc.zinnion.com", 7000
    );
    graphRequest_->set_connection_timeout(15);
}

Streaming::~Streaming() {}

[[noreturn]] void Streaming::start() {
    system_debug_ = (strcasecmp("true", utils::getEnvVar("DEBUG").c_str()) == 0);
    if (system_debug_) {
        spdlog::info("DEBUG MODE IS ENABLED");
    }

//    loadPancakeSwapPrices();
//    rungWebServer();

    while (true) {
        runCycle();

        spdlog::info("Waiting 10s before next check.");

        sleep(10);
    }
}

void Streaming::rungWebServer() {
    try {
        server_.Get("/", [](const httplib::Request &req, httplib::Response &res) {
            std::string content = "Hi :)";
            res.set_content(content, "text/html");
        });

        server_.Get("/connections", [this](const httplib::Request &req, httplib::Response &res) {
            // Logic
            std::unordered_map<std::string, int64_t> seq_mapping;
            std::unordered_map<std::string, std::vector<Quotes>> connections;
            std::vector<std::string> result;
            std::unordered_map<std::string, Quotes> quotes;

            // Load the data
            if (!loadPancakeSwapPrices(quotes, connections)) {
                res.set_content("No quotes", "text/plain");
                return;
            }

            // Map unique symbols across all platforms
            int position = 0;
            for (auto const &[key, _] : connections) {
                // remove the exchange, we want only the symbol
                std::string symbol = key;
                size_t pos = symbol.find('-');
                symbol.erase(0, pos + 1);
                if (seq_mapping.count(symbol) == 0) {
                    seq_mapping[symbol] = position;
                    position++;
                }
            }

            // Build the direct edges
            std::vector<DirectedEdge *> directedEdge;
            buildEdgeWeightedDigraph(directedEdge, quotes, connections, seq_mapping);
            EdgeWeightedDigraph G(position);

            // backwards loop to maintain the mapping of edge with asset
            for (int x = directedEdge.size(); x-- > 0;) {
                G.addEdge(directedEdge[x]);
            }

            cout << G.toString() << endl;

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

        spdlog::info("Webserver listening on: 0.0.0.0:{}", 8181);
        server_.listen("0.0.0.0", 8181);
    } catch (std::exception &e) {
        spdlog::error("Webserver error: {}", e.what());
    }
}


void Streaming::buildEdgeWeightedDigraph(std::vector<DirectedEdge *> &directedEdge,
                                         std::unordered_map<std::string, Quotes> &quotes,
                                         std::unordered_map<std::string, std::vector<Quotes>> &connections,
                                         std::unordered_map<std::string, int64_t> &seq_mapping) {
    std::unordered_map<std::string, bool> connections_mapping;

    for (auto const &[_, data] : quotes) {
        for (auto const &x : connections[data.protocol + "-" + data.token0Symbol]) {
            std::string key;
            key.append(x.token0Symbol).append("-").append(x.token1Symbol).append("-").append(x.id);
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

                auto *e = new DirectedEdge(seq_mapping[x.token1Symbol],
                                           seq_mapping[x.token0Symbol],
//                                           x.token0Price, asset_1, asset_0);
                                           -std::log(x.token0Price), asset_1, asset_0);
                directedEdge.emplace_back(e);
            }
        }

        for (auto const &x : connections[data.protocol + "-" + data.token1Symbol]) {
            std::string key;
            key.append(x.token1Symbol).append("-").append(x.token0Symbol).append("-").append(x.id);
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

                auto *e = new DirectedEdge(seq_mapping[x.token0Symbol],
                                           seq_mapping[x.token1Symbol],
//                                           x.token1Price, asset_0, asset_1);
                                           -std::log(x.token1Price), asset_0, asset_1);
                directedEdge.emplace_back(e);
            }
        }
    }
}

void Streaming::runCycle() {
    auto elapsed = make_unique<Elapsed>("Arb Cycle");
    std::unordered_map<std::string, int64_t> seq_mapping;
    std::unordered_map<std::string, std::vector<Quotes>> connections;
    std::vector<std::string> result;
    std::unordered_map<std::string, Quotes> quotes;

    // Load the data
    if (!loadPancakeSwapPrices(quotes, connections)) {
        return;
    }

    // Map unique symbols across all platforms
    int position = 0;
    for (auto const &[key, _] : connections) {
        // remove the exchange, we want only the symbol
        std::string symbol = key;
        size_t pos = symbol.find('-');
        symbol.erase(0, pos + 1);
        if (seq_mapping.count(symbol) == 0) {
            seq_mapping[symbol] = position;
            position++;
        }
    }

    // Build the direct edges
    std::vector<DirectedEdge *> directedEdge;
    buildEdgeWeightedDigraph(directedEdge, quotes, connections, seq_mapping);
    EdgeWeightedDigraph G(position);

    // Backwards loop to maintain the mapping of edge with asset with the right position
    for (auto x = directedEdge.size(); x-- > 0;) {
        G.addEdge(directedEdge[x]);
    }

    // Arb opportunities found
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

            // We can have multiple executions with the same path, so, lets make sure we get only one
            const std::string executionhash = md5_from_file(output);
            if (hash.count(executionhash)) {
                // if the hash already exist, we dont need to add it again
                continue;
            }

            hash[executionhash] = true;
            arbitrage.output = output;

            // Only if starts with WBNB
            // Mainnet and Testnet addresses
            if (arbitrage.addr[0] == "0xbb4cdb9cbd36b01bd1cbaebf2de08d9173bc095c" ||
                arbitrage.addr[0] == "0xae13d989dac2f0debff460ac112a837c89baa7cd") {
                arbitrages.emplace_back(arbitrage);
            }

            cout << output << endl;
        } else {
            // cout << "No negative cycle" << endl;
        }
    }
    // Send for execution
    simulateArbitrage(arbitrages);
}

void Streaming::simulateArbitrage(const std::vector<Arbitrage> &arbitrages) {
    try {
        if (arbitrages.empty()) {
            spdlog::info("No Opportunities found");
            return;
        }

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

            for (auto const &x : arb.pool) {
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

                cout << execution_json << endl;

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

bool Streaming::loadPancakeSwapPrices(std::unordered_map<std::string, Quotes> &quotes,
                                      std::unordered_map<std::string, std::vector<Quotes>> &connections) {
    try {
        rapidjson::Document document;

        std::string url = "/subgraphs/name/maurodelazeri/exchange";
        std::string data = R"({ "query": "{ pairs( first: 1000 orderBy: reserveBNB orderDirection: desc where: {reserve0_gt: 0, reserve1_gt: 0} ) { id token0 { id name symbol derivedBNB decimals } token1 { id name symbol derivedBNB decimals } reserve0 reserve1 volumeToken0 volumeToken1 reserveBNB reserveUSD token0Price token1Price } }"})";

        auto res = graphRequest_->Post(url.c_str(), data, "application/json");
        if (res == nullptr) {
            spdlog::error("PancakeSwap subgraph error: {}", "nullptr");
            return false;
        }

        if (res.error()) {
            spdlog::error("PancakeSwap subgraph error: {}", res.error());
            return false;
        }

        // Parse the JSON
        if (document.Parse(res->body.c_str()).HasParseError()) {
            spdlog::error("PancakeSwap subgraph document parse error: {}", res->body.c_str());
            return false;
        }

        if (!document.IsObject()) {
            spdlog::error("PancakeSwap subgraph  error: {}", "No data");
            return false;
        }

        // Put the data int the struct
        if (document.HasMember("data")) {
            if (document["data"].HasMember("pairs")) {
                if (document["data"]["pairs"].IsArray()) {
                    const rapidjson::Value &pairs = document["data"]["pairs"];
                    for (rapidjson::SizeType i = 0; i < pairs.Size(); i++) {
                        Quotes quote;
                        quote.id = sole::uuid4().str();
                        quote.protocol = "PANCAKESWAP";
                        quote.poolID = pairs[i]["id"].GetString();

                        // Token 0
                        quote.token0Symbol = pairs[i]["token0"]["symbol"].GetString();
                        quote.token0decimals = std::stoi(
                                pairs[i]["token0"]["decimals"].GetString());
                        quote.token0derivedBNB = std::stod(
                                pairs[i]["token0"]["derivedBNB"].GetString());
                        quote.token0Address = pairs[i]["token0"]["id"].GetString();
                        quote.token0Price =
                                std::stod(pairs[i]["token0Price"].GetString());

                        // Token 1
                        quote.token1Symbol = pairs[i]["token1"]["symbol"].GetString();
                        quote.token1decimals = std::stoi(
                                pairs[i]["token1"]["decimals"].GetString());
                        quote.token1Address = pairs[i]["token1"]["id"].GetString();
                        quote.token1derivedBNB = std::stod(
                                pairs[i]["token1"]["derivedBNB"].GetString());
                        quote.token1Price =
                                std::stod(pairs[i]["token1Price"].GetString());

                        if (quote.token0Symbol.empty() || quote.token1Symbol.empty()) {
                            rapidjson::StringBuffer sb;
                            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
                            pairs[i].Accept(writer);
                            puts(sb.GetString());
                            spdlog::warn("PancakeSwap problem with pair: {}", sb.GetString());
                            continue;
                        }

                        // Unique ID
                        quotes[quote.id] = quote;
                        connections[quote.protocol + "-" + quote.token1Symbol].emplace_back(quote);
                        connections[quote.protocol + "-" + quote.token0Symbol].emplace_back(quote);
                    }
                    if (quotes.empty()) {
                        spdlog::warn("No quotes for PancakeSwap");
                        return false;
                    }
                } else {
                    return false;
                }
            } else {
                return false;
            }
        } else {
            return false;
        }
        return true;
    } catch (std::exception &e) {
        spdlog::error("PancakeSwap subgraph parse error: {}", e.what());
    }
    return false;
}