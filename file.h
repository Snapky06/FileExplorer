#ifndef FILE_H
#define FILE_H

#include "originfile.h"

class File : public OriginFile {
private:
    QString content;

public:
    File(QString name, OriginFile* parent = nullptr);
    ~File();

    QString getContent();
    void setContent(QString newContent);

    long getSize() override;
    void writeBinary(QDataStream &out) override;
    void readBinary(QDataStream &in) override;
};

#endif
