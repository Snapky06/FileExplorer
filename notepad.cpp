#include "notepad.h"
#include "ui_notepad.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

Notepad::Notepad(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Notepad)
{
    ui->setupUi(this);

    // Minimalist tip: set the window title
    setWindowTitle("Simple Notepad");
}

Notepad::~Notepad()
{
    delete ui;
}

void Notepad::on_saveB_clicked()
{
    // Open the standard Windows save dialog
    QString fileName = QFileDialog::getSaveFileName(this, "Save File", "", "Text Files (*.txt)");

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, "Error", "Could not save file.");
        return;
    }

    QTextStream out(&file);
    // Use the name 'text' as seen in your image
    QString content = ui->text->toPlainText();
    out << content;

    file.close();
}
