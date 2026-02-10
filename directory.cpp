#include "directory.h"

Directory::Directory(QString name, OriginFile* parent) : OriginFile(name, true, parent) {
}

Directory::~Directory() {
}

long Directory::getSize() {
    long total = 0;
    int totalHijos = children.size();
    for (int i = 0; i < totalHijos; i++) {
        total = total + children[i]->getSize();
    }
    return total;
}

int Directory::getFileCount() {
    int contador = 0;
    int totalHijos = children.size();
    for (int i = 0; i < totalHijos; i++) {
        if (children[i]->checkIsDirectory() == false) {
            contador = contador + 1;
        }
    }
    return contador;
}

void Directory::writeBinary(QDataStream &out) {
    OriginFile::writeBinary(out);
}

void Directory::readBinary(QDataStream &in) {
    OriginFile::readBinary(in);
}
