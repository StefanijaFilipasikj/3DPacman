#ifndef OPENGLPRJ_GHOST_H
#define OPENGLPRJ_GHOST_H
#include <OpenGLPrj.hpp>
#include "Bfs.h"
#include "cmath"

class Ghost{
private:
    BFS algorithm;
    float moved;
    glm::vec3 destinationCellPosition;
    glm::vec3 ghostCellPosition;
    vector<int> path;

public:
    float moveSpeed;
    glm::vec3 position;
    bool isScared;
    float rotation;

    Ghost() {}

    Ghost(glm::vec3 pos){
        ghostCellPosition = pos;
        destinationCellPosition = pos;
        moved = 1;
        position = pos;
        algorithm = BFS();
        isScared=false;
        moveSpeed = 0.5f;
    }

    void move(float deltaTime, int dest){
        if(path.size()!=0){

            destinationCellPosition.x = path[0]%cols;
            destinationCellPosition.z = path[0]/cols;
            moved += deltaTime * moveSpeed;

            position.x += (destinationCellPosition.x - ghostCellPosition.x)*deltaTime*moveSpeed;
            position.z += (destinationCellPosition.z - ghostCellPosition.z)*deltaTime*moveSpeed;

            if(destinationCellPosition.x - ghostCellPosition.x != 0){
                if(destinationCellPosition.x - ghostCellPosition.x > 0)
                    rotation = 90.0f;
                else rotation = 270.0f;
            }
            if(destinationCellPosition.z - ghostCellPosition.z != 0){
                if(destinationCellPosition.z - ghostCellPosition.z > 0)
                    rotation = 0.0f;
                else rotation = 180.0f;
            }
        }else{
            moved = 1;
        }
        if(moved >= 1){
            ghostCellPosition = destinationCellPosition;
            position = destinationCellPosition;
            moved = 0;
            if(!isScared){
                path = algorithm.getPath(ghostCellPosition.x + ghostCellPosition.z*cols, dest);
            }else{
                path = vector<int>();
                path.push_back(algorithm.getRunningPath(ghostCellPosition.x + ghostCellPosition.z*cols,dest));
            }
        }
    }
};

#endif // OPENGLPRJ_GHOST_H
