//
// Created by mauro on 5/20/20.
//

#pragma once

#include <boost/regex.hpp>
#include <boost/date_time.hpp>
#include <stdio.h>      /* printf */
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */

namespace utils {

    inline char *buildCharFromString(std::string &str) {
        char *result_return = static_cast<char *>(malloc(sizeof(char) * (strlen(str.c_str()) + 1)));
        if (result_return == nullptr) exit(1);
        strcpy(result_return, str.c_str());
        return result_return;
    }

    template<typename T>
    inline std::string vector_to_string_with_comma(T begin, T end) {
        std::stringstream ss;
        bool first = true;
        for (; begin != end; begin++) {
            if (!first)
                ss << ", ";
            ss << "'" << *begin << "'";
            first = false;
        }
        return ss.str();
    }

    template<typename T>
    inline std::string to_string_with_precision(const T a_value, const int n = 8) {
        std::ostringstream out;
        out.precision(n);
        out << std::fixed << a_value;
        return out.str();
    }

/*
std::string split implementation by using delimeter as a character.
*/
    inline std::vector<std::string> split(const std::string &strToSplit, char delimeter) {
        std::stringstream ss(strToSplit);
        std::string item;
        std::vector<std::string> splittedStrings;
        while (std::getline(ss, item, delimeter)) {
            splittedStrings.push_back(item);
        }
        return splittedStrings;
    }

/*
std::string split implementation by using delimeter as an another string
*/
    inline std::vector<std::string> split(const std::string &stringToBeSplitted, const std::string &delimeter) {
        std::vector<std::string> splittedString;
        unsigned long startIndex = 0;
        unsigned long endIndex = 0;
        while ((endIndex = stringToBeSplitted.find(delimeter, startIndex)) < stringToBeSplitted.size()) {

            std::string val = stringToBeSplitted.substr(startIndex, endIndex - startIndex);
            splittedString.push_back(val);
            startIndex = endIndex + delimeter.size();

        }
        if (startIndex < stringToBeSplitted.size()) {
            std::string val = stringToBeSplitted.substr(startIndex);
            splittedString.push_back(val);
        }
        return splittedString;
    }

    inline std::string randomString(uint maxLength = 15,
                                    std::string charIndex = "abcdefghijklmnaoqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890") { // maxLength and charIndex can be customized, but I've also initialized them.
        uint length =
                rand() % maxLength + 1; // length of the string is a random value that can be up to 'l' characters.

        uint indexesOfRandomChars[15]; // array of random values that will be used to iterate through random indexes of 'charIndex'
        for (uint i = 0; i < length; ++i) // assigns a random number to each index of "indexesOfRandomChars"
            indexesOfRandomChars[i] = rand() % charIndex.length();

        std::string randomString = ""; // random string that will be returned by this function
        for (uint i = 0; i < length; ++i)// appends a random amount of random characters to "randomString"
        {
            randomString += charIndex[indexesOfRandomChars[i]];
        }
        return randomString;
    }

    inline std::string getEnvVar(std::string const &key) {
        char const *val = getenv(key.c_str());
        return val == NULL ? std::string() : std::string(val);
    }

    inline void findAndReplaceAll(std::string &data, std::string toSearch, std::string replaceStr) {
        // Get the first occurrence
        size_t pos = data.find(toSearch);

        // Repeat till end is reached
        while (pos != std::string::npos) {
            // Replace this occurrence of Sub String
            data.replace(pos, toSearch.size(), replaceStr);
            // Get the next occurrence from the current positions
            pos = data.find(toSearch, pos + replaceStr.size());
        }
    }


    inline const std::string StringFormat(const char *format, ...) {
        va_list args;
        va_start(args, format);

        std::string output;
        va_list tmpargs;

        va_copy(tmpargs, args);
        int characters_used = vsnprintf(nullptr, 0, format, tmpargs);
        va_end(tmpargs);

        if (characters_used > 0) {
            output.resize(characters_used + 1);

            va_copy(tmpargs, args);
            characters_used = vsnprintf(&output[0], output.capacity(), format, tmpargs);
            va_end(tmpargs);

            output.resize(characters_used);

            if (characters_used < 0)
                output.clear();
        }
        return output;
    }

    inline const bool validate_email(const std::string &a) {
        static const boost::regex e("^[_a-z0-9-]+(\\.[_a-z0-9-]+)*@[a-z0-9-]+(\\.[a-z0-9-]+)*(\\.[a-z]{2,4})$");
        return boost::regex_match(a, e);
    }

    inline const std::string jsonFromDocument(const rapidjson::Document &document) {
        rapidjson::StringBuffer sb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
        document.Accept(writer);
        return sb.GetString();
    }
}
