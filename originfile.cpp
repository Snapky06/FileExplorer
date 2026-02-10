#include "originfile.h"

OriginFile::OriginFile(QString name, bool isDirectory, OriginFile* parent) {
    this->name = name;
    this->isDirectory = isDirectory;
    this->parent = parent;
    this->created = QDateTime::currentDateTime();
    this->modified = this->created;
    this->isFavorite = false;
    this->inRecycleBin = false;
}

OriginFile::~OriginFile() {
    for (int i = 0; i < children.size(); i++) {
        delete children[i];
    }
}

QString OriginFile::getName() { return name; }
void OriginFile::setName(QString newName) { name = newName; modified = QDateTime::currentDateTime(); }
bool OriginFile::checkIsDirectory() { return isDirectory; }
OriginFile* OriginFile::getParent() { return parent; }
void OriginFile::setParent(OriginFile* newParent) { parent = newParent; }

void OriginFile::addChild(OriginFile* child) {
    if (child == nullptr) return;
    for (int i = 0; i < children.size(); i++) {
        if (children[i]->name == child->name) return;
    }
    child->setParent(this);
    children.push_back(child);
}

void OriginFile::removeChild(OriginFile* child) {
    int pos = -1;
    for (int i = 0; i < children.size(); i++) {
        if (children[i] == child) {
            pos = i;
            break;
        }
    }
    if (pos != -1) {
        children.erase(children.begin() + pos);
    }
}

int OriginFile::getChildCount() { return children.size(); }
OriginFile* OriginFile::getChild(int index) { return children[index]; }
bool OriginFile::getIsFavorite() { return isFavorite; }
void OriginFile::setIsFavorite(bool favorite) { isFavorite = favorite; }
bool OriginFile::getInRecycleBin() { return inRecycleBin; }
void OriginFile::setInRecycleBin(bool bin) { inRecycleBin = bin; }

void OriginFile::writeBinary(QDataStream &out) {
    out << name << created << modified << isDirectory << isFavorite << inRecycleBin << originalPath;
    out << (int)children.size();
    for (int i = 0; i < children.size(); i++) {
        children[i]->writeBinary(out);
    }
}

void OriginFile::readBinary(QDataStream &in) {
    in >> name >> created >> modified >> isDirectory >> isFavorite >> inRecycleBin >> originalPath;
}
