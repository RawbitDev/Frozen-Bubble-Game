#ifndef HMI_GRAPH_H
#define HMI_GRAPH_H
#include <iostream>
#include <utility>
#include <set>
#include <map>
#include <vector>
#include "Node.h"
using namespace std;

class Graph {
private:
    string name;
    map<string, Node> nodes;
    set<pair<string, string>> edges;
    bool checkPathNode(const string& a, const string& b, set<string>& visited);

public:
    explicit Graph(string  n);

    void clear();
    void print();
    const string &getName() const;
    Node* getNode(const string& n);
    Node* addNode(const string& n);
    void removeNode(const string& n);
    void clearEdges(const string& n);
    vector<string> getAllNodes() const;
    const map<string, Node> &getNodes() const;
    vector<string> getNeighbors(const string& n);
    void addEdge(const string& a, const string& b);
    Node* addNode(const string& n, const Color &c);
    bool checkEdge(const string& a, const string& b);
    bool checkPath(const string& a, const string& b);
    void setNodeColor(const string& n, const Color &c);
};

#endif //HMI_GRAPH_H