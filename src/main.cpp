#include <stdio.h>
#include <errno.h>
#include <cstdio>

#include "XCF.h"

USING_NS_XCF;

Socket *client;

void startServer() {
    LogFactory::get()->info("start server...");
    ServerBootstrap *server = ServerBootstrap::init(SocketProtocol::TCP, "127.0.0.1", 10000);
    server->start();
}

static void stdinCallback(EventLoop *loop, EventIoWatcher *watcher, int revents) {
    if (EV_ERROR & revents) {
        LogFactory::get()->error("[ClientBootstrap] stdinCallback: got invalid event!");
        return;
    }

    char *buffer = NULL;
    size_t size = 2048;
    getline(&buffer, &size, stdin);

    // message
    Utility::stringTrimLineBreak(buffer);
    LogFactory::get()->info(Utility::stringFormat("[ClientBootstrap] stdinCallback: stdin written: %s", buffer));

    // send
    // we cannot get client socket from socket pool:
    // since this is the stdin callback, the fd is 0, we cannot find the client socket via this fd
    ::client->write(buffer);

    delete buffer;
}

static void clientCallback(EventLoop *loop, EventIoWatcher *watcher, int revents) {
    if (EV_ERROR & revents) {
        LogFactory::get()->error("[ClientBootstrap] clientCallback: got invalid event!");
        return;
    }

    int32_t socketFd = watcher->fd;

    Socket *client = ClientBootstrap::get()->getSocketPool()->getSocket(socketFd);
    SocketBuffer *buffer = new SocketBuffer();
    int32_t received = client->read(buffer, XCF_SOCK_BUFFER_LENGTH);

    if (received < 0) {
        // error
        return;
    }

    if (received == 0) {
        // stop the app if server socket closed
        LogFactory::get()->info("[ClientBootstrap] clientCallback: server closed, shutdown client!");
        exit(0);
    } else {
        LogFactory::get()->info(
            Utility::stringFormat("[ClientBootstrap] clientCallback: message got: %s", buffer->getBuffer())
        );
    }
    delete buffer;
}

void startClient() {
    LogFactory::get()->info("start client...");

    ::client = new Socket("127.0.0.1", 10000, SocketProtocol::TCP, SocketEndType::CLIENT);
    if (::client->getSocketStatus() < SocketStatus::CONNECTED) {
        LogFactory::get()->error("failed to connect to server...");
        exit(1);
    }

    ClientBootstrap *clientBoot = ClientBootstrap::init(SocketProtocol::TCP);

    // add stdin io watcher
    clientBoot->getEventIo()->addWatcher(0, stdinCallback);
    // add client socket io watcher
    clientBoot->getSocketPool()->addSocket(::client);
    clientBoot->getEventIo()->addWatcher(::client->getSocketFd(), clientCallback);

    clientBoot->start();
}

int main(int argc, char* argv[]) {
    std::string input;

    if (argc != 2) {
        input = "server";
    } else {
        input = argv[1];
    }

    LogFactory::get()->registerTimer();

    char string1[] = "XCF-Sample: ";
    SocketBuffer *buffer = new SocketBuffer();
    buffer->copyBuffer(string1);
    char string2[] = "main: ";
    buffer->copyBuffer(string2);
    LogFactory::get()->info(buffer->getBuffer());

    LogFactory::get()->info(Utility::stringFormat("CPU core number: %d", Utility::getCpuNum()));

    if (input == "server") {
        startServer();
    } else if (input == "client") {
        startClient();
    } else {
        LogFactory::get()->error("Invalid input!");
    }

    return 0;
}
