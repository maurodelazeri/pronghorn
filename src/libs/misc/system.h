//
// Created by mauro on 5/20/20.
//

#pragma once

#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>

namespace utils {
    inline std::string exec(const char* cmd) {
        char buffer[128];
        std::string result;
        FILE* pipe = popen(cmd, "r");
        if (!pipe) throw std::runtime_error("popen() failed!");
        try {
            while (fgets(buffer, sizeof buffer, pipe) != NULL) {
                result += buffer;
            }
        } catch (...) {
            pclose(pipe);
            throw;
        }
        pclose(pipe);
        return result;
    }

    inline long parseLineStatus(char *line) {
        // This assumes that a digit will be found and the line ends in " Kb".
        size_t i = strlen(line);
        const char *p = line;
        while (*p < '0' || *p > '9') {
            p++;
        }
        line[i - 3] = '\0';
        long val = strtol(p, nullptr, 10);
        return val;
    }

    inline long getRamMemoryUsage() { //Note: this value is in KB!
        FILE *file = fopen("/proc/self/status", "r");
        long result = -1;
        char line[128];

        while (fgets(line, 128, file) != nullptr) {
            if (strncmp(line, "VmRSS:", 6) == 0) {
                result = parseLineStatus(line);
                break;
            }
        }
        fclose(file);
        return result;
    }

    inline long getVirMemoryUsage() { //Note: this value is in KB!
        FILE *file = fopen("/proc/self/status", "r");
        long result = -1;
        char line[128];

        while (fgets(line, 128, file) != nullptr) {
            if (strncmp(line, "VmSize:", 7) == 0) {
                result = parseLineStatus(line);
                break;
            }
        }
        fclose(file);
        return result;
    }
}