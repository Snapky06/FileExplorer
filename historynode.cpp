#include "historynode.h"

HistoryNode::HistoryNode(OriginFile* dir) {
    this->directory = dir;
    this->next = nullptr;
    this->prev = nullptr;
}
