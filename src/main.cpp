#include <stdio.h>
#include <errno.h>

#include "lib/socket/Bootstrap.h"
#include "lib/socket/Socket.h"
#include "lib/event/Event.h"
#include "lib/utility/Utility.h"

using namespace XCF;

Socket *client;
Log *logger = LogFactory::get();

void startServer(Log *logger) {
    ::logger->info("start server...");
    ServerBootstrap *server = new ServerBootstrap(SocketProtocol::TCP, "127.0.0.1", 10000);
    server->start();
}

static void stdinCallback(EventLoop *loop, EventWatcher *watcher, int revents) {
    if (EV_ERROR & revents) {
        ::logger->error("[ClientBootstrap] stdinCallback: got invalid event!");
        return;
    }

    char *buffer = NULL;
    size_t size = 2048;
    getline(&buffer, &size, stdin);

    std::string format(buffer);
    format.erase(std::remove(format.begin(), format.end(), '\n'), format.end());
    char *message = Utility::stringToChar(format);
    ::logger->info(Utility::stringFormat("[ClientBootstrap] stdinCallback: stdin written: %s", message));
    ::client->write(message);

    delete buffer;
    delete message;
}

static void clientCallback(EventLoop *loop, EventWatcher *watcher, int revents) {
    if (EV_ERROR & revents) {
        ::logger->error("[ClientBootstrap] clientCallback: got invalid event!");
        return;
    }

    int32_t socketFd = watcher->fd;

    SocketBuffer *buffer = new SocketBuffer();
    int32_t received = recv(socketFd, buffer->getBuffer(), SOCK_BUFFER_LENGTH, 0);

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

    ::client = new Socket("127.0.0.1", 10000, SocketProtocol::TCP, SocketEndType::CLIENT);
    if (::client->getSocketStatus() < SocketStatus::CONNECTED) {
        ::logger->error("failed to connect to server...");
        exit(1);
    }

    EventLoop *loop = ev_default_loop(0);

    EventWatcher *clientWatcher = (EventWatcher *) malloc(sizeof(EventWatcher));
    ev_io_init(clientWatcher, clientCallback, ::client->getSocketFd(), EV_READ);
    ev_io_start(loop, clientWatcher);

    EventWatcher *stdinWatcher = (EventWatcher *) malloc(sizeof(EventWatcher));
    ev_io_init(stdinWatcher, stdinCallback, /*STDIN_FILENO*/ 0, EV_READ);
    ev_io_start(loop, stdinWatcher);

    ev_run(loop, 0);
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
