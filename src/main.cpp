#include "lib/log/Log.h"

using namespace XCF;

int main(int argc, char** argv) {
    Log *logger = LogFactory::get(LogType::SysLog);

    logger->setPriority(LogPriority::Info);

    logger->debug("debug message!");
    logger->info("info message!");
    logger->warn("warning message!");
    logger->error("error message!");

    logger->output();

    return 0;
}
