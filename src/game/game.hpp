#ifndef GAME_HPP
#define GAME_HPP

#include "../shared/shared.hpp"
#include <list>
#include <unordered_map>
#include <vector>

class Game {
    int _width;
    int _height;

    std::vector<Cell> _cells;

    // wether the border is counted as dead cell or neutral
    bool _borderAlive;

    std::list<std::pair<size_t, Cell>> *_changedCells = nullptr;

    void setCellState(size_t index, std::vector<Cell> *cells);

public:
    Game(int width, int height, bool borderAlive = false);

    inline Cell cell(int x, int y, std::vector<Cell> *cells);

    inline int nbNeighborsAlive(int x, int y, std::vector<Cell> *cells);

    inline std::unordered_map<int, int> neighborsOwners(int x, int y, std::vector<Cell> *cells);

    // returns the index and the cell of each one who changed state
    std::list<std::pair<size_t, Cell>> *next();

    void setCellAlive(size_t index, int owner);

    void setCellDead(size_t index);

    void setCellStates(std::unordered_set<size_t> indexes, int owner, uint64_t tick); // TODO: manage ticks
};

#endif // GAME_HPP
