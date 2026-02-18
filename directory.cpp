#include "directory.h"
#include "file.h"

Directory::Directory(QString name, OriginFile* parent) : OriginFile(name, true, parent) {
}

Directory::~Directory() {
    for (int i = 0; i < (int)children.size(); i++) {
        delete children[i];
    }
    children.clear();
}

void Directory::addChild(OriginFile* child) {
    for (int i = 0; i < (int)children.size(); i++) {
        if (children[i]->getName() == child->getName()) {
            return;
        }
    }
    child->setParent(this);
    children.push_back(child);
}

void Directory::removeChild(OriginFile* child) {
    for (int i = 0; i < (int)children.size(); i++) {
        if (children[i] == child) {
            children.erase(children.begin() + i);
            break;
        }
    }
}

int Directory::getChildCount() {
    return (int)children.size();
}

OriginFile* Directory::getChild(int index) {
    if (index < 0 || index >= (int)children.size()) return nullptr;
    return children[index];
}

std::vector<OriginFile*> Directory::getChildren() {
    return children;
}

long Directory::getSize() {
    long total = 0;
    for (int i = 0; i < (int)children.size(); i++) {
        total += children[i]->getSize();
    }
    return total;
}

void Directory::write(QDataStream &out) {
    OriginFile::write(out);

    out << (int)children.size();

    for (int i = 0; i < (int)children.size(); i++) {
        bool isDir = children[i]->getIsDirectory();
        out << isDir;
        children[i]->write(out);
    }
}

void Directory::read(QDataStream &in) {
    OriginFile::read(in);

    int count;
    in >> count;

    for (int i = 0; i < count; i++) {
        bool isChildDirectory;
        in >> isChildDirectory;

        OriginFile* child;
        if (isChildDirectory) {
            child = new Directory("", this);
        } else {
            child = new File("", this);
        }

        child->read(in);
        children.push_back(child);
    }
}
