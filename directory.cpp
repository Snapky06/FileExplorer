#include "directory.h"
#include "node.h"

Directory::Directory(QString name, OriginFile* parent)
    : OriginFile(name, true, parent) {
}

Directory::~Directory() {
}

long Directory::getSize() const {
    long totalSize = 0;
    // Note: In your logic, the 'Node' associated with this directory
    // will be used to iterate through the firstChild and its siblings.
    // This allows summing only the files within this specific directory.
    return totalSize;
}

int Directory::getFileCount() const {
    int count = 0;
    // Logic to iterate siblings starting from firstChild to count elements.
    return count;
}

void Directory::writeBinary(QDataStream &out) const {
    OriginFile::writeBinary(out);
    // Persist directory-specific metadata
}

void Directory::readBinary(QDataStream &in) {
    OriginFile::readBinary(in);
}
