#ifndef NAVIGATIONHISTORY_H
#define NAVIGATIONHISTORY_H

#include "historynode.h"

class NavigationHistory {
private:
    HistoryNode* current;
    HistoryNode* head;
    HistoryNode* tail;

    void clearForward();

public:
    NavigationHistory();
    ~NavigationHistory();

    void addVisit(QString path);
    QString goBack();
    QString goForward();
    QString getCurrent();

    bool canGoBack();
    bool canGoForward();
};
#endif // NAVIGATIONHISTORY_H
