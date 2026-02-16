#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "notepad.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QMenu>
#include <QUiLoader>
#include <QWidgetAction>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    listModel = new QStandardItemModel(this);
    ui->listView->setModel(listModel);
    ui->listView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->listView, &QWidget::customContextMenuRequested, this, &MainWindow::showContextMenu);

    QDir dir(QDir::currentPath());
    if (!dir.exists("MisArchivos")) {
        dir.mkdir("MisArchivos");
    }

    currentPath = dir.absoluteFilePath("MisArchivos");
    history.addVisit(currentPath);

    refreshUI();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::refreshUI() {
    listModel->clear();
    QDir dir(currentPath);
    QFileInfoList list = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::DirsFirst);

    for (const QFileInfo &fileInfo : list) {
        QStandardItem* item = new QStandardItem(fileInfo.fileName());
        if (fileInfo.isDir()) {
            QFont font = item->font();
            font.setBold(true);
            item->setFont(font);
        }
        listModel->appendRow(item);
    }
    ui->pathline->setText(currentPath);
    updateButtons();
}

void MainWindow::updateButtons() {
}

void MainWindow::on_directoryb_clicked() {
    bool ok;
    QString name = QInputDialog::getText(this, "New Folder", "Name:", QLineEdit::Normal, "", &ok);
    if (ok && !name.isEmpty()) {
        QDir dir(currentPath);
        dir.mkdir(name);
        refreshUI();
    }
}

void MainWindow::on_fileb_clicked() {
    bool ok;
    QString name = QInputDialog::getText(this, "New File", "Name:", QLineEdit::Normal, "", &ok);
    if (ok && !name.isEmpty()) {
        if (!name.endsWith(".txt")) name += ".txt";
        QFile file(currentPath + "/" + name);
        if (file.open(QIODevice::WriteOnly)) {
            file.close();
        }
        refreshUI();
    }
}

void MainWindow::on_deleteb_clicked() {
    QModelIndex index = ui->listView->currentIndex();
    if (!index.isValid()) return;

    QString name = index.data().toString();
    QString fullPath = currentPath + "/" + name;
    QFileInfo info(fullPath);

    if (info.isDir()) {
        QDir dir(fullPath);
        dir.removeRecursively();
    } else {
        QFile::remove(fullPath);
    }
    refreshUI();
}

void MainWindow::on_copyb_clicked() {
    QModelIndex index = ui->listView->currentIndex();
    if (index.isValid()) {
        clipboardPath = currentPath + "/" + index.data().toString();
        isCutOperation = false;
    }
}

void MainWindow::on_cutb_clicked() {
    QModelIndex index = ui->listView->currentIndex();
    if (index.isValid()) {
        clipboardPath = currentPath + "/" + index.data().toString();
        isCutOperation = true;
    }
}

void MainWindow::on_pasteb_clicked() {
    if (clipboardPath.isEmpty()) return;

    QFileInfo info(clipboardPath);
    QString destPath = currentPath + "/" + info.fileName();

    int i = 1;
    QString base = currentPath + "/" + info.baseName();
    QString suffix = info.suffix();
    while (QFile::exists(destPath)) {
        if (info.isDir()) destPath = base + "_copy" + QString::number(i);
        else destPath = base + "_copy" + QString::number(i) + "." + suffix;
        i++;
    }

    if (info.isDir()) {
        if (isCutOperation) QDir().rename(clipboardPath, destPath);
        else QDir().mkdir(destPath);
    } else {
        if (isCutOperation) QFile::rename(clipboardPath, destPath);
        else QFile::copy(clipboardPath, destPath);
    }

    if (isCutOperation) clipboardPath.clear();
    refreshUI();
}

void MainWindow::on_backwardb_clicked() {
    if (history.canGoBack()) {
        currentPath = history.goBack();
        refreshUI();
    }
}

void MainWindow::on_forwardb_clicked() {
    if (history.canGoForward()) {
        currentPath = history.goForward();
        refreshUI();
    }
}

void MainWindow::on_parentb_clicked() {
    QDir dir(currentPath);
    if (dir.cdUp()) {
        currentPath = dir.absolutePath();
        history.addVisit(currentPath);
        refreshUI();
    }
}

void MainWindow::on_listView_doubleClicked(const QModelIndex &index) {
    QString name = index.data().toString();
    QString fullPath = currentPath + "/" + name;
    QFileInfo info(fullPath);

    if (info.isDir()) {
        currentPath = fullPath;
        history.addVisit(currentPath);
        refreshUI();
    } else if (name.endsWith(".txt")) {
        Notepad *editor = new Notepad(fullPath, this);
        editor->show();
    }
}

void MainWindow::showContextMenu(const QPoint &pos) {
    QModelIndex index = ui->listView->indexAt(pos);
    if (!index.isValid()) return;

    QUiLoader loader;
    QFile uiFile("minint.ui");
    if (uiFile.open(QFile::ReadOnly)) {
        QWidget* widget = loader.load(&uiFile, this);
        uiFile.close();
        if (widget) {
            QMenu menu;
            QWidgetAction* action = new QWidgetAction(&menu);
            action->setDefaultWidget(widget);
            menu.addAction(action);
            menu.exec(ui->listView->mapToGlobal(pos));
        }
    }
}
