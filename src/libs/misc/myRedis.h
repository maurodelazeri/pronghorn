//
// Created by mauro on 12/24/20.
//

#pragma once

#include <sw/redis++/redis++.h>

template<typename T>
class Redis {
public:
    static T *getInstance() {
        pthread_once(&once_, init);
        return _pInstance;
    }

    static void init() {
        _con.db = 0;
//        _con.host = "64.111.209.146";
        _con.host = "127.0.0.1";
        _con.port = 6379;
        _con_pool.size = 3;
        _con.password = "Br@sa154";
        _pInstance = new T(_con, _con_pool);
        atexit(destroy);
    }

    static void destroy() {
        if (_pInstance) {
            delete _pInstance;
            _pInstance = nullptr;
        }
    }

private:
    Redis() {}

    ~Redis() {}

    static T *_pInstance;
    static pthread_once_t once_;
    static sw::redis::ConnectionOptions _con;
    static sw::redis::ConnectionPoolOptions _con_pool;
};

template<typename T>
T *Redis<T>::_pInstance = nullptr;

template<typename T>
pthread_once_t Redis<T>::once_ = PTHREAD_ONCE_INIT;

template<typename T>
sw::redis::ConnectionOptions Redis<T>::_con;

template<typename T>
sw::redis::ConnectionPoolOptions Redis<T>::_con_pool;