#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "originfile.h"
#include "node.h"

class Directory : public OriginFile {
public:
    Directory(QString name, OriginFile* parent = nullptr);
    ~Directory();

    // Sum of all files inside
    long getSize() const override;

    // Metadata: Count of immediate children
    int getFileCount() const;

    void writeBinary(QDataStream &out) const override;
    void readBinary(QDataStream &in) override;
};

#endif // DIRECTORY_H
