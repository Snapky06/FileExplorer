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
}

QString OriginFile::getName() {
    return name;
}

void OriginFile::setName(QString n) {
    name = n;
    modified = QDateTime::currentDateTime();
}

bool OriginFile::getIsDirectory() {
    return isDirectory;
}

bool OriginFile::getIsFavorite() {
    return isFavorite;
}

void OriginFile::setIsFavorite(bool val) {
    isFavorite = val;
}

bool OriginFile::getInRecycleBin() {
    return inRecycleBin;
}

void OriginFile::setInRecycleBin(bool val) {
    inRecycleBin = val;
}

OriginFile* OriginFile::getParent() {
    return parent;
}

void OriginFile::setParent(OriginFile* p) {
    parent = p;
}

void OriginFile::write(QDataStream &out) {
    out << name << created << modified << isDirectory << isFavorite << inRecycleBin;
}

void OriginFile::read(QDataStream &in) {
    in >> name >> created >> modified >> isDirectory >> isFavorite >> inRecycleBin;
}
