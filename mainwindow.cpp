#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "notepad.h"
#include <QFile>
#include <QDataStream>
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStyle>
#include <QMenu>
#include <QAction>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    setupModels();

    root = nullptr;
    recycleBin = nullptr;
    clipboard = nullptr;
    isCutOperation = false;

    loadSystem();

    if (root == nullptr) {
        root = new Directory("/", nullptr);
        recycleBin = new Directory("Recycle Bin", nullptr);
    }

    if (root->getName() == "Root") {
        root->setName("/");
        saveSystem();
    }

    currentDirectory = (Directory*)root;
    refreshUI();
}

MainWindow::~MainWindow() {
    saveSystem();
    delete root;
    delete recycleBin;
    delete ui;
}

void MainWindow::setupModels() {
    listModel = new QStandardItemModel(this);
    QStringList headers;
    headers << "Name";
    listModel->setHorizontalHeaderLabels(headers);
    ui->listView->setModel(listModel);

    ui->listView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listView, &QWidget::customContextMenuRequested, this, &MainWindow::customMenu);

    treeModel = new QStandardItemModel(this);
    treeModel->setHorizontalHeaderLabels(headers);
    ui->treeView->setModel(treeModel);
}

void MainWindow::customMenu(const QPoint &pos) {
    QModelIndex index = ui->listView->indexAt(pos);
    if (!index.isValid()) return;

    OriginFile* item = (OriginFile*)index.data(Qt::UserRole + 1).value<void*>();
    if (!item) return;

    QMenu menu(this);
    QAction* openAct = menu.addAction("Open");

    QAction* editAct = nullptr;
    if (!item->getIsDirectory()) {
        editAct = menu.addAction("Edit");
    }

    QAction* renameAct = menu.addAction("Rename");

    connect(openAct, &QAction::triggered, this, [this, index]() {
        on_listView_doubleClicked(index);
    });

    if (editAct) {
        connect(editAct, &QAction::triggered, this, [this, item]() {
            Notepad* editor = new Notepad((File*)item, this);
            editor->show();
        });
    }

    connect(renameAct, &QAction::triggered, this, [this, item]() {
        bool ok;
        QString oldName = item->getName();
        QString newName = QInputDialog::getText(this, "Rename", "New Name:", QLineEdit::Normal, oldName, &ok);

        if (ok && !newName.isEmpty() && newName != oldName) {
            if (checkDuplicateName(newName)) {
                QMessageBox::warning(this, "Error", "A file or folder with this name already exists.");
            } else {
                item->setName(newName);
                refreshUI();
                saveSystem();
            }
        }
    });

    menu.exec(ui->listView->mapToGlobal(pos));
}

void MainWindow::refreshUI() {
    listModel->clear();
    treeModel->clear();

    QStringList headers;
    headers << "Name";
    treeModel->setHorizontalHeaderLabels(headers);
    listModel->setHorizontalHeaderLabels(headers);

    ui->pathline->setText(calculateFullPath(currentDirectory));

    std::vector<OriginFile*> children = currentDirectory->getChildren();
    for (int i = 0; i < (int)children.size(); i++) {
        OriginFile* item = children[i];
        QStandardItem* listItem = new QStandardItem(item->getName());
        listItem->setData(QVariant::fromValue((void*)item));

        if (item->getIsDirectory()) {
            listItem->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
            QFont font = listItem->font();
            font.setBold(true);
            listItem->setFont(font);
        } else {
            listItem->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
        }
        listModel->appendRow(listItem);
    }

    QStandardItem* favRoot = new QStandardItem("Favorites");
    favRoot->setIcon(style()->standardIcon(QStyle::SP_DirLinkIcon));

    QStandardItem* binRoot = new QStandardItem("Recycle Bin");
    binRoot->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));

    fillTreeRecursive(root, favRoot);
    fillTreeRecursive(recycleBin, binRoot);

    treeModel->appendRow(favRoot);
    treeModel->appendRow(binRoot);
    ui->treeView->expandAll();
}

QString MainWindow::calculateFullPath(OriginFile* node) {
    if (!node) return "";
    if (node->getParent() == nullptr) return node->getName();

    QString parentPath = calculateFullPath(node->getParent());

    if (parentPath == "/") return parentPath + node->getName();

    return parentPath + "/" + node->getName();
}

void MainWindow::on_listView_doubleClicked(const QModelIndex &index) {
    OriginFile* item = (OriginFile*)index.data(Qt::UserRole + 1).value<void*>();

    if (item) {
        if (item->getIsDirectory()) {
            currentDirectory = (Directory*)item;
            refreshUI();
        } else {
            Notepad* editor = new Notepad((File*)item, this);
            editor->show();
        }
    }
}

void MainWindow::fillTreeRecursive(OriginFile* node, QStandardItem* parentItem) {
    if (!node) return;

    if (node->getIsDirectory()) {
        if (node->getIsFavorite() && !node->getInRecycleBin()) {
            QStandardItem* item = new QStandardItem(node->getName());
            item->setData(QVariant::fromValue((void*)node));
            item->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
            parentItem->appendRow(item);

            Directory* dir = (Directory*)node;
            std::vector<OriginFile*> children = dir->getChildren();
            for (int i = 0; i < (int)children.size(); i++) {
                fillTreeRecursive(children[i], item);
            }
        } else {
            Directory* dir = (Directory*)node;
            std::vector<OriginFile*> children = dir->getChildren();
            for (int i = 0; i < (int)children.size(); i++) {
                fillTreeRecursive(children[i], parentItem);
            }
        }
    }
}

void MainWindow::saveSystem() {
    QFile file("system.bin");
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        if (root) root->write(out);
        if (recycleBin) recycleBin->write(out);
        file.close();
    }
}

void MainWindow::loadSystem() {
    QFile file("system.bin");
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);

        root = new Directory("", nullptr);
        root->read(in);

        if (!in.atEnd()) {
            recycleBin = new Directory("", nullptr);
            recycleBin->read(in);
        }
        file.close();
    }
}

bool MainWindow::checkDuplicateName(QString name) {
    std::vector<OriginFile*> children = currentDirectory->getChildren();
    for (int i = 0; i < (int)children.size(); i++) {
        if (children[i]->getName() == name) return true;
    }
    return false;
}

void MainWindow::on_createb_clicked() {
    QDialog dialog(this);
    dialog.setWindowTitle("Create New");
    dialog.setFixedSize(300, 100);

    QHBoxLayout* layout = new QHBoxLayout(&dialog);

    QPushButton* dirBtn = new QPushButton("Directory", &dialog);
    dirBtn->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
    dirBtn->setIconSize(QSize(32, 32));
    dirBtn->setMinimumHeight(60);

    QPushButton* fileBtn = new QPushButton("File", &dialog);
    fileBtn->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
    fileBtn->setIconSize(QSize(32, 32));
    fileBtn->setMinimumHeight(60);

    layout->addWidget(dirBtn);
    layout->addWidget(fileBtn);

    int choice = 0;

    connect(dirBtn, &QPushButton::clicked, [&]() { choice = 1; dialog.accept(); });
    connect(fileBtn, &QPushButton::clicked, [&]() { choice = 2; dialog.accept(); });

    dialog.exec();

    if (choice == 1) {
        bool ok;
        QString name = QInputDialog::getText(this, "New Directory", "Name:", QLineEdit::Normal, "", &ok);
        if (ok && !name.isEmpty()) {
            if (checkDuplicateName(name)) {
                QMessageBox::warning(this, "Error", "A folder with this name already exists.");
            } else {
                Directory* newDir = new Directory(name, currentDirectory);
                currentDirectory->addChild(newDir);
                refreshUI();
                saveSystem();
            }
        }
    } else if (choice == 2) {
        bool ok;
        QString name = QInputDialog::getText(this, "New File", "Name:", QLineEdit::Normal, "", &ok);
        if (ok && !name.isEmpty()) {
            if (!name.endsWith(".txt")) name += ".txt";

            if (checkDuplicateName(name)) {
                QMessageBox::warning(this, "Error", "A file with this name already exists.");
            } else {
                File* newFile = new File(name, currentDirectory);
                currentDirectory->addChild(newFile);
                refreshUI();
                saveSystem();
            }
        }
    }
}

void MainWindow::on_deleteb_clicked() {
}

void MainWindow::on_copyb_clicked() {
}

void MainWindow::on_cutb_clicked() {
}

void MainWindow::on_pasteb_clicked() {
}

void MainWindow::on_backwardb_clicked() {
}

void MainWindow::on_forwardb_clicked() {
}

void MainWindow::on_parentb_clicked() {
    if (currentDirectory && currentDirectory->getParent()) {
        currentDirectory = (Directory*)currentDirectory->getParent();
        refreshUI();
    }
}

void MainWindow::on_treeView_doubleClicked(const QModelIndex &index) {
}

