//
// Created by mauro on 5/20/20.
//
#pragma once

#include <openssl/sha.h>
#include <cstdio>
#include <string>
#include <iomanip>

namespace utils {
    inline std::string sha256(std::string const &data) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, data.c_str(), data.size());
        SHA256_Final(hash, &sha256);

        char outputBuffer[SHA256_DIGEST_LENGTH * 2 + 1];
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
            sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
        outputBuffer[SHA256_DIGEST_LENGTH * 2] = '\0';
        return outputBuffer;
    }

    inline const std::string generateSalt() {
        char salt[SHA256_DIGEST_LENGTH];
        const char alphanum[] =
                "0123456789"
                "!@#$%^&*"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz";
        for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
            salt[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
        }

        //salt[SHA256_DIGEST_LENGTH] = 0;
        std::string _salt = std::string(salt);
        return _salt;
    }

    inline const std::string generateHash(const std::string &password, const std::string &salt) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, password.c_str(), password.size());
        SHA256_Update(&sha256, salt.c_str(), salt.size());
        SHA256_Final(hash, &sha256);
        std::stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int) hash[i];
        }
        return ss.str();
    }
}