#include "notepad.h"
#include "ui_notepad.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

Notepad::Notepad(QString path, QWidget *parent) : QMainWindow(parent), ui(new Ui::Notepad) {
    ui->setupUi(this);
    filePath = path;
    loadFile();
}

Notepad::~Notepad() {
    delete ui;
}

void Notepad::loadFile() {
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        ui->text->setText(in.readAll());
        file.close();
    }
}

void Notepad::on_saveButton_clicked() {
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << ui->text->toPlainText();
        file.close();
        QMessageBox::information(this, "Success", "File saved successfully!");
    } else {
        QMessageBox::warning(this, "Error", "Could not save file.");
    }
}

void Notepad::on_closeButton_clicked() {
    this->close();
}
