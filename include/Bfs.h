#ifndef OPENGLPRJ_BFS_H
#define OPENGLPRJ_BFS_H

#include <stack>
#include "Maze.h"
using namespace std;

class BFS {

    // No. of vertices
    int V;

    // Pointer to an array containing adjacency lists
    vector<vector<int> > adj;

    int maxElement(vector<int> list){
        int max = 0;
        for(int i=1;i<list.size();i++){
            if(list[i] > list[max]){
                max = i;
            }
        }
        return max;
    }

public:

    BFS(){
        this->V = 0;
        adj.resize(V);
    };

    BFS(int V){
        this->V = V;
        adj.resize(V);
        for(int i=0;i<rows;i++){
            for(int j=0;j<cols;j++){
                if(isValid(i-1,j) && !grid[i][j].wallUp){
                    addEdge(j+i*cols,j+(i-1)*cols);
                }
                if(isValid(i+1,j) && !grid[i][j].wallDown){
                    addEdge(j+i*cols,j+(i+1)*cols);
                }
                if(isValid(i,j-1) && !grid[i][j].wallLeft){
                    addEdge(j+i*cols,(j-1)+(i)*cols);
                }
                if(isValid(i,j+1) && !grid[i][j].wallRight){
                    addEdge(j+i*cols,(j+1)+(i)*cols);
                }
            }
        }
    }

    int getRunningPath(int src, int pacman){
        int src_x = src%cols;
        int src_z = src/cols;
        int pac_x = pacman%cols;
        int pac_z = pacman/cols;
        vector<int> distances;
        for(int cord : adj[src]){
            int x = cord%cols;
            int z = cord/cols;
            int dist = glm::abs(pac_x-x) + glm::abs(pac_z-z);
            distances.push_back(dist);
        }
        int next = adj[src][maxElement(distances)];
        if(distances[maxElement(distances)] <= glm::abs(pac_x-src_x)+glm::abs(pac_z-src_z))
            return src;
        return next;
    }

    bool isValid(int row, int col){
        return (row >= 0 && row < rows && col >= 0 && col < cols);
    }

    // Function to add an edge to graph
    void addEdge(int v, int w){
        adj[v].push_back(w);
    }

    void print(){
        for(int i=0;i<V;i++){
            cout<<i<<": ";
            for(int num:adj[i]){
                cout<<num<<" ";
            }
            cout<<endl;
        }
    }

    // function to print the shortest path.
    vector<int> get_path(int source, int dest) {

        // it stores parent for each vertex to trace the path.
        int* parent = new int[V];

        // stack to store the shortest path.
        stack<int> st;
        vector<int> path;

        // BFS returns false means destination is not reachable.
        if(Run_BFS(source, dest, parent) == false) {
            cout << "Destination is not reachable from this source.\n";
            return path;
        }

        // tracing the path.
        while(parent[dest] != -1) {
            st.push(dest);
            dest = parent[dest];
        }

        // printing the path.
        while(!st.empty()) {
            path.push_back(st.top());
            st.pop();
        }

        return path;
    }

    // this function returns if destination is reachable or not
// additionally it sets the parent array to say the path (if exist).
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

            // check for all adjacents.
            for(int k: adj[temp]) {
                if(visited[k] == false) {

                    // pushing into queue and mark it visited as well as
                    // set the parent of the adjacent in parent array.
                    q.push(k);
                    visited[k] = true;
                    parent[k] = temp;

                    // if destination is reached, returns true
                    // to state that there exist a path.
                    if(k == dest)
                        return true;
                }
            }
        }

        // if destination is not reachable.
        return false;
    }


};


#endif //OPENGLPRJ_BFS_H
