#include "directory.h"

Directory::Directory(QString name, OriginFile* parent) : OriginFile(name, true, parent) {
}

Directory::~Directory() {
}

long Directory::getSize() {
    long total = 0;
    int count = (int)children.size();

    for (int i = 0; i < count; i++) {
        total = total + children[i]->getSize();
    }

    return total;
}

int Directory::getFileCount() {
    int fileCount = 0;
    int count = (int)children.size();

    for (int i = 0; i < count; i++) {
        if (children[i]->checkIsDirectory() == false) {
            fileCount = fileCount + 1;
        }
    }
    return fileCount;
}

void Directory::writeBinary(QDataStream &out) {
    OriginFile::writeBinary(out);
}

void Directory::readBinary(QDataStream &in) {
    OriginFile::readBinary(in);
}
