//
// Created by mauro on 3/5/21.
//

#pragma once

struct Asset {
    std::string quoteId{};
    std::string poolID{};
    std::string exchange{};
    std::string symbol{};
    std::string address{};
    std::string exchange_symbol{};
    int64_t decimals{};
};