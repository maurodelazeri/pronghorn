//
// Created by mauro on 5/20/20.
//

#pragma once

#include <jwt-cpp/jwt.h>

namespace utils {

    inline void CheckToken(std::unordered_map<std::string, std::string> &token) {
        try {
            auto decoded = jwt::decode(token["token"]);
            auto verifier = jwt::verify()
                    .allow_algorithm(jwt::algorithm::hs256{"59fae4e5-53de-47aa-95c9-73dd811ffb45"})
                    .with_issuer("phenix");

            verifier.verify(decoded);

            for (auto &e : decoded.get_payload_claims()) {
                if (e.first == "jti") {
                    token["status"] = "success";
                    token["user_id"] = e.second.as_string();
                }
            }

            if (token["user_id"].empty()) {
                token["message"] = "user jti not set";
            }

        } catch (std::exception &e) {
            std::cerr << "Caught unknown exception:" << e.what();
            token["message"] = e.what();
            return;
        }
    }

    inline std::string generateToken(const std::string &issuer, const std::string &user_id) {
        // JWT Token
        //Create JWT object
        auto token = jwt::create()
                .set_issuer(issuer)
                .set_issued_at(std::chrono::system_clock::now())
                .set_id(user_id)
                .set_issued_at(std::chrono::system_clock::now())
                .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours{8760})
                .sign(jwt::algorithm::hs256{"59fae4e5-53de-47aa-95c9-73dd811ffb45"});

        return token;
    }
}