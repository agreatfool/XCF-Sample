#include "XCF.h"
#include "lib/socket/Bootstrap.h"
#include "lib/socket/Socket.h"
#include "lib/socket/SocketBuffer.h"

using namespace XCF;

void startServer(Log *logger) {
    logger->info("start server...");
    ServerBootstrap *server = new ServerBootstrap(SocketProtocol::TCP, "127.0.0.1", 10000);
    server->start();
}

void startClient(Log *logger) {
    logger->info("start client...");

    char buffer[SOCK_BUFFER_LENGTH] = "";

    Socket *connection = new Socket("127.0.0.1", 10000, SocketProtocol::TCP, SocketEndType::CLIENT);
    connection->socketInit();

    while (std::cin >> buffer) {
        SocketBuffer *buff = new SocketBuffer(buffer);
        connection->socketWrite(buff);
        buff->clearBuffer();

        connection->socketRead(buff, SOCK_BUFFER_LENGTH);
        logger->info("response:");
        logger->info(buff->getBuffer());
    }
}

int main(int argc, char* argv[]) {
    Log *logger = LogFactory::get();

    std::string input;

    if (argc != 2) {
        input = "client";
    } else {
        input = argv[1];
    }

    logger->info(Utility::stringFormat("data: %s", "this is data"));

    char string1[] = "this is the first part!";
    SocketBuffer *buffer = new SocketBuffer();
    buffer->copyBuffer(string1);

    char string2[] = ", this shall be the part two!";
    buffer->copyBuffer(string2);

    logger->error(buffer->getBuffer());

    if (input == "server") {
        startServer(logger);
    } else if (input == "client") {
        startClient(logger);
    } else {
        logger->error("Invalid input!");
    }

    return 0;
}
