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

vector<vector<Cell>> maze(rows, vector<Cell>(cols));

class Maze {
public:

    Maze(){
        initializeGrid();
    }

    void initializeGrid() {

        //initialize maze full of walls
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                maze[i][j].row = i;
                maze[i][j].col = j;
                maze[i][j].visited = false;
                maze[i][j].wallUp = true;
                maze[i][j].wallDown = true;
                maze[i][j].wallLeft = true;
                maze[i][j].wallRight = true;
                maze[i][j].hasCoin = true;
                maze[i][j].hasPellet = false;
            }
        }

        //add pellets
        maze[0][0].hasPellet = true;
        maze[0][cols-1].hasPellet = true;
        maze[rows-1][0].hasPellet = true;
        maze[rows-1][cols-1].hasPellet = true;
        maze[2][2].hasPellet = true;
        maze[2][cols-3].hasPellet = true;
        maze[rows-3][2].hasPellet = true;
        maze[rows-3][cols-3].hasPellet = true;

        //delete coins on cells that have pellets
        maze[0][0].hasCoin = false;
        maze[0][cols-1].hasCoin = false;
        maze[rows-1][0].hasCoin = false;
        maze[rows-1][cols-1].hasCoin = false;
        maze[2][2].hasCoin = false;
        maze[2][cols-3].hasCoin = false;
        maze[rows-3][2].hasCoin = false;
        maze[rows-3][cols-3].hasCoin = false;
    }

    bool isNotVisited(int row, int col) {
        return (row >= 0 && row < rows && col >= 0 && col < cols && !maze[row][col].visited);
    }

    bool isVisited(int row, int col){
        return (row >= 0 && row < rows && col >= 0 && col < cols && maze[row][col].visited);
    }

    bool contains(vector<pair<int, int>> walls, int row, int col){
        for(pair<int,int> wall: walls)
            if(wall.first==row && wall.second==col)
                return true;
        return false;
    }

    void addNeighbor(int currentRow, int currentCol, int neighborRow, int neighborCol) const {
        if (neighborRow == currentRow - 1) {
            maze[currentRow][currentCol].wallUp = false;
            maze[neighborRow][neighborCol].wallDown = false;
        } else if (neighborRow == currentRow + 1) {
            maze[currentRow][currentCol].wallDown = false;
            maze[neighborRow][neighborCol].wallUp = false;
        } else if (neighborCol == currentCol - 1) {
            maze[currentRow][currentCol].wallLeft = false;
            maze[neighborRow][neighborCol].wallRight = false;
        } else if (neighborCol == currentCol + 1) {
            maze[currentRow][currentCol].wallRight = false;
            maze[neighborRow][neighborCol].wallLeft = false;
        }
    }

    void generateMaze() {

        //generate random column and row
        int startRow = rand() % rows;
        int startCol = rand() % cols;

        //mark cell as visited
        maze[startRow][startCol].visited = true;
        vector<pair<int, int>> unvisitedCells;

        //add unvisited neighbors to list
        if (isNotVisited(startRow - 1, startCol)) unvisitedCells.push_back(make_pair(startRow - 1, startCol));
        if (isNotVisited(startRow + 1, startCol)) unvisitedCells.push_back(make_pair(startRow + 1, startCol));
        if (isNotVisited(startRow, startCol - 1)) unvisitedCells.push_back(make_pair(startRow, startCol - 1));
        if (isNotVisited(startRow, startCol + 1)) unvisitedCells.push_back(make_pair(startRow, startCol + 1));

        //loop while unvisited cells list is full
        while (!unvisitedCells.empty()) {
            //choose a cell
            int randomIndex = rand() % unvisitedCells.size();
            int currentRow = unvisitedCells[randomIndex].first;
            int currentCol = unvisitedCells[randomIndex].second;
            maze[currentRow][currentCol].visited = true;

            //current cells neighbors
            vector<pair<int, int>> neighbors;

            //add all visited neighboring cells
            if (isVisited(currentRow - 1, currentCol)) neighbors.push_back(make_pair(currentRow - 1, currentCol));
            if (isVisited(currentRow + 1, currentCol)) neighbors.push_back(make_pair(currentRow + 1, currentCol));
            if (isVisited(currentRow, currentCol - 1)) neighbors.push_back(make_pair(currentRow, currentCol - 1));
            if (isVisited(currentRow, currentCol + 1)) neighbors.push_back(make_pair(currentRow, currentCol + 1));

            //pick a random neighbor to connect to
            if (!neighbors.empty()) {
                //randomly connect to 2 neighbors
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
            //erase current cell from list
            unvisitedCells.erase(unvisitedCells.begin() + randomIndex);

            //add all new unvisited neighbors
            if (isNotVisited(currentRow - 1, currentCol) && !contains(unvisitedCells,currentRow - 1, currentCol)) unvisitedCells.push_back(make_pair(currentRow - 1, currentCol));
            if (isNotVisited(currentRow + 1, currentCol) && !contains(unvisitedCells,currentRow + 1, currentCol)) unvisitedCells.push_back(make_pair(currentRow + 1, currentCol));
            if (isNotVisited(currentRow, currentCol - 1) && !contains(unvisitedCells,currentRow, currentCol-1)) unvisitedCells.push_back(make_pair(currentRow, currentCol - 1));
            if (isNotVisited(currentRow, currentCol + 1) && !contains(unvisitedCells,currentRow, currentCol+1)) unvisitedCells.push_back(make_pair(currentRow, currentCol + 1));
        }
    }
};

#endif //OPENGLPRJ_MAZE_H
