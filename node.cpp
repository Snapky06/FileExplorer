#include "node.h"

Node::Node(OriginFile* file) {
    this->data = file;
    this->next = nullptr;
    this->prev = nullptr;
}
