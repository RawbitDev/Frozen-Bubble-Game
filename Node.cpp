#include "Node.h"

Node::Node(string n) : name(move(n)) {}

Node::Node(string n, const Color &c) : name(std::move(n)), color(c) {}

const string &Node::getName() const {
    return name;
}

const Color &Node::getColor() const {
    return color;
}

void Node::setColor(const Color &c) {
    Node::color = c;
}

void Node::setColor(int r, int g, int b) {
    Node::color = Color(r, g, b);
}

void Node::setColor(int r, int g, int b, int a) {
    Node::color = Color(r, g, b, a);
}
