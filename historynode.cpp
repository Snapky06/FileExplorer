#include "historynode.h"

HistoryNode::HistoryNode(QString p) {
    path = p;
    next = nullptr;
    prev = nullptr;
}
