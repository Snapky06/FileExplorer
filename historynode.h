#ifndef HISTORYNODE_H
#define HISTORYNODE_H

#include "originfile.h"

class HistoryNode {
public:
    OriginFile* directory;
    HistoryNode* next;
    HistoryNode* prev;

    HistoryNode(OriginFile* dir);
};

#endif // HISTORYNODE_H
