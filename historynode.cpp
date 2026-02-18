#include "historynode.h"

HistoryNode::HistoryNode(OriginFile* dir) {
    directory = dir;
    next = nullptr;
    prev = nullptr;
}
