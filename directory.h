#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "originfile.h"
#include <vector>

class Directory : public OriginFile {
private:
    std::vector<OriginFile*> children;

public:
    Directory(QString name, OriginFile* parent = nullptr);
    ~Directory();

    void addChild(OriginFile* child);
    void removeChild(OriginFile* child);

    int getChildCount();
    OriginFile* getChild(int index);
    std::vector<OriginFile*> getChildren();

    long getSize() override;
    void write(QDataStream &out) override;
    void read(QDataStream &in) override;
};

#endif // DIRECTORY_H
