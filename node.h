#ifndef NODE_H
#define NODE_H

#include "originfile.h"

class Node {
public:
    OriginFile* data;
    Node* firstChild;
    Node* nextSibling;
    Node* parent;

    Node(OriginFile* file, Node* parentNode = nullptr);
    ~Node();

    void addChild(Node* child);
    void removeChild(Node* child);
    Node* findChild(const QString &name);
};

#endif // NODE_H
