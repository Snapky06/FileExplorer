#include "originfile.h"

OriginFile::OriginFile(QString name, bool isDirectory, OriginFile* parent) {
    this->name = name;
    this->isDirectory = isDirectory;
    this->parent = parent;

    this->created = QDateTime::currentDateTime();
    this->modified = this->created;

    this->isFavorite = false;
    this->inRecycleBin = false;
    this->originalPath = "";
}

OriginFile::~OriginFile() {
    for (int i = 0; i < (int)children.size(); i++) {
        delete children[i];
    }
    children.clear();
}

QString OriginFile::getName() {
    return name;
}

void OriginFile::setName(QString newName) {
    name = newName;
    modified = QDateTime::currentDateTime();
}

bool OriginFile::checkIsDirectory() {
    return isDirectory;
}

OriginFile* OriginFile::getParent() {
    return parent;
}

void OriginFile::setParent(OriginFile* newParent) {
    parent = newParent;
}

void OriginFile::addChild(OriginFile* child) {
    if (child == nullptr) {
        return;
    }

    for (int i = 0; i < (int)children.size(); i++) {
        if (children[i]->getName() == child->getName()) {
            return;
        }
    }

    child->setParent(this);
    children.push_back(child);
}

void OriginFile::removeChild(OriginFile* child) {
    int index = -1;

    for (int i = 0; i < (int)children.size(); i++) {
        if (children[i] == child) {
            index = i;
            break;
        }
    }

    if (index != -1) {
        children.erase(children.begin() + index);
    }
}

int OriginFile::getChildCount() {
    return (int)children.size();
}

OriginFile* OriginFile::getChild(int index) {
    return children[index];
}

bool OriginFile::getIsFavorite() {
    return isFavorite;
}

void OriginFile::setIsFavorite(bool favorite) {
    isFavorite = favorite;
}

bool OriginFile::getInRecycleBin() {
    return inRecycleBin;
}

void OriginFile::setInRecycleBin(bool bin) {
    inRecycleBin = bin;
}

QString OriginFile::getOriginalPath() {
    return originalPath;
}

void OriginFile::setOriginalPath(QString path) {
    originalPath = path;
}

void OriginFile::writeBinary(QDataStream &out) {
    out << name << created << modified << isDirectory << isFavorite << inRecycleBin << originalPath;
}

void OriginFile::readBinary(QDataStream &in) {
    in >> name >> created >> modified >> isDirectory >> isFavorite >> inRecycleBin >> originalPath;
}
