#include "navigationhistory.h"

NavigationHistory::NavigationHistory() {
    head = nullptr;
    tail = nullptr;
    current = nullptr;
}

NavigationHistory::~NavigationHistory() {
    HistoryNode* tmp = head;
    while (tmp!=nullptr) {
        HistoryNode* next = tmp->next;
        delete tmp;
        tmp = next;
    }
}

void NavigationHistory::addVisit(OriginFile* dir) {
    if (current && current->directory == dir) return;

    clearForwardHistory();

    HistoryNode* newNode = new HistoryNode(dir);

    if (!head) {
        head = tail = current = newNode;
    } else {
        tail->next = newNode;
        newNode->prev = tail;
        tail = newNode;
        current = newNode;
    }
}

OriginFile* NavigationHistory::goBack() {
    if (canGoBack()) {
        current = current->prev;
        return current->directory;
    }
    return nullptr;
}

OriginFile* NavigationHistory::goForward() {
    if (canGoForward()) {
        current = current->next;
        return current->directory;
    }
    return nullptr;
}

bool NavigationHistory::canGoBack() const {
    return current && current->prev;
}

bool NavigationHistory::canGoForward() const {
    return current && current->next;
}

void NavigationHistory::clearForwardHistory() {
    if (!current) return;

    HistoryNode* toDelete = current->next;
    current->next = nullptr;
    tail = current;

    while (toDelete) {
        HistoryNode* next = toDelete->next;
        delete toDelete;
        toDelete = next;
    }
}
