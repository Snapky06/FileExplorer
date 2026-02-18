#include "file.h"

File::File(QString name, OriginFile* parent) : OriginFile(name, false, parent) {
    content = "";
}

File::~File() {
}

QString File::getContent() {
    return content;
}

void File::setContent(QString c) {
    content = c;
    modified = QDateTime::currentDateTime();
}

long File::getSize() {
    return content.toUtf8().size();
}

void File::write(QDataStream &out) {
    OriginFile::write(out);
    out << content;
}

void File::read(QDataStream &in) {
    OriginFile::read(in);
    in >> content;
}
