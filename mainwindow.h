#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QDir>
#include "navigationhistory.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_directoryb_clicked();
    void on_fileb_clicked();
    void on_deleteb_clicked();
    void on_copyb_clicked();
    void on_cutb_clicked();
    void on_pasteb_clicked();
    void on_backwardb_clicked();
    void on_forwardb_clicked();
    void on_parentb_clicked();
    void on_listView_doubleClicked(const QModelIndex &index);
    void showContextMenu(const QPoint &pos);

private:
    Ui::MainWindow *ui;
    QStandardItemModel* listModel;
    NavigationHistory history;

    QString currentPath;
    QString clipboardPath;
    bool isCutOperation;

    void extracted(QFileInfoList &list);
    void refreshUI();
    void updateButtons();
};
#endif // MAINWINDOW_H
