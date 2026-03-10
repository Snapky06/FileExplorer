#include "navigationhistory.h"

NavigationHistory::NavigationHistory() {
    head = nullptr;
    tail = nullptr;
    current = nullptr;
}

NavigationHistory::~NavigationHistory() {
    HistoryNode* temp = head;
    while (temp != nullptr) {
        HistoryNode* nextNode = temp->next;
        delete temp;
        temp = nextNode;
    }
}

void NavigationHistory::addVisit(OriginFile* dir) {
    if (current != nullptr && current->directory == dir) {
        return;
    }

    HistoryNode* newNode = new HistoryNode(dir);
    if (head == nullptr) {
        head = newNode;
        tail = newNode;
        current = newNode;
    } else {
        HistoryNode* toDelete = current->next;
        while (toDelete != nullptr) {
            HistoryNode* nextToDelete = toDelete->next;
            delete toDelete;
            toDelete = nextToDelete;
        }

        current->next = newNode;
        newNode->prev = current;
        current = newNode;
        tail = newNode;
    }
}

OriginFile* NavigationHistory::goBack() {
    if (current != nullptr && current->prev != nullptr) {
        current = current->prev;
        return current->directory;
    }
    return nullptr;
}

OriginFile* NavigationHistory::goForward() {
    if (current != nullptr && current->next != nullptr) {
        current = current->next;
        return current->directory;
    }
    return nullptr;
}

bool NavigationHistory::canGoBack() {
    return (current != nullptr && current->prev != nullptr);
}

bool NavigationHistory::canGoForward() {
    return (current != nullptr && current->next != nullptr);
}
