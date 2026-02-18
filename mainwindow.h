#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include "navigationhistory.h"
#include "originfile.h"
#include "directory.h"
#include "file.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_createb_clicked();
    void on_deleteb_clicked();
    void on_copyb_clicked();
    void on_cutb_clicked();
    void on_pasteb_clicked();
    void on_backwardb_clicked();
    void on_forwardb_clicked();
    void on_parentb_clicked();
    void on_listView_doubleClicked(const QModelIndex &index);
    void on_treeView_doubleClicked(const QModelIndex &index);
    void customMenu(const QPoint &pos);

    void on_renameb_clicked();

private:
    Ui::MainWindow *ui;

    NavigationHistory history;
    OriginFile* root;
    Directory* currentDirectory;
    Directory* recycleBin;
    OriginFile* clipboard;
    bool isCutOperation;

    QStandardItemModel* listModel;
    QStandardItemModel* treeModel;

private:
    void triggerRename(OriginFile* item);
    void refreshUI();
    void saveSystem();
    void loadSystem();
    void setupModels();
    void fillTreeRecursive(OriginFile* node, QStandardItem* parentItem);
    bool checkDuplicateName(QString name);
    QString calculateFullPath(OriginFile* node);
};
#endif // MAINWINDOW_H
