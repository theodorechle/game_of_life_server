#include "network.hpp"

ssize_t Network::sendCells(int clientSocket, std::unordered_map<size_t, Cell> *cells, uint64_t tick) {
    size_t contentLength = sizeof(char) * cells->size();
    std::stringstream answer;

    answer << contentLength << "\n" << tick << "\n";

    for (std::pair<size_t, Cell> cell : *cells) {
        answer << cell.first << "\n" << cell.second.owner << "\n";
    }
    std::string answerStr = answer.str();

    std::cerr << "sent data: '" << answerStr << ", size=" << answerStr.size() << "'\n";

    return send(clientSocket, answerStr.c_str(), answerStr.size(), MSG_NOSIGNAL); // MSG_NOSIGNAL ignore SIGPIPE (still return the error though)
}

int Network::getClientInput(int clientSocket, NetworkInputHandler *networkInputHandler) {
    std::string data;

    int errorCode = networkInputHandler->readUntilDelimiter('\n', data, false, true);

    if (errorCode) {
#ifdef DEBUG
        std::cerr << "waiting messages? " << (errno == EAGAIN || errno == EWOULDBLOCK) << "\n";
#endif
        return errno != EAGAIN && errno != EWOULDBLOCK;
    }

    size_t contentLength = atoi(data.c_str());

    std::string date;
    errorCode = networkInputHandler->readUntilDelimiter('\n', date, false, true);

    if (errorCode) {
#ifdef DEBUG
        std::cerr << "waiting messages? " << (errno == EAGAIN || errno == EWOULDBLOCK) << "\n";
#endif
        return errno != EAGAIN && errno != EWOULDBLOCK;
    }

    uint64_t tick = atoi(date.c_str());
    std::cerr << "contentLength: " << contentLength << "\n";

    std::unordered_set<size_t> indexes = {};

    for (; contentLength > 0; contentLength--) {
        errorCode = networkInputHandler->readUntilDelimiter('\n', data, false, true); // FIXME: should be blocking, to be sure the message is received
        if (errorCode) return errorCode;
        size_t pos = static_cast<size_t>(atoi(data.c_str()));
        indexes.insert(pos);
    }

    _inputQueue->push(new ToggleCellsInputEventData{clientSocket, InputEvent::TOGGLE_CELLS, tick, indexes});

    return 0;
}

bool Network::getClientInputs() {
    for (std::map<int, NetworkInputHandler>::iterator clientSocket = _clientSockets.begin(); clientSocket != _clientSockets.end();) {
        int errorCode = getClientInput(clientSocket->first, &(clientSocket->second));
        if (errorCode == 1) return true;
        if (errorCode == 2) {
            _inputQueue->push(new InputEventData{clientSocket->first, InputEvent::REMOVE_CLIENT});
            clientSocket = _clientSockets.erase(clientSocket);
        }
        else clientSocket++;
    }
    return false;
}

bool Network::sendUpdateToClients() {
    UpdateToClients update;
    while (_clientUpdateQueue->tryPop(&update)) {
        std::cerr << "nb clients: " << update.clients.size() << ", nb updated cells: " << update.updatedCells->size() << "\n";
        for (int client : update.clients) {
            std::cerr << "update to client: " << client << "\n";
            if (sendCells(client, update.updatedCells, update.tick) == -1) {
                std::cerr << "error: " << errno << "\n";
                _inputQueue->push(new InputEventData{client, InputEvent::REMOVE_CLIENT});
                if (errno != EPIPE && errno != ECONNRESET) {
                    delete update.updatedCells;
                    return true;
                }
            }
        }
        delete update.updatedCells;
    }
    return false;
}

Network::Network(ThreadSafeQueue<InputEventData *> *inputQueue, ThreadSafeQueue<UpdateToClients> *clientUpdateQueue)
    : _inputQueue{inputQueue}, _clientUpdateQueue{clientUpdateQueue} {
    _serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt,
               sizeof(opt)); // allow reusing same adress on restart (else, the OS will keep it used for a while)

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(_serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        throw NetworkException("bind failed: " + std::string(strerror(errno)));
        return;
    }

    if (listen(_serverSocket, 15) == -1) { // TODO: const
        throw NetworkException("listen failed: " + std::string(strerror(errno)));
        return;
    }

    int flags = fcntl(_serverSocket, F_GETFL, 0);
    if (flags == -1) {
        throw NetworkException("fcntl get failed: " + std::string(strerror(errno)));
        return;
    }

    // set I/O to non-blocking
    if (fcntl(_serverSocket, F_SETFL, flags | O_NONBLOCK) == -1) {
        throw NetworkException("fcntl set failed: " + std::string(strerror(errno)));
        return;
    }
}

Network::~Network() {
    close(_serverSocket);
    threadsRunning = false;
}

void Network::run() {
    while (threadsRunning) {
        std::chrono::milliseconds tick = std::chrono::milliseconds(50);
        std::chrono::steady_clock::time_point next = std::chrono::steady_clock::now();

        int clientSocket = accept(_serverSocket, nullptr, nullptr);
        if (clientSocket == -1) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("accept failed");
                return;
            }
        }
        else {
            int flags = fcntl(clientSocket, F_GETFL, 0);
            if (flags == -1) {
                perror("fcntl on client get failed");
                return;
            }

            // set I/O to non-blocking
            if (fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK) == -1) {
                perror("fcntl on client set failed");
                return;
            }
            std::cerr << "accepted\n";
            _clientSockets.insert({clientSocket, NetworkInputHandler(clientSocket)});
            _inputQueue->push(new InputEventData{clientSocket, InputEvent::ADD_CLIENT});
        }

        if (getClientInputs()) {
            perror("get client input failed");
            return;
        }

        if (sendUpdateToClients()) {
            perror("sendCells failed");
            return;
        }

        next += tick;
        std::this_thread::sleep_until(next);
    }
}

void runNetworkLoop(ThreadSafeQueue<InputEventData *> *inputQueue, ThreadSafeQueue<UpdateToClients> *clientUpdateQueue) {
    try {
        Network(inputQueue, clientUpdateQueue).run();
    }
    catch (NetworkException &e) {
        std::cerr << "network exception: " << e.what() << "\n";
    }
}
