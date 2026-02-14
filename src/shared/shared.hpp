#ifndef SHARED_HPP
#define SHARED_HPP

#include <atomic>
#include <list>
#include <signal.h>
#include <string>
#include <unordered_set>
#include <unordered_map>

constexpr int DEAD = 0;
constexpr int NEUTRAL = 1;
constexpr int USER = 2;
// max size is 9 (7 users)
// more will need send protocol to change

struct Cell {
    // a cell, can be dead, neutral (nobody owns it, but it's alive), or owned by a player (>=USER)
    int owner = DEAD;
};

typedef std::unordered_map<size_t, Cell> UpdatedCells;

struct UpdateToClients {
    std::list<int> clients;
    // pair of alive, player who owned it, must be freed by caller
    UpdatedCells *updatedCells;
    uint64_t tick;
};

enum class InputEvent { ADD_CLIENT, REMOVE_CLIENT, TOGGLE_CELLS };

std::string inputEventToString(InputEvent &event);

struct InputEventData {
    int client;
    InputEvent event;
    uint64_t tick;
};

struct ToggleCellsInputEventData : InputEventData {
    std::unordered_set<size_t> indexes;
};

extern std::atomic<bool> threadsRunning;

void sigintHandler(int);

#endif // SHARED_HPP
