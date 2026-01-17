#include "game.hpp"

Game::Game(int width, int height, bool borderAlive)
    : _width{width}, _height{height}, _cells{std::vector<Cell>(width * height)}, _borderAlive{borderAlive} {}

inline Cell Game::cell(int x, int y) {
    if (x < 0 || x > _width - 1 || y < 0 || y > _height - 1) return Cell{_borderAlive ? NEUTRAL : DEAD};
    return _oldCells[y * _width + x];
}

inline int Game::nbNeighborsAlive(int x, int y) {
    return (cell(x + 1, y).owner != 0)
           + (cell(x - 1, y).owner != 0)
           + (cell(x, y + 1).owner != 0)
           + (cell(x, y - 1).owner != 0)
           + (cell(x - 1, y - 1).owner != 0)
           + (cell(x + 1, y - 1).owner != 0)
           + (cell(x - 1, y + 1).owner != 0)
           + (cell(x + 1, y + 1).owner != 0);
}

inline std::unordered_map<int, int> Game::neighborsOwners(int x, int y) {
    std::unordered_map<int, int> owners = std::unordered_map<int, int>();
    owners[cell(x + 1, y).owner] += 1;
    owners[cell(x - 1, y).owner] += 1;
    owners[cell(x, y + 1).owner] += 1;
    owners[cell(x, y - 1).owner] += 1;
    owners[cell(x - 1, y - 1).owner] += 1;
    owners[cell(x + 1, y - 1).owner] += 1;
    owners[cell(x - 1, y + 1).owner] += 1;
    owners[cell(x + 1, y + 1).owner] += 1;
    return owners;
}

std::list<std::pair<size_t, Cell>> Game::next() {
    _oldCells = _cells;

    for (size_t index = _oldCells.size() - 1; index > 0; index--) {
        int x, y;
        x = index % _width;
        y = index / _width;
        int nbAlive = nbNeighborsAlive(x, y);
        Cell cell = _oldCells[index];

        std::unordered_map<int, int> owners = neighborsOwners(x, y);
        owners.erase(NEUTRAL);
        if (cell.owner) { // alive
            if (nbAlive == 2 || nbAlive == 3) {
                if (cell.owner == NEUTRAL) {
                    if (owners.size() == 1) { // cell is neutral and a single player has cells around, he takes the cell
                        _cells[index].owner = owners.cbegin()->first;
                        changedCells.push_back({index, _cells[index]});
                    }
                }
                else if (owners.size() > 1) { // not neutral and multiple players around, it becomes neutral
                    _cells[index].owner = NEUTRAL;
                    changedCells.push_back({index, _cells[index]});
                }
            }
            else { // die
                _cells[index].owner = DEAD;
                changedCells.push_back({index, _cells[index]});
            }
        }
        else if (nbAlive == 3) {                                                        // birth
            _cells[index].owner = owners.size() > 1 ? NEUTRAL : owners.cbegin()->first; // if one player around, it's him, else it's neutral
            changedCells.push_back({index, _cells[index]});
        }
    }
    return changedCells;
}

void Game::setCellAlive(int index, int owner) { _cells[index].owner = owner; }

void Game::setCellDead(int index) { _cells[index].owner = DEAD; }
