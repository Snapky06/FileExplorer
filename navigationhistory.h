#ifndef NAVIGATIONHISTORY_H
#define NAVIGATIONHISTORY_H

#include "historynode.h"

class NavigationHistory {
private:
    HistoryNode* head;
    HistoryNode* tail;
    HistoryNode* current;

public:
    NavigationHistory();
    ~NavigationHistory();

    void addVisit(OriginFile* dir);
    OriginFile* goBack();
    OriginFile* goForward();
    bool canGoBack();
    bool canGoForward();
    void purgeSubtree(OriginFile* subtreeRoot);
};

#endif // NAVIGATIONHISTORY_H
