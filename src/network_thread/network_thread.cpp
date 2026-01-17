#include "network_thread.hpp"

ssize_t Network::sendCells(std::list<std::pair<size_t, Cell>> cells, int clientSocket) {
    size_t contentLength = sizeof(char) * cells.size();
    std::stringstream answer;

    answer << contentLength << "\n";

    for (std::pair<size_t, Cell> cell : cells) {
        answer << cell.first << "\n" << cell.second.owner << "\n";
    }
    std::string answerStr = answer.str();

    std::cerr << "sent data: '" << answerStr << ", size=" << answerStr.size() << "'\n";

    return send(clientSocket, answerStr.c_str(), answerStr.size(), MSG_NOSIGNAL);
}

bool Network::getClientInput(int clientSocket) {
    size_t BUFFER_SIZE = 1024; // TODO: move it elsewhere

    char buffer[BUFFER_SIZE] = {0};

    ssize_t errorCode = recv(clientSocket, buffer, BUFFER_SIZE, 0);

    if (errorCode == -1) return errno != EAGAIN && errno != EWOULDBLOCK; // if false, client haven't sent anything (non-blocking mode)
    if (errorCode == 0) return true;                                     // client disconnected

    size_t cellToggled = atoi(buffer);
    std::cerr << "cellToggled: " << cellToggled << "\n";
    _inputQueue->push(new ToggleCellInputEventData{clientSocket, InputEvent::TOGGLED_CELL, cellToggled});
    return false;
}

bool Network::getClientInputs() {
    for (int clientSocket : _clientSockets) {
        if (getClientInput(clientSocket)) return true;
    }
    return false;
}

bool Network::sendUpdateToClients() {
    UpdateToClients update;
    while (_clientUpdateQueue->tryPop(&update)) {
        for (int client : update.clients) {
            std::cerr << "update to client: " << client << "\n";
            if (sendCells(update.updatedCells, client) == -1) {
                std::cerr << "error: " << errno << "\n";
                _inputQueue->push(new InputEventData{client, InputEvent::REMOVE_CLIENT});
                if (errno != EPIPE) {
                    return true;
                }
            }
        }
    }
    return false;
}

Network::~Network() {
    close(serverSocket);
    threadsRunning = false;
}

void Network::run() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt,
               sizeof(opt)); // allow reusing same adress on restart (else, the OS will keep it used for a while)

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("bind failed");
        return;
    }

    if (listen(serverSocket, 15) == -1) {
        perror("listen failed");
        return;
    }

    int flags = fcntl(serverSocket, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl get failed");
        return;
    }

    // set I/O to non-blocking
    if (fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl set failed");
        return;
    }

    bool listenNewConnections = true;
    while (threadsRunning) {
        std::chrono::milliseconds tick = std::chrono::milliseconds(500);
        std::chrono::steady_clock::time_point next = std::chrono::steady_clock::now();

        if (listenNewConnections) {

            int clientSocket = accept(serverSocket, nullptr, nullptr);
            if (clientSocket == -1) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("accept failed");
                    return;
                }
            }
            else {
                std::cerr << "accepted\n";
                _clientSockets.push_back(clientSocket);
                _inputQueue->push(new InputEventData{clientSocket, InputEvent::ADD_CLIENT});
            }
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
    Network(inputQueue, clientUpdateQueue).run();
}
