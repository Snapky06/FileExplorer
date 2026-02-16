#ifndef NOTEPAD_H
#define NOTEPAD_H

#include <QMainWindow>

namespace Ui { class Notepad; }

class Notepad : public QMainWindow {
    Q_OBJECT

public:
    explicit Notepad(QString filePath, QWidget *parent = nullptr);
    ~Notepad();

private slots:
    void on_saveButton_clicked();
    void on_closeButton_clicked();

private:
    Ui::Notepad *ui;
    QString filePath;
    void loadFile();
};
#endif // NOTEPAD_H
