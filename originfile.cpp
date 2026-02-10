#include "originfile.h"

OriginFile::OriginFile(QString name, bool isDirectory, OriginFile* parent) {
    this->name = name;
    this->isDirectory = isDirectory;
    this->parent = parent;
    this->created = QDateTime::currentDateTime();
    this->modified = this->created;
    this->size = 0;
}

OriginFile::~OriginFile() {}

QString OriginFile::getName() const { return name; }

void OriginFile::setName(const QString &newName) {
    name = newName;
    modified = QDateTime::currentDateTime();
}

bool OriginFile::checkIsDirectory() const { return isDirectory; }

OriginFile* OriginFile::getParent() const { return parent; }

void OriginFile::writeBinary(QDataStream &out) const {
    out << name;
    out << isDirectory;
    out << created;
    out << modified;
}

void OriginFile::readBinary(QDataStream &in) {
    in >> name;
    in >> isDirectory;
    in >> created;
    in >> modified;
}
