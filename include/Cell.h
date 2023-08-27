//
// Created by drist on 8/25/2023.
//

#ifndef UNTITLED2_CELL_H
#define UNTITLED2_CELL_H

class Cell{
public:
    int row, col;
    bool visited;
    bool wallUp, wallDown, wallLeft, wallRight;

    int numWalls(){
        int num=0;
        if(wallUp) num++;
        if(wallDown) num++;
        if(wallRight) num++;
        if(wallLeft) num++;
        return num;
    }
};

#endif //UNTITLED2_CELL_H
