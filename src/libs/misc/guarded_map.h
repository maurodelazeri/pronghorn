//
// Created by mauro on 5/23/19.
//

#pragma once

#include <map>
#include <mutex>

template <class K, class V, class Compare = std::less<K>, class Allocator = std::allocator<std::pair<const K, V> > >
class guarded_map {
private:
    std::map<K, V, Compare, Allocator> _map;
    std::mutex _m;

public:
    void set(K key, V value) {
        std::lock_guard<std::mutex> lk(this->_m);
        this->_map[key] = value;
    }

    V & get(K key) {
        std::lock_guard<std::mutex> lk(this->_m);
        return this->_map[key];
    }

    bool empty() {
        std::lock_guard<std::mutex> lk(this->_m);
        return this->_map.empty();
    }

    bool clear() {
        std::lock_guard<std::mutex> lk(this->_m);
        return this->_map.clear();
    }

    bool erase(K key) {
        std::lock_guard<std::mutex> lk(this->_m);
        return this->_map.erase(key);
    }
    // other public methods you need to implement
};
