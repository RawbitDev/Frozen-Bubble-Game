#include "Game.h"

Game::Game() {
    cout << "Preparing levels..." << endl;
    importLevels();
    srand(time(nullptr));
}

void Game::nextLevel() {
    cout << "Starting new Level..." << endl;
    lvlCounter = (lvlCounter + 1) % levels.size();
    if(lvlCounter == 0) lvlCounter++;
    playLevel(levels[lvlCounter]);
}

void Game::playLevel(Level l) {
    cout << "Loading level " + l.getName() + "..." << endl;
    l.removeDroppedNodes();
    currentLevel = l;

    currentLevel.getGraph().setNodeColor("QUEUE_0", createNewNodeColor());
    currentLevel.getGraph().setNodeColor("QUEUE_1", createNewNodeColor());
}

void Game::importLevels() {
    for (const string &levelName: LEVEL_ORDER) {
        string filename = LEVEL_PATH + "/" + levelName + ".txt";
        Level level = Level(levelName);
        ifstream file(filename);

        if (file.is_open()) {
            string line;
            bool firstLine = true;

            while (getline(file, line)) {
                line.erase(remove(line.begin(), line.end(), ' '), line.end()); // Remove all spaces
                if (!line.empty() && line.back() == ';') {
                    line.pop_back();

                    if (firstLine) {
                        map<string, Color> levelColors;
                        firstLine = false;
                        stringstream lineS = stringstream(line);
                        string segment;

                        while (getline(lineS, segment, ',')) {
                            levelColors.insert(make_pair(segment, stringToColor(segment)));
                        }
                        level.setColors(levelColors);
                    } else {
                        // Nodes
                        string nodePosition = line.substr(0, line.find('_'));
                        char nodeRow = nodePosition[0];
                        int nodeColumn = nodePosition[1] - 48;

                        string nodeColor = line.substr(line.find('_') + 1);
                        level.insertNode(nodeRow, nodeColumn, stringToColor(nodeColor));
                    }
                }
            }
            file.close();
        } else {
            throw invalid_argument("Cannot import level: Level '" + levelName + "' wasn't found!");
        }
        levels.push_back(level);
        cout << "LEVEL: " << levelName<< " importiert." << endl;
    }
}

Color Game::stringToColor(const string &color) {
    if (color == "ORANGE") {
        return {"ORANGE", 240, 143, 17};
    }
    if (color == "BLUE") {
        return {"BLUE",0, 0, 255};
    }
    if (color == "GREEN") {
        return {"GREEN",0, 255, 0};
    }
    if (color == "PURPLE") {
        return {"PURPLE",166, 32, 240};
    }
    if (color == "BLACK") {
        return {"BLACK",0, 0, 0};
    }
    if (color == "YELLOW") {
        return {"YELLOW",255, 255, 0};
    }
    if (color == "WHITE") {
        return {"WHITE",255, 255, 255};
    }
    if (color == "GREY") {
        return {"GREY",150, 150, 150};
    }
    return {255, 255, 255};
}

Color Game::createNewNodeColor() {
    currentLevel.calculateCurrentColors();
    int r = rand() % currentLevel.getCurrentColors().size();
    auto it = currentLevel.getCurrentColors().begin();
    std::advance(it, r);
    return stringToColor(it->first);
}

void Game::shoot(char row, int column) {
    string nodeName = row + to_string(column);

    //cout << "\nBEFORE:" << endl;
    //currentLevel.print();

    Color newNodeColor = currentLevel.getGraph().getNode("QUEUE_0")->getColor();

    // Durchschieben der Farben von 1 auf 0
    Color queue_1 = currentLevel.getGraph().getNode("QUEUE_1")->getColor();
    currentLevel.getGraph().setNodeColor("QUEUE_0", queue_1);


    currentLevel.insertNode(row, column, newNodeColor);
    currentLevel.checkLine(nodeName);
    currentLevel.removeDroppedNodes();

    // Erstellen neuer Farbe für neuen Node in Queue
    Color newQueueColor = createNewNodeColor();
    currentLevel.getGraph().setNodeColor("QUEUE_1", newQueueColor);

    //cout << "\nAFTER:" << endl;
    //currentLevel.print();

}

Level &Game::getCurrentLevel() {
    return currentLevel;
}

Level &Game::getFullGraphLevel() {
    return levels[0];
}



