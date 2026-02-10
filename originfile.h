#ifndef ORIGINFILE_H
#define ORIGINFILE_H

#include <QString>
#include <QDateTime>
#include <QDataStream>
#include <vector>

class OriginFile {
protected:
    QString name;
    QDateTime created;
    QDateTime modified;
    bool isDirectory;
    bool isFavorite;
    bool inRecycleBin;
    QString originalPath;

    OriginFile* parent;
    std::vector<OriginFile*> children;

public:
    OriginFile(QString name, bool isDirectory, OriginFile* parent = nullptr);
    virtual ~OriginFile();

    QString getName();
    void setName(QString newName);
    bool checkIsDirectory();

    OriginFile* getParent();
    void setParent(OriginFile* newParent);

    void addChild(OriginFile* child);
    void removeChild(OriginFile* child);

    int getChildCount();
    OriginFile* getChild(int index);

    bool getIsFavorite();
    void setIsFavorite(bool favorite);

    bool getInRecycleBin();
    void setInRecycleBin(bool bin);

    virtual long getSize() = 0;
    virtual void writeBinary(QDataStream &out);
    virtual void readBinary(QDataStream &in);
};

#endif
