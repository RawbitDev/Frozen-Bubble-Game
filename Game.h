#ifndef HMI_GAME_H
#define HMI_GAME_H
#include <cstdlib>
#include <ctime>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include "Graph.h"
#include "Level.h"
#include "Color.h"
using namespace std;

static const string LEVEL_PATH =  "../level";
static string LEVEL_ORDER[] = {".full","1" ,"2" ,"3" , "4"};

class Game {
private:
    vector<Level> levels;
    Level currentLevel;
    int lvlCounter = 0;

    void importLevels();
    void playLevel(Level l);
    Color createNewNodeColor();
public:
    Game();

    void nextLevel();
    void shoot(char row, int column);
    static Color stringToColor(const string& color);

    Level &getCurrentLevel();
    Level &getFullGraphLevel();
};

#endif //HMI_GAME_H
