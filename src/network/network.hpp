#ifndef NETWORKTHREAD_HPP
#define NETWORKTHREAD_HPP

#include "../shared/shared.hpp"
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

#include "../thread_safe_queue/thread_safe_queue.hpp"

class Network {
    ThreadSafeQueue<InputEventData *> *_inputQueue = nullptr;
    ThreadSafeQueue<UpdateToClients> *_clientUpdateQueue = nullptr;
    std::list<int> _clientSockets = {};

    int serverSocket;

    ssize_t sendCells(std::list<std::pair<size_t, Cell>> cells, int clientSocket);

    /**
     * returns true on failure
     */
    bool getClientInput(int clientSocket);

    /**
     * returns true on failure
     */
    bool getClientInputs();

    /**
     * returns true on failure
     */
    bool sendUpdateToClients();

public:
    Network(ThreadSafeQueue<InputEventData *> *inputQueue, ThreadSafeQueue<UpdateToClients> *clientUpdateQueue)
        : _inputQueue{inputQueue}, _clientUpdateQueue{clientUpdateQueue} {}

    ~Network();

    void run();
};

void runNetworkLoop(ThreadSafeQueue<InputEventData *> *inputQueue, ThreadSafeQueue<UpdateToClients> *clientUpdateQueue);

#endif // NETWORKTHREAD_HPP
