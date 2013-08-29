#include <errno.h>

#include "lib/socket/Bootstrap.h"
#include "lib/socket/Socket.h"
#include "lib/event/Event.h"
#include "lib/utility/Utility.h"

using namespace XCF;

Log *logger = LogFactory::get();

void startServer(Log *logger) {
    ::logger->info("start server...");
    ServerBootstrap *server = new ServerBootstrap(SocketProtocol::TCP, "127.0.0.1", 10000);
    server->start();
}

static void clientCallback(EventLoop *loop, EventWatcher *watcher, int revents) {
    if (EV_ERROR & revents) {
        ::logger->error("[ClientBootstrap] clientCallback: got invalid event!");
        return;
    }

    int32_t socketFd = watcher->fd;
    Socket *socket = ClientBootstrap::socketPool->getSocket(socketFd);
    if (Utility::isNullPtr(socket)) {
        ::logger->error("[ClientBootstrap] clientCallback: socket not found!");
        return;
    }

    SocketBuffer *buffer = new SocketBuffer();
    int32_t received = socket->read(buffer, SOCK_BUFFER_LENGTH);

    if (received < 0) {
        // error
        return;
    }

    if (received == 0) {
        // stop the app if server socket closed
        ::logger->info("[ClientBootstrap] clientCallback: server closed, shutdown client!");
        exit(0);
    } else {
        ::logger->info(
            Utility::stringFormat("[ClientBootstrap] clientCallback: message got: %s", buffer->getBuffer())
        );
    }
    delete buffer;
}

void startClient(Log *logger) {
    ::logger->info("start client...");

    char buffer[SOCK_BUFFER_LENGTH] = "";

    Socket *connection = new Socket("127.0.0.1", 10000, SocketProtocol::TCP, SocketEndType::CLIENT);
    if (connection->getSocketStatus() < SocketStatus::CONNECTED) {
        ::logger->error("failed to connect to server...");
        exit(1);
    }

    ClientBootstrap *client = new ClientBootstrap(SocketProtocol::TCP);
    ClientBootstrap::eventIo->addWatcher(connection->getSocketFd(), clientCallback);
    client->start();

    while (std::cin >> buffer) {
        connection->write(buffer);
    }
}

int main(int argc, char* argv[]) {
    std::string input;

    if (argc != 2) {
        input = "client";
    } else {
        input = argv[1];
    }

    char string1[] = "XCF-Sample: ";
    SocketBuffer *buffer = new SocketBuffer();
    buffer->copyBuffer(string1);
    char string2[] = "main: ";
    buffer->copyBuffer(string2);
    ::logger->info(buffer->getBuffer());

    if (input == "server") {
        startServer(logger);
    } else if (input == "client") {
        startClient(logger);
    } else {
        ::logger->error("Invalid input!");
    }

    return 0;
}
