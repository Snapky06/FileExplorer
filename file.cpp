#include "file.h"

File::File(QString name, OriginFile* parent) : OriginFile(name, false, parent) {
}

File::~File() {
}

QString File::getContent() {
    return content;
}

void File::setContent(QString newContent) {
    content = newContent;
    modified = QDateTime::currentDateTime();
}

long File::getSize() {
    return content.toUtf8().size();
}

void File::writeBinary(QDataStream &out) {
    OriginFile::writeBinary(out);
    out << content;
}

void File::readBinary(QDataStream &in) {
    OriginFile::readBinary(in);
    in >> content;
}
