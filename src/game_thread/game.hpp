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
    std::vector<Cell> _oldCells;

    // wether the border is counted as dead cell or neutral
    bool _borderAlive;

    std::list<std::pair<size_t, Cell>> changedCells = {{}};

public:
    Game(int width, int height, bool borderAlive = false);

    inline Cell cell(int x, int y);

    inline int nbNeighborsAlive(int x, int y);

    inline std::unordered_map<int, int> neighborsOwners(int x, int y);

    // returns the index and the cell of each one who changed state
    std::list<std::pair<size_t, Cell>> next();

    void setCellAlive(int index, int owner);

    void setCellDead(int index);
};

#endif // GAME_HPP
