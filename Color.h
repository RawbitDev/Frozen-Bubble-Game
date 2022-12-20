#include <utility>

#ifndef HMI_COLOR_H
#define HMI_COLOR_H
#include <iostream>
using namespace std;

const string DEFAULT_COLOR_NAME = "WHITE";
const int DEFAULT_COLOR[] = {255, 255, 255};

class Color {
public:
    Color() = default;
    Color(int r, int g, int b) {
        this->r = max(0, min(255, r));
        this->g = max(0, min(255, g));
        this->b = max(0, min(255, b));
    };
    Color(string colorName, int r, int g, int b) {
        this->colorName = std::move(colorName);
        this->r = max(0, min(255, r));
        this->g = max(0, min(255, g));
        this->b = max(0, min(255, b));
    };
    Color(int r, int g, int b, float a) {
        this->r = max(0, min(255, r));
        this->g = max(0, min(255, g));
        this->b = max(0, min(255, b));
        this->a = max(0.0f, min(1.0f, a));
    };
    Color(string colorName, int r, int g, int b, float a) {
        this->colorName = std::move(colorName);
        this->r = max(0, min(255, r));
        this->g = max(0, min(255, g));
        this->b = max(0, min(255, b));
        this->a = max(0.0f, min(1.0f, a));
    };
    string colorName = DEFAULT_COLOR_NAME;
    int r = DEFAULT_COLOR[0];
    int g = DEFAULT_COLOR[1];
    int b = DEFAULT_COLOR[2];
    float a = 1.0f;

    friend bool operator==(const Color& c1, const Color& c2) {
        return (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b && c1.a == c2.a);
    }
};

#endif //HMI_COLOR_H