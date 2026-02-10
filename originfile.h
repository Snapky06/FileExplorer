#ifndef ORIGINFILE_H
#define ORIGINFILE_H

#include <QString>
#include <QDateTime>
#include <QDataStream>

class OriginFile {
protected:
    QString name;
    long size;
    QDateTime created;
    QDateTime modified;
    bool isDirectory;
    OriginFile* parent;

public:
    OriginFile(QString name, bool isDirectory, OriginFile* parent = nullptr);
    virtual ~OriginFile();

    QString getName() const;
    void setName(const QString &newName);
    bool checkIsDirectory() const;
    OriginFile* getParent() const;

    virtual void writeBinary(QDataStream &out) const;
    virtual void readBinary(QDataStream &in);

    virtual long getSize() const = 0;
};

#endif // ORIGINFILE_H
