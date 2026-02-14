#include "game.hpp"
#include <iostream>

void Game::setCellState(size_t index, std::vector<Cell> *cells) {
    int x = index % _width;
    int y = index / _width;
    Cell cell = (*cells)[index];

    int nbAlive = nbNeighborsAlive(x, y, cells);

    std::unordered_map<int, int> owners = neighborsOwners(x, y, cells);
    owners.erase(NEUTRAL);
    owners.erase(DEAD);

    if (cell.owner) { // alive
        std::cerr << index << ": ";
        if (nbAlive == 2 || nbAlive == 3) {
            if (cell.owner == NEUTRAL) {
                if (owners.size() == 1) { // cell is neutral and a single player has cells around, he takes the cell
                    _cells[index].owner = owners.cbegin()->first;
                    _changedCells->insert_or_assign(index, _cells[index]);
                    std::cerr << owners.cbegin()->first << " is taking\n";
                }
            }
            else if (owners.size() > 1) { // not neutral and multiple players around, it becomes neutral
                _cells[index].owner = NEUTRAL;
                _changedCells->insert_or_assign(index, _cells[index]);
                std::cerr << "now neutral\n";
            }
        }
        else { // die
            _cells[index].owner = DEAD;
            _changedCells->insert_or_assign(index, _cells[index]);
            std::cerr << "now dead\n";
        }
    }
    else if (nbAlive == 3) {                                                        // birth
        _cells[index].owner = owners.size() > 1 ? NEUTRAL : owners.cbegin()->first; // if one player around, it's him, else it's neutral
        std::cerr << index << ": " << "now alive, taken by " << _cells[index].owner << "\n";
        _changedCells->insert_or_assign(index, _cells[index]);
    }
}

Game::Game(int width, int height, bool borderAlive)
    : _width{width}, _height{height}, _cells{std::vector<Cell>(width * height)}, _borderAlive{borderAlive},
      _changedCells{new std::unordered_map<size_t, Cell>()} {}

inline Cell Game::cell(int x, int y, std::vector<Cell> *cells) {
    if (x < 0 || x > _width - 1 || y < 0 || y > _height - 1) return Cell{_borderAlive ? NEUTRAL : DEAD};
    return (*cells)[y * _width + x];
}

inline int Game::nbNeighborsAlive(int x, int y, std::vector<Cell> *cells) {
    return (cell(x + 1, y, cells).owner != 0)
           + (cell(x - 1, y, cells).owner != 0)
           + (cell(x, y + 1, cells).owner != 0)
           + (cell(x, y - 1, cells).owner != 0)
           + (cell(x - 1, y - 1, cells).owner != 0)
           + (cell(x + 1, y - 1, cells).owner != 0)
           + (cell(x - 1, y + 1, cells).owner != 0)
           + (cell(x + 1, y + 1, cells).owner != 0);
}

inline std::unordered_map<int, int> Game::neighborsOwners(int x, int y, std::vector<Cell> *cells) {
    std::unordered_map<int, int> owners = std::unordered_map<int, int>();
    owners[cell(x + 1, y, cells).owner] += 1;
    owners[cell(x - 1, y, cells).owner] += 1;
    owners[cell(x, y + 1, cells).owner] += 1;
    owners[cell(x, y - 1, cells).owner] += 1;
    owners[cell(x - 1, y - 1, cells).owner] += 1;
    owners[cell(x + 1, y - 1, cells).owner] += 1;
    owners[cell(x - 1, y + 1, cells).owner] += 1;
    owners[cell(x + 1, y + 1, cells).owner] += 1;
    return owners;
}

std::pair<uint64_t, UpdatedCells *> Game::next() {
    std::vector<Cell> oldCells = _cells;

    for (size_t index = oldCells.size() - 1; index > 0; index--) {
        setCellState(index, &oldCells);
    }

    UpdatedCells *changedCells = _changedCells;
    _changedCells = new UpdatedCells();

    uint64_t lastTick = _tick;

    _tick++;

    return {lastTick, changedCells};
}

void Game::setCellAlive(size_t index, int owner) { _cells[index].owner = owner; }

void Game::setCellDead(size_t index) { _cells[index].owner = DEAD; }

void Game::setCellsState(std::unordered_set<size_t> indexes, int owner, uint64_t tick) {
    if (tick != _tick) return;
    for (size_t index : indexes) {
        if (_cells[index].owner == owner) _cells[index].owner = DEAD;
        else _cells[index].owner = owner;
        std::cerr << (index % _width) << ", " << (index / _width) << ": alive? " << (_cells[index].owner == DEAD ? "no" : "yes") << "\n";
        _changedCells->insert({index, _cells[index]});
    }
}
