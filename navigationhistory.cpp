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

void NavigationHistory::addVisit(QString path) {
    if (current != nullptr && current->path == path) return;

    clearForward();

    HistoryNode* newNode = new HistoryNode(path);

    if (head == nullptr) {
        head = newNode;
        tail = newNode;
        current = newNode;
    } else {
        tail->next = newNode;
        newNode->prev = tail;
        tail = newNode;
        current = newNode;
    }
}

QString NavigationHistory::goBack() {
    if (canGoBack()) {
        current = current->prev;
        return current->path;
    }
    return "";
}

QString NavigationHistory::goForward() {
    if (canGoForward()) {
        current = current->next;
        return current->path;
    }
    return "";
}

QString NavigationHistory::getCurrent() {
    if (current) return current->path;
    return "";
}

bool NavigationHistory::canGoBack() {
    return current != nullptr && current->prev != nullptr;
}

bool NavigationHistory::canGoForward() {
    return current != nullptr && current->next != nullptr;
}

void NavigationHistory::clearForward() {
    if (current == nullptr) return;

    HistoryNode* temp = current->next;
    current->next = nullptr;
    tail = current;

    while (temp != nullptr) {
        HistoryNode* nextNode = temp->next;
        delete temp;
        temp = nextNode;
    }
}
