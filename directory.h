#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "originfile.h"

class Directory : public OriginFile {
public:
    Directory(QString name, OriginFile* parent = nullptr);
    ~Directory();

    long getSize() override;
    int getFileCount();

    void writeBinary(QDataStream &out) override;
    void readBinary(QDataStream &in) override;
};

#endif
