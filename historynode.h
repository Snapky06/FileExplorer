#ifndef HISTORYNODE_H
#define HISTORYNODE_H

#include <QString>

class HistoryNode {
public:
    QString path;
    HistoryNode* next;
    HistoryNode* prev;

    HistoryNode(QString p);
};
#endif // HISTORYNODE_H
