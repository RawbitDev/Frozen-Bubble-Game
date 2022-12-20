#ifndef HMI_LEVEL_H
#define HMI_LEVEL_H
#include <utility>
#include "iostream"
#include "map"
#include "Graph.h"


const int LINE_NUM = 3;

class Level {
private:
    string name;
    Graph graph;
    map<string, Color> colors;
    map <string, int> currentColors;

private:
    char deathZone = 'L';
    void checkLineNode(const string& n, Color color, set<string>& found);
    
public:
    Level();
    Level(string n, Graph g);
    explicit Level(const string &name);

    void print();
    bool isWon() const;
    bool isGameOver() const;
    void expandDeathZone();
    void removeDroppedNodes();
    char getDeathZone() const;
    Graph &getGraph();
    const string &getName() const;
    void checkLine(const string& n);
    void insertNode(char row, int column, Color color);
    void setColors(const map<string, Color> &colors);
    void calculateCurrentColors();


    const map<string, int> &getCurrentColors() const;


};

#endif //HMI_LEVEL_H