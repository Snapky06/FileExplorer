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
    void setContent(QString c);

    long getSize() override;
    void write(QDataStream &out) override;
    void read(QDataStream &in) override;
};

#endif //FILE_H
