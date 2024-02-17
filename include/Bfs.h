#ifndef OPENGLPRJ_BFS_H
#define OPENGLPRJ_BFS_H
#include <stack>
#include "Maze.h"
using namespace std;

class BFS {

private:

    // number of cells
    int V;

    // adjacency List
    vector<vector<int> > adj;

    // get max element from list
    int maxElement(vector<int> list){
        int max = 0;
        for(int i=1;i<list.size();i++){
            if(list[i] > list[max]){
                max = i;
            }
        }
        return max;
    }

    // checks if row and column are inside the maze
    bool isValid(int row, int col){
        return (row >= 0 && row < rows && col >= 0 && col < cols);
    }

    // add edge to cell
    void addEdge(int v, int w){
        adj[v].push_back(w);
    }

    // this function returns if destination is reachable or not
    // additionally it sets the parent array to say the path (if exist)
    bool Run_BFS(int source, int dest, int parent[]) {

        queue<int> q;
        bool* visited = new bool[V];

        // setting all the vertices as non-visited
        // and parents of all vertices as -1.
        for(int i=0; i<V; i++) {
            visited[i] = false;
            parent[i] = -1;
        }

        // pushing the source into the queue and mark it as visited.
        q.push(source);
        visited[source] = true;

        // loop executes until all vertices are traversed.
        while(!q.empty()) {

            // popping one element from queue.
            int temp = q.front();
            q.pop();

            // check for all adjacent
            for(int k: adj[temp]) {
                if(visited[k] == false) {

                    // pushing into queue and mark it visited as well as
                    // set the parent of the adjacent in parent array
                    q.push(k);
                    visited[k] = true;
                    parent[k] = temp;

                    // if destination is reached, returns true to indicate that a path exists
                    if(k == dest)
                        return true;
                }
            }
        }

        // if destination is not reachable
        return false;
    }

public:
    // use maze from Maze.h to initialize the adjacency list
    BFS(){
        this->V = rows*cols;
        adj.resize(V);
        for(int i=0;i<rows;i++){
            for(int j=0;j<cols;j++){
                if(isValid(i-1,j) && !maze[i][j].wallUp){
                    addEdge(j+i*cols,j+(i-1)*cols);
                }
                if(isValid(i+1,j) && !maze[i][j].wallDown){
                    addEdge(j+i*cols,j+(i+1)*cols);
                }
                if(isValid(i,j-1) && !maze[i][j].wallLeft){
                    addEdge(j+i*cols,(j-1)+(i)*cols);
                }
                if(isValid(i,j+1) && !maze[i][j].wallRight){
                    addEdge(j+i*cols,(j+1)+(i)*cols);
                }
            }
        }
    }

    // used by ghosts running from pacman
    int getRunningPath(int src, int pacman){
        int src_x = src % cols;
        int src_z = src / cols;
        int pac_x = pacman % cols;
        int pac_z = pacman / cols;
        vector<int> distances;
        for(int cord : adj[src]){
            int x = cord % cols;
            int z = cord / cols;
            int dist = glm::abs(pac_x - x) + glm::abs(pac_z - z);
            distances.push_back(dist);
        }
        int next = adj[src][maxElement(distances)];
        if(distances[maxElement(distances)] <= glm::abs(pac_x - src_x)+glm::abs(pac_z - src_z))
            return src;
        return next;
    }

    // function to get the shortest path
    vector<int> getPath(int source, int dest) {

        // it stores parent for each vertex to trace the path
        int* parent = new int[V];

        vector<int> path;

        // BFS returns false means destination is not reachable, return empty path
        if(Run_BFS(source, dest, parent) == false) {
            return path;
        }

        // tracing the path
        while(parent[dest] != -1) {
            // Add to the front of the list
            path.insert(path.begin(), dest);
            dest = parent[dest];
        }

        return path;
    }
};

#endif // OPENGLPRJ_BFS_H
