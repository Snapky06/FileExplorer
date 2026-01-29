#ifndef ORIGINFILE_H
#define ORIGINFILE_H

#include <QString>
#include <QDateTime>
class OriginFile
{
protected:

    QString name;
    long size;
    QDateTime created;
    QDateTime modified;
    bool isDirectory;
    OriginFile* parent;

public:
    OriginFile(QString name, bool isDirectory, OriginFile* parent = nullptr);
    virtual ~OriginFile() = default;

    QString getName() const {return name;}
    void setName(const QString newName) {name = newName;}

    virtual long getSize() const = 0;
    QDateTime getCreationDate() const {return created;}
    QDateTime getModificationDate() const {return modified;}
    bool checkIsDirectory() const {return isDirectory;}

};

#endif // ORIGINFILE_H
