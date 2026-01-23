#ifndef GAME_RUNNER_HPP
#define GAME_RUNNER_HPP
#include "../shared/shared.hpp"
#include "../thread_safe_queue/thread_safe_queue.hpp"
#include "game.hpp"
#include <chrono>
#include <iostream>
#include <list>
#include <thread>
#include <algorithm>

struct GameWithPlayers {
    Game game;
    std::list<int> players;
};

class GameRunner {
    ThreadSafeQueue<InputEventData *> *_inputQueue;
    ThreadSafeQueue<UpdateToClients> *_clientUpdateQueue;
    std::list<GameWithPlayers> games = {};

public:
    GameRunner(ThreadSafeQueue<InputEventData *> *inputQueue, ThreadSafeQueue<UpdateToClients> *clientUpdateQueue);

    void run();
    void pullNewPlayers();
    void updateGames();
};

void runGames(ThreadSafeQueue<InputEventData *> *inputQueue, ThreadSafeQueue<UpdateToClients> *clientUpdateQueue);

#endif // GAME_RUNNER_HPP
