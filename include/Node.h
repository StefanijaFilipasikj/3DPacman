//
// Created by drist on 8/27/2023.
//

#ifndef OPENGLPRJ_NODE_H
#define OPENGLPRJ_NODE_H

class Node {
public:
    Vector2 cords;
    Node* parent;
    Node(Vector2 v) : cords(v) {parent = nullptr;}
    vector<Node*> getNeighbors(){
        vector<Node*> neighbors;
        if(cords.x+1!=6 && !contains(obsticles, cords)){
            Node* n = new Node(Vector2(cords.x+1,cords.y));
            neighbors.push_back(n);
            n->parent = this;
        }
        if(cords.x-1!=-1 && !contains(obsticles, cords)){
            Node* n = new Node(Vector2(cords.x-1,cords.y));
            neighbors.push_back(n);
            n->parent = this;
        }
        if(cords.y+1!=6 && !contains(obsticles, cords)){
            Node* n = new Node(Vector2(cords.x,cords.y+1));
            neighbors.push_back(n);
            n->parent = this;
        }
        if(cords.y-1!=-1 && !contains(obsticles, cords)){
            Node* n = new Node(Vector2(cords.x,cords.y-1));
            neighbors.push_back(n);
            n->parent = this;
        }
        return neighbors;
    }
    bool equals(Vector2 v){
        return (cords.x == v.x && cords.y == v.y);
    }
};

#endif //OPENGLPRJ_NODE_H
