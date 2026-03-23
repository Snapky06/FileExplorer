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

    if (head == nullptr) {
        HistoryNode* newNode = new HistoryNode(dir);
        head = newNode;
        tail = newNode;
        current = newNode;
    } else if (current != nullptr) {
        HistoryNode* toDelete = current->next;
        while (toDelete != nullptr) {
            HistoryNode* nextToDelete = toDelete->next;
            delete toDelete;
            toDelete = nextToDelete;
        }

        HistoryNode* newNode = new HistoryNode(dir);
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

void NavigationHistory::purgeSubtree(OriginFile* subtreeRoot) {
    if (!subtreeRoot) return;

    HistoryNode* node = head;
    while (node != nullptr) {
        HistoryNode* next = node->next;

        bool shouldRemove = false;
        OriginFile* temp = node->directory;
        while (temp != nullptr) {
            if (temp == subtreeRoot) {
                shouldRemove = true;
                break;
            }
            temp = temp->getParent();
        }

        if (shouldRemove) {

            if (node->prev) node->prev->next = node->next;
            else head = node->next;

            if (node->next) node->next->prev = node->prev;
            else tail = node->prev;

            if (node == current) {
                current = node->prev ? node->prev : node->next;
            }

            delete node;
        }

        node = next;
    }

    if (head == nullptr) {
        tail = nullptr;
        current = nullptr;
    }
}
