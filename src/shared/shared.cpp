#include "shared.hpp"

std::string inputEventToString(InputEvent &event) {
    switch (event) {
    case InputEvent::ADD_CLIENT:
        return "add client";
    case InputEvent::REMOVE_CLIENT:
        return "remove client";
    case InputEvent::TOGGLED_CELL:
        return "toggle cell";
    default:
        return "unknown type";
    }
}

std::atomic<bool> threadsRunning{true};

void sigintHandler(int) { threadsRunning = false; }
