#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include "directory.h"
#include "file.h"
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
    void on_listView_doubleClicked(const QModelIndex &index);
    void on_treeView_doubleClicked(const QModelIndex &index);
    void on_createb_clicked();
    void on_deleteb_clicked();
    void on_copyb_clicked();
    void on_cutb_clicked();
    void on_pasteb_clicked();
    void on_backwardb_clicked();
    void on_forwardb_clicked();
    void on_parentb_clicked();
    void on_renameb_clicked();
    void customMenu(const QPoint &pos);

private:
    Ui::MainWindow *ui;

    Directory* root;
    Directory* recycleBin;
    Directory* currentDirectory;

    OriginFile* clipboard;
    bool isCutOperation;

    NavigationHistory history;

    QStandardItemModel* listModel;
    QStandardItemModel* treeModel;

    void refreshUI();
    QString calculateFullPath(OriginFile* node);
    void fillTreeRecursive(OriginFile* node, QStandardItem* parentItem, bool showFiles);
    void fillFavorites(OriginFile* node, QStandardItem* favRoot);
    void saveSystem();
    void loadSystem();
};

#endif // MAINWINDOW_H
