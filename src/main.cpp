#include "game/game_runner.hpp"
#include "network/network.hpp"
#include "shared/shared.hpp"
#include "thread_safe_queue/thread_safe_queue.hpp"
#include <iostream>
#include <sstream>
#include <thread>

int main() {
    ThreadSafeQueue<InputEventData *> inputQueue = ThreadSafeQueue<InputEventData *>();
    ThreadSafeQueue<UpdateToClients> clientUpdateQueue = ThreadSafeQueue<UpdateToClients>();

    std::thread networkThread(runNetworkLoop, &inputQueue, &clientUpdateQueue);
    std::thread gameRunnerThread(runGames, &inputQueue, &clientUpdateQueue);

    networkThread.join();
    gameRunnerThread.join();
    return 0;
}