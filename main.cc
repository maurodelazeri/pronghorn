#include <iostream>
#include <spdlog/spdlog.h>
#include "src/streaming.h"
using namespace std;

int main() {
    cout << "\n"
            "██████  ██████   ██████  ███    ██  ██████  ██   ██  ██████  ██████  ███    ██ \n"
            "██   ██ ██   ██ ██    ██ ████   ██ ██       ██   ██ ██    ██ ██   ██ ████   ██ \n"
            "██████  ██████  ██    ██ ██ ██  ██ ██   ███ ███████ ██    ██ ██████  ██ ██  ██ \n"
            "██      ██   ██ ██    ██ ██  ██ ██ ██    ██ ██   ██ ██    ██ ██   ██ ██  ██ ██ \n"
            "██      ██   ██  ██████  ██   ████  ██████  ██   ██  ██████  ██   ██ ██   ████ \n"
            "                                                                               \n\n";


    spdlog::info("Build generated: {} {}", __DATE__, __TIME__);

    std::unique_ptr<Streaming> stream = std::make_unique<Streaming>();
    stream->start();

    return 0;
}
