#include <iostream>
#include <spdlog/spdlog.h>
#include "src/streaming.h"
using namespace std;

int main() {
    spdlog::info("Build generated: {} {}", __DATE__, __TIME__);

    cout << "\n"
            "██████  ██████   ██████  ███    ██  ██████  ██   ██  ██████  ██████  ███    ██ \n"
            "██   ██ ██   ██ ██    ██ ████   ██ ██       ██   ██ ██    ██ ██   ██ ████   ██ \n"
            "██████  ██████  ██    ██ ██ ██  ██ ██   ███ ███████ ██    ██ ██████  ██ ██  ██ \n"
            "██      ██   ██ ██    ██ ██  ██ ██ ██    ██ ██   ██ ██    ██ ██   ██ ██  ██ ██ \n"
            "██      ██   ██  ██████  ██   ████  ██████  ██   ██  ██████  ██   ██ ██   ████ \n"
            "                                                                               \n";


    std::unique_ptr<Streaming> stream = std::make_unique<Streaming>();
    stream->start();

    sleep(1000);

    return 0;
}
