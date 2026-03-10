#ifndef ORIGINFILE_H
#define ORIGINFILE_H

#include <QString>
#include <QDateTime>
#include <QDataStream>

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

public:
    OriginFile(QString name, bool isDirectory, OriginFile* parent = nullptr);
    virtual ~OriginFile();

    QString getName();
    void setName(QString n);

    bool getIsDirectory();

    bool getIsFavorite();
    void setIsFavorite(bool val);

    bool getInRecycleBin();
    void setInRecycleBin(bool val);

    QString getOriginalPath();
    void setOriginalPath(QString path);

    OriginFile* getParent();
    void setParent(OriginFile* p);

    virtual long getSize() = 0;
    virtual void write(QDataStream &out);
    virtual void read(QDataStream &in);
};

#endif // ORIGIN_FILE
