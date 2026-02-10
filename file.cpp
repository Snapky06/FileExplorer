#include "file.h"

File::File(QString name, OriginFile* parent)
    : OriginFile(name, false, parent) {
}

File::~File() {
}

QString File::getContent() const {
    return content;
}

void File::setContent(const QString &newContent) {
    content = newContent;
    this->modified = QDateTime::currentDateTime();
}

long File::getSize() const {
    return content.toUtf8().size();
}

void File::writeBinary(QDataStream &out) const {
    OriginFile::writeBinary(out);
    out << content;
}

void File::readBinary(QDataStream &in) {
    OriginFile::readBinary(in);
    in >> content;
}
