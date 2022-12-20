#include "Level.h"

Level::Level(): name("NULL"), graph(Graph("NULL")) {}

Level::Level(string n, Graph g): name(move(n)), graph(move(g)) {
    graph.addNode("ROOT");
    graph.addNode("QUEUE_0");
    graph.addEdge("QUEUE_0", "ROOT");
    graph.addNode("QUEUE_1");
    graph.addEdge("QUEUE_1", "ROOT");
}

Level::Level(const string &name) : name(name), graph(name){
    graph.addNode("ROOT");
    graph.addNode("QUEUE_0");
    graph.addEdge("QUEUE_0", "ROOT");
    graph.addNode("QUEUE_1");
    graph.addEdge("QUEUE_1", "ROOT");
}

const string &Level::getName() const {
    return name;
}

void Level::print() {
    graph.print();
}

void Level::insertNode(char row, int column, Color color) {
    string nodeName = row + to_string(column);
    graph.addNode(nodeName, color);
    if(row == 'A') {
        graph.addEdge(nodeName, "ROOT");
    }

    vector<string> possibleNeighbors = {
            static_cast<char>(row)+to_string(column-1),
            static_cast<char>(row)+to_string(column+1)
    };

    if((row-'A') % 2 == 0) {
        possibleNeighbors.push_back(static_cast<char>(row-1)+to_string(column-1));
        possibleNeighbors.push_back(static_cast<char>(row-1)+to_string(column));
        possibleNeighbors.push_back(static_cast<char>(row+1)+to_string(column-1));
        possibleNeighbors.push_back(static_cast<char>(row+1)+to_string(column));

    } else {
        possibleNeighbors.push_back(static_cast<char>(row-1)+to_string(column));
        possibleNeighbors.push_back(static_cast<char>(row-1)+to_string(column+1));
        possibleNeighbors.push_back(static_cast<char>(row+1)+to_string(column));
        possibleNeighbors.push_back(static_cast<char>(row+1)+to_string(column+1));

    }

    for(const string& pn : possibleNeighbors) {
        if (graph.getNode(pn)) {
            graph.addEdge(nodeName, pn);
        }
    }
}

void Level::setColors(const map<string, Color> &colors) {
    Level::colors = colors;
}

bool Level::isWon() const {
    // Only root left --> Game is won
    return graph.getAllNodes().size() == 3;
}

bool Level::isGameOver() const {
    for(const string& n : graph.getAllNodes()) {
        if (n[0] >= deathZone && n != "ROOT" && n != "QUEUE_0" && n != "QUEUE_1") {
            return true;
        }
    }
    return false;
}

void Level::expandDeathZone() {
    if(deathZone > 'A') {
        deathZone -= 1;
    }
}

void Level::checkLineNode(const string &n, Color color, set<string> &found) {
    for(const string& neighbor : graph.getNeighbors(n)) {
        if(graph.getNode(neighbor)->getColor() == color) {
            if(found.insert(neighbor).second) {
                // If neighbor wasn't already found
                checkLineNode(neighbor, color, found);
            }
        }
    }
}

void Level::checkLine(const string &node) {
    if(!graph.getNode(node)) {
        throw invalid_argument("Cannot check line: Node '"+node+"' wasn't found!");
    }
    set<string> found = {node};
    const Color color = graph.getNode(node)->getColor();
    checkLineNode(node, color, found);

    if(found.size() >= LINE_NUM) {
        for(const string& n : found) {
            graph.removeNode(n);
        }
    }
}

char Level::getDeathZone() const {
    return deathZone;
}

void Level::removeDroppedNodes() {
    for(const string& n : graph.getAllNodes()) {
        if(!graph.checkPath(n, "ROOT")) {
            graph.removeNode(n);
        }
    }
}

Graph &Level::getGraph(){
    return graph;
}

void Level::calculateCurrentColors() {
    map <string, int> amountColors;
    map<string, Node> nodes = graph.getNodes();

    for (const auto& n : nodes){
        string currentNodeColor = n.second.getColor().colorName;

        // WHITE is default color
        if (currentNodeColor == "WHITE"){
            continue;
        }

        auto it = amountColors.find(currentNodeColor);
        if(it != amountColors.end()){
            it->second = it->second + 1;
        }
        else{
            amountColors.insert(make_pair(currentNodeColor, 1));
        }
    }
    currentColors = amountColors;
}

const map<string, int> &Level::getCurrentColors() const {
    return currentColors;
}

