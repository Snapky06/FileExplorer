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

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

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
    void on_sortb_clicked();
    void on_detailsd_clicked();
    void on_enterb_clicked();
    void on_search_returnPressed();
    void customMenu(const QPoint &pos);

    void on_actionpaste_triggered();
    void on_actioncut_triggered();
    void on_actioncopy_triggered();
    void on_actionrename_triggered();
    void on_actiondelete_triggered();

private:
    Ui::MainWindow *ui;

    Directory* root;
    Directory* recycleBin;
    Directory* currentDirectory;

    OriginFile* clipboard;
    bool isCutOperation;
    int currentViewMode;

    NavigationHistory history;

    QStandardItemModel* listModel;
    QStandardItemModel* treeModel;

    void refreshUI();
    QString calculateFullPath(OriginFile* node);
    void fillFavorites(OriginFile* node, QStandardItem* parentItem);
    void saveSystem();
    void loadSystem();
    OriginFile* cloneNode(OriginFile* node, Directory* parent);
};

#endif // MAINWINDOW_H
