#ifndef NOTEPAD_H
#define NOTEPAD_H

#include <QMainWindow>
#include "file.h"

namespace Ui { class Notepad; }

class Notepad : public QMainWindow {
    Q_OBJECT

public:
    explicit Notepad(File* file, QWidget *parent = nullptr);
    ~Notepad();

private slots:
    void on_saveB_clicked();

private:
    Ui::Notepad *ui;
    File* currentFile;
};

#endif // NOTEPAD_H
