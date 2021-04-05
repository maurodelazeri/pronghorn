//
// Created by mauro on 5/20/20.
//

#pragma once

#include <chrono>
#include <sstream>
#include <date/date.h>
#include <sys/time.h>
#include "strings.h"

namespace utils {

    inline int ParseInt(const char *value) {
        return std::strtol(value, nullptr, 10);
    }

    inline std::string to_iso_8601(std::chrono::time_point<std::chrono::system_clock> t) {

        // convert to time_t which will represent the number of
        // seconds since the UNIX epoch, UTC 00:00:00 Thursday, 1st. January 1970
        auto epoch_seconds = std::chrono::system_clock::to_time_t(t);

        // Format this as date time to seconds resolution
        // e.g. 2016-08-30T08:18:51
        std::stringstream stream;
        stream << std::put_time(gmtime(&epoch_seconds), "%FT%T%");

        // If we now convert back to a time_point we will get the time truncated
        // to whole seconds
        auto truncated = std::chrono::system_clock::from_time_t(epoch_seconds);

        // Now we subtract this seconds count from the original time to
        // get the number of extra microseconds..
        auto delta_us = std::chrono::duration_cast<std::chrono::microseconds>(t - truncated).count();

        // And append this to the output stream as fractional seconds
        // e.g. 2016-08-30T08:18:51.867479
        stream << "." << std::fixed << std::setw(6) << std::setfill('0') << delta_us;

        return stream.str();
    }

    inline std::time_t ParseISO8601(const std::string &input) {
        constexpr const size_t expectedLength = sizeof("1234-12-12T12:12:12") - 1;
        static_assert(expectedLength == 19, "Unexpected ISO 8601 date/time length");

        if (input.length() < expectedLength) {
            return 0;
        }

        std::tm time = {0};
        time.tm_year = ParseInt(&input[0]) - 1900;
        time.tm_mon = ParseInt(&input[5]) - 1;
        time.tm_mday = ParseInt(&input[8]);
        time.tm_hour = ParseInt(&input[11]);
        time.tm_min = ParseInt(&input[14]);
        time.tm_sec = ParseInt(&input[17]);
        time.tm_isdst = 0;
        const int millis = input.length() > 20 ? ParseInt(&input[20]) : 0;
        return timegm(&time) * 1000 + millis;
    }

    inline std::string date_back_in_time(const std::string &ref_time, const unsigned int &minutes_back) {
        // format expected to receive 2020-05-15T13:16:52.359144Z
        // format expected to give back 2020-05-15T13:16:52

        std::unordered_map<int, std::string> fill_zero = {
                {0,  "00"},
                {1,  "01"},
                {2,  "02"},
                {3,  "03"},
                {4,  "04"},
                {5,  "05"},
                {6,  "06"},
                {7,  "07"},
                {8,  "08"},
                {9,  "09"},
                {10, "10"},
                {11, "11"},
                {12, "12"},
        };

        std::string mystr = ref_time;
        std::string dateStr = mystr.substr(0, 10);
        std::string timeStr = mystr.erase(0, 11);
        timeStr = mystr.substr(0, 8);
        std::vector<std::string> date_vec = utils::split(dateStr, "-");
        std::vector<std::string> time_vec = utils::split(timeStr, ":");
        boost::posix_time::ptime t = boost::posix_time::ptime(
                boost::gregorian::date((short unsigned int) std::strtol(date_vec[0].c_str(), nullptr, 10),
                                       (short unsigned int) std::strtol(date_vec[1].c_str(), nullptr, 10),
                                       (short unsigned int) std::strtol(date_vec[2].c_str(), nullptr, 10)),
                boost::posix_time::time_duration((short unsigned int) std::strtol(time_vec[0].c_str(), nullptr, 10),
                                                 (short unsigned int) std::strtol(time_vec[1].c_str(), nullptr, 10),
                                                 (short unsigned int) std::strtol(time_vec[2].c_str(), nullptr, 10),
                                                 0));

        t -= boost::posix_time::minutes(minutes_back);
        std::stringstream stream;
        stream << t.date().year();
        stream << "-";
        stream << fill_zero[t.date().month().as_enum()];
        stream << "-";
        if (t.date().day() < 10) {
            stream << fill_zero[t.date().day()];
        } else {
            stream << t.date().day();
        }
        stream << "T";
        stream << t.time_of_day();
        return stream.str();
    }

    inline std::chrono::microseconds
    micros_from_date(const std::string &s) {
        using namespace std::chrono;
        using sys_microseconds = time_point<system_clock, microseconds>;
        sys_microseconds pt;
        std::istringstream is(s);
        is >> date::parse("%FT%T", pt);
        return pt.time_since_epoch();
    }

    //    cout << getISOCurrentTimestamp<chrono::seconds>();
    //    cout << getISOCurrentTimestamp<chrono::milliseconds>();
    //    cout << getISOCurrentTimestamp<chrono::microseconds>();
    template<class Precision>
    inline std::string getISOCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        return date::format("%FT%T", date::floor<Precision>(now));
    }

    inline uint64_t convert_str_time_to_uint64(const std::string &time) {
        return micros_from_date(time).count();
    }

    inline std::string format_time(time_t t) {
        return ctime(&t);
    }

//    inline time_t get_time_tstamp() {
//        auto start = std::chrono::high_resolution_clock::now();
//        time_t start_time = std::chrono::system_clock::to_time_t(start);
//        return start_time;
//    }

    //micro seconds
    inline uint64_t getMicroTstamp() {
        struct timeval tv{};
        gettimeofday(&tv, nullptr);
        return tv.tv_sec * (uint64_t) 1000000 + tv.tv_usec;
    }
}
