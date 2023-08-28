//
// Created by drist on 8/28/2023.
//

#ifndef OPENGLPRJ_GHOST_H
#define OPENGLPRJ_GHOST_H
#include <OpenGLPrj.hpp>
#include "Bfs.h"
#include "cmath"

class Ghost{
private:
    BFS algorithm;
    float moveSpeed;
    float moved;
    glm::vec3 moveTo;
    glm::vec3 algPosition;
    vector<int> path;
public:
    glm::vec3 position;
    Ghost(glm::vec3 pos, int V){
        moveSpeed = 10.0f;
        algPosition = pos;
        moveTo = pos;
        moved = 1;
        position = pos;
        algorithm = BFS(V);
    }

    void move(float deltaTime, int dest){

        if(path.size()!=0){
            int dest_X = path[0]%cols;
            int dest_Z = path[0]/cols;
            moveTo.x = dest_X;
            moveTo.z = dest_Z;
            moved += deltaTime;

            position.x += (dest_X - algPosition.x)*deltaTime;
            position.z += (dest_Z - algPosition.z)*deltaTime;
        }else{
            moved = 1;
        }
        if(moved >= 1){
            algPosition = moveTo;
            position = moveTo;
            moved = 0;
            path = algorithm.get_path(algPosition.x + algPosition.z*cols, dest);
        }
    }
};

#endif //OPENGLPRJ_GHOST_H
