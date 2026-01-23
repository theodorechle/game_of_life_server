#include "game_runner.hpp"

GameRunner::GameRunner(ThreadSafeQueue<InputEventData *> *inputQueue, ThreadSafeQueue<UpdateToClients> *clientUpdateQueue)
    : _inputQueue{inputQueue}, _clientUpdateQueue{clientUpdateQueue} {}

void GameRunner::pullInputQueue() {
    InputEventData *event;

    while (_inputQueue->tryPop(&event)) {
        std::cerr << "new event: " << inputEventToString(event->event) << " (from client: " << event->client << ")\n";
        switch (event->event) {
        case InputEvent::ADD_CLIENT:
            if (games.empty() || games.back().players.size() == 4) games.push_back(GameWithPlayers{Game(10, 10), {event->client}});
            else games.back().players.push_back(event->client);
            break;
        case InputEvent::REMOVE_CLIENT: {
            for (std::list<GameWithPlayers>::iterator game = games.begin(); game != games.end(); game++) {
                std::list<int>::const_iterator user = std::find(game->players.cbegin(), game->players.cend(), event->client);
                if (user != game->players.cend()) {
                    if (game->players.size() == 1) games.erase(game);
                    else game->players.erase(user);
                    break;
                }
            }
            break;
        }
        case InputEvent::TOGGLE_CELLS: {
            ToggleCellsInputEventData *toggleCellsEvent = static_cast<ToggleCellsInputEventData *>(event);
            for (std::list<GameWithPlayers>::iterator game = games.begin(); game != games.end(); game++) {
                std::list<int>::const_iterator user = std::find(game->players.cbegin(), game->players.cend(), event->client);
                if (user != game->players.cend()) {
                    game->game.setCellStates(toggleCellsEvent->indexes, toggleCellsEvent->client, toggleCellsEvent->tick);
                }
            }
            break;
        }
        default:
            break;
        }
        delete event;
    }
}

void GameRunner::updateGames() {
    for (GameWithPlayers &game : games) {
        std::cerr << "updating game\n";
        _clientUpdateQueue->push(UpdateToClients{game.players, game.game.next()});
    }
}

void GameRunner::run() {
    std::chrono::milliseconds tick = std::chrono::milliseconds(500);
    std::chrono::steady_clock::time_point next = std::chrono::steady_clock::now();

    while (threadsRunning) {
        pullInputQueue();
        updateGames();
        next += tick;
        std::this_thread::sleep_until(next);
    }
    threadsRunning = false;
}

void runGames(ThreadSafeQueue<InputEventData *> *inputQueue, ThreadSafeQueue<UpdateToClients> *clientUpdateQueue) {
    GameRunner(inputQueue, clientUpdateQueue).run();
}
