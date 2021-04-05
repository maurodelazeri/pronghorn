//
// Created by mauro on 5/20/20.
//
#pragma once

#include <google/protobuf/util/json_util.h>
#include <google/protobuf/util/json_util.h>

namespace utils {
    inline std::string ProtoToJson(const google::protobuf::Message &proto) {
        try {
            google::protobuf::util::JsonOptions stOpt;
            stOpt.always_print_enums_as_ints = true;
            stOpt.always_print_primitive_fields = true;
            stOpt.preserve_proto_field_names = true;

            std::string json;
            google::protobuf::util::MessageToJsonString(proto, &json, stOpt);
            return json;
        }
        catch (std::exception &e) {
            std::cerr << "Problem to parse inline json" << e.what() << std::endl;
            return "";
        }
    }
}