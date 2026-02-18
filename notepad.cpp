#include "notepad.h"
#include "ui_notepad.h"
#include <QMessageBox>

Notepad::Notepad(File* file, QWidget *parent) : QMainWindow(parent), ui(new Ui::Notepad) {
    ui->setupUi(this);
    currentFile = file;

    if (currentFile) {
        ui->text->setText(currentFile->getContent());
        this->setWindowTitle("Editing: " + currentFile->getName());
    }
}

Notepad::~Notepad() {
    delete ui;
}

void Notepad::on_saveB_clicked() {
    if (currentFile) {
        currentFile->setContent(ui->text->toPlainText());
        QMessageBox::information(this, "Success", "File saved successfully.");
    }
}


