#include "XCF.h"

using namespace XCF;

int main(int argc, char** argv) {

    Log* logger = LogFactory::get();

    logger->debug("123");
    logger->debug("12333");

    logger->output();

    return 0;
}
