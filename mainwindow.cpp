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
#include <QCoreApplication>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

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
    history.addVisit(currentDirectory);
    refreshUI();
}

MainWindow::~MainWindow() {
    saveSystem();
    delete root;
    delete recycleBin;
    delete ui;
}

void MainWindow::customMenu(const QPoint &pos) {
    QModelIndex index = ui->listView->indexAt(pos);
    if (!index.isValid()) return;

    OriginFile* item = (OriginFile*)index.data(Qt::UserRole + 1).value<void*>();
    if (!item) return;

    QMenu menu(this);

    if (currentDirectory == recycleBin) {
        QAction* recoverAct = menu.addAction("Recover");
        QAction* deleteAct = menu.addAction("Delete");

        connect(recoverAct, &QAction::triggered, this, [this, item]() {
            QString path = item->getOriginalPath();
            OriginFile* targetParent = root;

            if (path != "/" && path != "") {
                QStringList parts = path.split("/", Qt::SkipEmptyParts);
                Directory* temp = (Directory*)root;
                bool pathFound = true;

                for (int i = 0; i < parts.size(); i++) {
                    bool found = false;
                    std::vector<OriginFile*> children = temp->getChildren();
                    for (int j = 0; j < (int)children.size(); j++) {
                        if (children[j]->getName() == parts[i] && children[j]->getIsDirectory()) {
                            temp = (Directory*)children[j];
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        pathFound = false;
                        break;
                    }
                }
                if (pathFound) {
                    targetParent = temp;
                }
            }

            recycleBin->detachChild(item);
            item->setInRecycleBin(false);
            ((Directory*)targetParent)->addChild(item);
            item->setParent(targetParent);
            refreshUI();
            saveSystem();
        });

        connect(deleteAct, &QAction::triggered, this, [this, item]() {
            recycleBin->removeChild(item);
            refreshUI();
            saveSystem();
        });
    } else {
        QAction* openAct = nullptr;
        QAction* editAct = nullptr;

        if (item->getIsDirectory()) {
            openAct = menu.addAction("Open");
        } else {
            editAct = menu.addAction("Edit");
        }

        QAction* renameAct = menu.addAction("Rename");
        QAction* deleteAct = menu.addAction("Delete");

        if (openAct) {
            connect(openAct, &QAction::triggered, this, [this, index]() {
                on_listView_doubleClicked(index);
            });
        }

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
                if (!item->getIsDirectory() && !newName.endsWith(".txt")) {
                    newName += ".txt";
                }

                bool duplicate = false;
                std::vector<OriginFile*> children = currentDirectory->getChildren();
                for (int i = 0; i < (int)children.size(); i++) {
                    if (children[i]->getName() == newName) {
                        duplicate = true;
                        break;
                    }
                }

                if (duplicate) {
                    QMessageBox::warning(this, "Error", "A file or folder with this name already exists.");
                } else {
                    item->setName(newName);
                    refreshUI();
                    saveSystem();
                }
            }
        });

        connect(deleteAct, &QAction::triggered, this, [this, item]() {
            item->setOriginalPath(calculateFullPath(currentDirectory));
            currentDirectory->detachChild(item);
            recycleBin->addChild(item);
            item->setInRecycleBin(true);
            item->setParent(recycleBin);
            refreshUI();
            saveSystem();
        });
    }

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

    QStandardItem* specialFolders = new QStandardItem("Special Folders");
    QFont sfFont = specialFolders->font();
    sfFont.setBold(true);
    specialFolders->setFont(sfFont);

    QStandardItem* homeNode = new QStandardItem("Home");
    homeNode->setIcon(style()->standardIcon(QStyle::SP_DirHomeIcon));
    homeNode->setData(QVariant::fromValue((void*)root));

    QStandardItem* favNode = new QStandardItem("Favorites");
    favNode->setIcon(style()->standardIcon(QStyle::SP_DirLinkIcon));

    QStandardItem* binNode = new QStandardItem("Recycle Bin");
    binNode->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    binNode->setData(QVariant::fromValue((void*)recycleBin));

    specialFolders->appendRow(homeNode);
    specialFolders->appendRow(favNode);
    specialFolders->appendRow(binNode);

    treeModel->appendRow(specialFolders);

    fillTreeRecursive(root, homeNode, false);
    fillFavorites(root, favNode);
    fillTreeRecursive(recycleBin, binNode, true);

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
    if (item != nullptr) {
        if (item->getIsDirectory()) {
            currentDirectory = (Directory*)item;
            history.addVisit(currentDirectory);
            refreshUI();
        } else {
            Notepad* editor = new Notepad((File*)item, this);
            editor->show();
        }
    }
}

void MainWindow::fillTreeRecursive(OriginFile* node, QStandardItem* parentItem, bool showFiles) {
    if (!node || !parentItem) return;

    if (node->getIsDirectory()) {
        Directory* dir = (Directory*)node;
        std::vector<OriginFile*> children = dir->getChildren();
        for (int i = 0; i < (int)children.size(); i++) {
            OriginFile* child = children[i];

            if (child->getIsDirectory() || showFiles) {
                QStandardItem* item = new QStandardItem(child->getName());
                item->setData(QVariant::fromValue((void*)child));

                if (child->getIsDirectory()) {
                    item->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
                    fillTreeRecursive(child, item, showFiles);
                } else {
                    item->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
                }

                parentItem->appendRow(item);
            }
        }
    }
}

void MainWindow::fillFavorites(OriginFile* node, QStandardItem* favRoot) {
    if (!node || !favRoot) return;

    if (node->getIsDirectory()) {
        Directory* dir = (Directory*)node;
        std::vector<OriginFile*> children = dir->getChildren();
        for (int i = 0; i < (int)children.size(); i++) {
            OriginFile* child = children[i];

            if (child->getIsFavorite() && !child->getInRecycleBin()) {
                QStandardItem* item = new QStandardItem(child->getName());
                item->setData(QVariant::fromValue((void*)child));
                if (child->getIsDirectory()) {
                    item->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
                } else {
                    item->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
                }
                favRoot->appendRow(item);
            }

            if (child->getIsDirectory()) {
                fillFavorites(child, favRoot);
            }
        }
    }
}

void MainWindow::saveSystem() {
    QString path = QCoreApplication::applicationDirPath() + "/system.bin";
    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        if (root) root->write(out);
        if (recycleBin) recycleBin->write(out);
        file.close();
    }
}

void MainWindow::loadSystem() {
    QString path = QCoreApplication::applicationDirPath() + "/system.bin";
    QFile file(path);
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
            bool duplicate = false;
            std::vector<OriginFile*> children = currentDirectory->getChildren();
            for (int i = 0; i < (int)children.size(); i++) {
                if (children[i]->getName() == name) {
                    duplicate = true;
                    break;
                }
            }

            if (duplicate) {
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

            bool duplicate = false;
            std::vector<OriginFile*> children = currentDirectory->getChildren();
            for (int i = 0; i < (int)children.size(); i++) {
                if (children[i]->getName() == name) {
                    duplicate = true;
                    break;
                }
            }

            if (duplicate) {
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
    QModelIndex index = ui->listView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Selection", "Please select an item first.");
        return;
    }

    OriginFile* item = (OriginFile*)index.data(Qt::UserRole + 1).value<void*>();
    if (!item) return;

    if (currentDirectory == recycleBin) {
        recycleBin->removeChild(item);
    } else {
        item->setOriginalPath(calculateFullPath(currentDirectory));
        currentDirectory->detachChild(item);
        recycleBin->addChild(item);
        item->setInRecycleBin(true);
        item->setParent(recycleBin);
    }
    refreshUI();
    saveSystem();
}

void MainWindow::on_copyb_clicked() {
}

void MainWindow::on_cutb_clicked() {
}

void MainWindow::on_pasteb_clicked() {
}

void MainWindow::on_backwardb_clicked() {
    OriginFile* prev = history.goBack();
    if (prev != nullptr) {
        currentDirectory = (Directory*)prev;
        refreshUI();
    }
}

void MainWindow::on_forwardb_clicked() {
    OriginFile* next = history.goForward();
    if (next != nullptr) {
        currentDirectory = (Directory*)next;
        refreshUI();
    }
}

void MainWindow::on_parentb_clicked() {
    if (currentDirectory && currentDirectory->getParent()) {
        currentDirectory = (Directory*)currentDirectory->getParent();
        refreshUI();
    }
}

void MainWindow::on_treeView_doubleClicked(const QModelIndex &index) {
    OriginFile* item = (OriginFile*)index.data(Qt::UserRole + 1).value<void*>();
    if (item != nullptr && item->getIsDirectory()) {
        currentDirectory = (Directory*)item;
        history.addVisit(currentDirectory);
        refreshUI();
    }
}

void MainWindow::on_renameb_clicked() {
    QModelIndex index = ui->listView->currentIndex();
    if (index.isValid()) {
        OriginFile* item = (OriginFile*)index.data(Qt::UserRole + 1).value<void*>();
        if (!item) return;

        bool ok;
        QString oldName = item->getName();
        QString newName = QInputDialog::getText(this, "Rename", "New Name:", QLineEdit::Normal, oldName, &ok);

        if (ok && !newName.isEmpty() && newName != oldName) {
            if (!item->getIsDirectory() && !newName.endsWith(".txt")) {
                newName += ".txt";
            }

            bool duplicate = false;
            std::vector<OriginFile*> children = currentDirectory->getChildren();
            for (int i = 0; i < (int)children.size(); i++) {
                if (children[i]->getName() == newName) {
                    duplicate = true;
                    break;
                }
            }

            if (duplicate) {
                QMessageBox::warning(this, "Error", "A file or folder with this name already exists.");
            } else {
                item->setName(newName);
                refreshUI();
                saveSystem();
            }
        }
    } else {
        QMessageBox::warning(this, "Selection", "Please select an item first.");
    }
}
