#ifndef NETWORKTHREAD_HPP
#define NETWORKTHREAD_HPP

#include "../shared/shared.hpp"
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>

#include "../../game_of_life_commons/src/network_input_handler/network_input_handler.hpp"
#include "../thread_safe_queue/thread_safe_queue.hpp"
#include "network_exception.hpp"

class Network {
    ThreadSafeQueue<InputEventData *> *_inputQueue = nullptr;
    ThreadSafeQueue<UpdateToClients> *_clientUpdateQueue = nullptr;
    std::map<int, NetworkInputHandler> _clientSockets = {};

    int _serverSocket;

    ssize_t sendCells(int clientSocket, std::unordered_map<size_t, Cell> *cells, uint64_t tick);

    /**
     * returns 0 on success, 1 on error and 2 on client disconnection
     */
    int getClientInput(int clientSocket, NetworkInputHandler *networkInputHandler);

    /**
     * returns true on failure
     */
    bool getClientInputs();

    /**
     * returns true on failure
     */
    bool sendUpdateToClients();

public:
    Network(ThreadSafeQueue<InputEventData *> *inputQueue, ThreadSafeQueue<UpdateToClients> *clientUpdateQueue);

    ~Network();

    void run();
};

void runNetworkLoop(ThreadSafeQueue<InputEventData *> *inputQueue, ThreadSafeQueue<UpdateToClients> *clientUpdateQueue);

#endif // NETWORKTHREAD_HPP
