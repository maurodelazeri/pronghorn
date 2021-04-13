//
// Created by mauro on 4/13/21.
//
#pragma once

#include <openssl/md5.h>
#include <iomanip>
#include <boost/iostreams/device/mapped_file.hpp>

inline std::string md5_from_file(const std::string &str) {
    unsigned char result[MD5_DIGEST_LENGTH];
    MD5((unsigned char *) str.data(), str.size(), result);
    std::ostringstream sout;
    sout << std::hex << std::setfill('0');
    for (auto c: result) sout << std::setw(2) << (int) c;
    return sout.str();
}

