//
// Created by drist on 8/25/2023.
//

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
            }
        }
    }
    vector<vector<Cell>> getGrid(){
        return grid;
    }

    bool isValid(int row, int col) {
        //return (row >= 0 && row < rows && col >= 0 && col < cols);
        return (row >= 0 && row < rows && col >= 0 && col < cols && !grid[row][col].visited);
    }

    bool isVisited(int row, int col){
        return (row >= 0 && row < rows && col >= 0 && col < cols && grid[row][col].visited);
    }

    bool contains(vector<pair<int, int>> walls, int row, int col){
        for(pair<int,int> wall: walls)
            if(wall.first==row&&wall.second==col)
                return true;
        return false;
    }

    void generateMaze() {
        srand(time(0));

        int startRow = rand() % rows;
        int startCol = rand() % cols;

        //cout<<"Start: "<<startCol<<" "<<startRow<<endl;

        grid[startRow][startCol].visited = true;
        vector<pair<int, int>> walls;

        if (isValid(startRow - 1, startCol)) walls.push_back(make_pair(startRow - 1, startCol));
        if (isValid(startRow + 1, startCol)) walls.push_back(make_pair(startRow + 1, startCol));
        if (isValid(startRow, startCol - 1)) walls.push_back(make_pair(startRow, startCol - 1));
        if (isValid(startRow, startCol + 1)) walls.push_back(make_pair(startRow, startCol + 1));

        /*cout<<"Walls: ";
        for(pair<int,int>n : walls){
            cout<<" "<<n.second<<","<<n.first<<" ";
        }
        cout<<endl;*/

        while (!walls.empty()) {
            //choose a wall
            int randomIndex = rand() % walls.size();
            int currentRow = walls[randomIndex].first;
            int currentCol = walls[randomIndex].second;
            grid[currentRow][currentCol].visited = true;

            //cout<<"Current: "<<currentCol<<","<<currentRow<<"    ";
            //for choosing the visited wall
            vector<pair<int, int>> neighbors;

            //add all walls that you can connect to
            if (isVisited(currentRow - 1, currentCol)) neighbors.push_back(make_pair(currentRow - 1, currentCol));
            if (isVisited(currentRow + 1, currentCol)) neighbors.push_back(make_pair(currentRow + 1, currentCol));
            if (isVisited(currentRow, currentCol - 1)) neighbors.push_back(make_pair(currentRow, currentCol - 1));
            if (isVisited(currentRow, currentCol + 1)) neighbors.push_back(make_pair(currentRow, currentCol + 1));
            /*cout<<"Neighbors:";
            for(pair<int,int>n : neighbors){
                cout<<" "<<n.second<<","<<n.first<<" ";
            }
            cout<<endl;*/
            //pick a random wall to connect to
            if (!neighbors.empty()) {
                int randomNeighborIndex = rand() % neighbors.size();
                int neighborRow = neighbors[randomNeighborIndex].first;
                int neighborCol = neighbors[randomNeighborIndex].second;

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

                //grid[neighborRow][neighborCol].visited = true;

                /*if (isValid(neighborRow - 1, neighborCol)) walls.push_back(make_pair(neighborRow - 1, neighborCol));
                if (isValid(neighborRow + 1, neighborCol)) walls.push_back(make_pair(neighborRow + 1, neighborCol));
                if (isValid(neighborRow, neighborCol - 1)) walls.push_back(make_pair(neighborRow, neighborCol - 1));
                if (isValid(neighborRow, neighborCol + 1)) walls.push_back(make_pair(neighborRow, neighborCol + 1));*/
            }
            walls.erase(walls.begin() + randomIndex);
            //add all new unvisited walls
            if (isValid(currentRow - 1, currentCol) && !contains(walls,currentRow - 1, currentCol)) walls.push_back(make_pair(currentRow - 1, currentCol));
            if (isValid(currentRow + 1, currentCol) && !contains(walls,currentRow + 1, currentCol)) walls.push_back(make_pair(currentRow + 1, currentCol));
            if (isValid(currentRow, currentCol - 1) && !contains(walls,currentRow, currentCol-1)) walls.push_back(make_pair(currentRow, currentCol - 1));
            if (isValid(currentRow, currentCol + 1) && !contains(walls,currentRow, currentCol+1)) walls.push_back(make_pair(currentRow, currentCol + 1));
        }
    }

    void printMaze() {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                cout << "+";
                if (grid[i][j].wallUp) cout << "-";
                else cout << " ";
                /*cout<<"("<<i<<","<<j<<")"<<"   ";
                cout<<grid[i][j].wallLeft<<grid[i][j].wallUp<<grid[i][j].wallRight<<grid[i][j].wallDown<<endl;*/

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
