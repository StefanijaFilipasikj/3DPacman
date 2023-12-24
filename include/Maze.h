#ifndef OPENGLPRJ_MAZE_H
#define OPENGLPRJ_MAZE_H

#include <iostream>
#include <vector>
#include <queue>
#include <ctime>
#include <cstdlib>
#include "Cell.h"

using namespace std;

const int rows = 10;
const int cols = 10;

vector<vector<Cell>> grid(rows, vector<Cell>(cols));

class Maze {
public:
    Maze(){
        initializeGrid();
    }
    void initializeGrid() {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                grid[i][j].row = i;
                grid[i][j].col = j;
                grid[i][j].visited = false;
                grid[i][j].wallUp = true;
                grid[i][j].wallDown = true;
                grid[i][j].wallLeft = true;
                grid[i][j].wallRight = true;
                grid[i][j].hasCoin = true;
                grid[i][j].hasPellet = false;
            }
        }

        //add pellets
        grid[0][0].hasPellet = true;
        grid[0][cols-1].hasPellet = true;
        grid[rows-1][0].hasPellet = true;
        grid[rows-1][cols-1].hasPellet = true;
        grid[2][2].hasPellet = true;
        grid[2][cols-3].hasPellet = true;
        grid[rows-3][2].hasPellet = true;
        grid[rows-3][cols-3].hasPellet = true;

        //delete coins on cells that have pellets
        grid[0][0].hasCoin = false;
        grid[0][cols-1].hasCoin = false;
        grid[rows-1][0].hasCoin = false;
        grid[rows-1][cols-1].hasCoin = false;
        grid[2][2].hasCoin = false;
        grid[2][cols-3].hasCoin = false;
        grid[rows-3][2].hasCoin = false;
        grid[rows-3][cols-3].hasCoin = false;
    }

    bool isValid(int row, int col) {
        return (row >= 0 && row < rows && col >= 0 && col < cols && !grid[row][col].visited);
    }

    bool isVisited(int row, int col){
        return (row >= 0 && row < rows && col >= 0 && col < cols && grid[row][col].visited);
    }

    bool contains(vector<pair<int, int>> walls, int row, int col){
        for(pair<int,int> wall: walls)
            if(wall.first==row && wall.second==col)
                return true;
        return false;
    }

    void generateMaze() {
        srand(time(0));

        int startRow = rand() % rows;
        int startCol = rand() % cols;

        grid[startRow][startCol].visited = true;
        vector<pair<int, int>> walls;

        if (isValid(startRow - 1, startCol)) walls.push_back(make_pair(startRow - 1, startCol));
        if (isValid(startRow + 1, startCol)) walls.push_back(make_pair(startRow + 1, startCol));
        if (isValid(startRow, startCol - 1)) walls.push_back(make_pair(startRow, startCol - 1));
        if (isValid(startRow, startCol + 1)) walls.push_back(make_pair(startRow, startCol + 1));

        while (!walls.empty()) {
            //choose a wall
            int randomIndex = rand() % walls.size();
            int currentRow = walls[randomIndex].first;
            int currentCol = walls[randomIndex].second;
            grid[currentRow][currentCol].visited = true;

            //for choosing the visited wall
            vector<pair<int, int>> neighbors;

            //add all walls that you can connect to
            if (isVisited(currentRow - 1, currentCol)) neighbors.push_back(make_pair(currentRow - 1, currentCol));
            if (isVisited(currentRow + 1, currentCol)) neighbors.push_back(make_pair(currentRow + 1, currentCol));
            if (isVisited(currentRow, currentCol - 1)) neighbors.push_back(make_pair(currentRow, currentCol - 1));
            if (isVisited(currentRow, currentCol + 1)) neighbors.push_back(make_pair(currentRow, currentCol + 1));

            //pick a random wall to connect to
            if (!neighbors.empty()) {
                //randomly connect to 2 walls
                int br = rand() % 100;
                if(br >= 30){
                    if(neighbors.size() > 1){
                        for(int i=0;i<2;i++){
                            int randomNeighborIndex = rand() % neighbors.size();
                            int neighborRow = neighbors[randomNeighborIndex].first;
                            int neighborCol = neighbors[randomNeighborIndex].second;

                            addNeighbor(currentRow, currentCol, neighborRow, neighborCol);
                            neighbors.erase(neighbors.begin() + randomNeighborIndex);
                        }
                    }else{
                        int randomNeighborIndex = rand() % neighbors.size();
                        int neighborRow = neighbors[randomNeighborIndex].first;
                        int neighborCol = neighbors[randomNeighborIndex].second;

                        addNeighbor(currentRow, currentCol, neighborRow, neighborCol);
                    }
                }else{
                    int randomNeighborIndex = rand() % neighbors.size();
                    int neighborRow = neighbors[randomNeighborIndex].first;
                    int neighborCol = neighbors[randomNeighborIndex].second;

                    addNeighbor(currentRow, currentCol, neighborRow, neighborCol);
                }
            }
            walls.erase(walls.begin() + randomIndex);

            //add all new unvisited walls
            if (isValid(currentRow - 1, currentCol) && !contains(walls,currentRow - 1, currentCol)) walls.push_back(make_pair(currentRow - 1, currentCol));
            if (isValid(currentRow + 1, currentCol) && !contains(walls,currentRow + 1, currentCol)) walls.push_back(make_pair(currentRow + 1, currentCol));
            if (isValid(currentRow, currentCol - 1) && !contains(walls,currentRow, currentCol-1)) walls.push_back(make_pair(currentRow, currentCol - 1));
            if (isValid(currentRow, currentCol + 1) && !contains(walls,currentRow, currentCol+1)) walls.push_back(make_pair(currentRow, currentCol + 1));
        }
    }

    void addNeighbor(int currentRow, int currentCol, int neighborRow, int neighborCol) const {
        if (neighborRow == currentRow - 1) {
            grid[currentRow][currentCol].wallUp = false;
            grid[neighborRow][neighborCol].wallDown = false;
        } else if (neighborRow == currentRow + 1) {
            grid[currentRow][currentCol].wallDown = false;
            grid[neighborRow][neighborCol].wallUp = false;
        } else if (neighborCol == currentCol - 1) {
            grid[currentRow][currentCol].wallLeft = false;
            grid[neighborRow][neighborCol].wallRight = false;
        } else if (neighborCol == currentCol + 1) {
            grid[currentRow][currentCol].wallRight = false;
            grid[neighborRow][neighborCol].wallLeft = false;
        }
    }

    void printMaze() {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                cout << "+";
                if (grid[i][j].wallUp) cout << "-";
                else cout << " ";

            }
            cout << "+" << endl;

            for (int j = 0; j < cols; ++j) {
                if (grid[i][j].wallLeft) cout << "|";
                else cout << " ";

                cout << " ";

                if (j == cols - 1) {
                    if (grid[i][j].wallRight) cout << "|";
                    else cout << " ";
                }
            }
            cout << endl;
        }

        for (int j = 0; j < cols; ++j) {
            cout << "+";
            if (grid[rows - 1][j].wallDown) cout << "-";
            else cout << " ";
        }
        cout << "+" << endl;
    }
};

#endif //OPENGLPRJ_MAZE_H
