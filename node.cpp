#include "node.h"

Node::Node(OriginFile* file, Node* parentNode) {
    this->data = file;
    this->parent = parentNode;
    this->firstChild = nullptr;
    this->nextSibling = nullptr;
}

Node::~Node() {
    delete data;
    delete firstChild;
    delete nextSibling;
}

void Node::addChild(Node* child) {
    if (!child) return;

    child->parent = this;

    if (!firstChild) {
        firstChild = child;
    } else {
        Node* current = firstChild;
        while (current->nextSibling) {
            if (current->data->getName() == child->data->getName()) return;
            current = current->nextSibling;
        }

        if (current->data->getName() != child->data->getName()) {
            current->nextSibling = child;
        }
    }
}

Node* Node::findChild(const QString &name) {
    Node* current = firstChild;
    while (current) {
        if (current->data->getName() == name) {
            return current;
        }
        current = current->nextSibling;
    }
    return nullptr;
}
