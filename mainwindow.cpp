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
#include <QAbstractItemView>
#include <QCursor>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    listModel = new QStandardItemModel(this);
    QStringList headers;
    headers << "Name";
    listModel->setHorizontalHeaderLabels(headers);
    ui->listView->setModel(listModel);

    ui->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->listView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listView, &QWidget::customContextMenuRequested, this, &MainWindow::customMenu);

    treeModel = new QStandardItemModel(this);
    treeModel->setHorizontalHeaderLabels(headers);
    ui->treeView->setModel(treeModel);
    ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(ui->listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this](const QItemSelection &selected, const QItemSelection &deselected) {
        if (!selected.isEmpty()) {
            ui->treeView->selectionModel()->clearSelection();
        }
    });

    connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this](const QItemSelection &selected, const QItemSelection &deselected) {
        if (!selected.isEmpty()) {
            ui->listView->selectionModel()->clearSelection();
        }
    });

    ui->listView->addAction(ui->actioncopy);
    ui->listView->addAction(ui->actioncut);
    ui->listView->addAction(ui->actionpaste);
    ui->listView->addAction(ui->actiondelete);
    ui->listView->addAction(ui->actionrename);

    ui->treeView->addAction(ui->actioncopy);
    ui->treeView->addAction(ui->actioncut);
    ui->treeView->addAction(ui->actionpaste);
    ui->treeView->addAction(ui->actiondelete);
    ui->treeView->addAction(ui->actionrename);

    root = nullptr;
    recycleBin = nullptr;
    clipboard = nullptr;
    isCutOperation = false;
    currentViewMode = 1;

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
    if (isCutOperation && clipboard) delete clipboard;
    delete root;
    delete recycleBin;
    delete ui;
}

void MainWindow::on_sortb_clicked() {
    QMenu menu(this);
    QAction* mode0 = menu.addAction("•");
    QAction* mode1 = menu.addAction("••");
    QAction* mode2 = menu.addAction(":");

    connect(mode0, &QAction::triggered, this, [this]() {
        currentViewMode = 0;
        refreshUI();
    });
    connect(mode1, &QAction::triggered, this, [this]() {
        currentViewMode = 1;
        refreshUI();
    });
    connect(mode2, &QAction::triggered, this, [this]() {
        currentViewMode = 2;
        refreshUI();
    });

    menu.exec(QCursor::pos());
}

void MainWindow::customMenu(const QPoint &pos) {
    QModelIndex index = ui->listView->indexAt(pos);
    if (!index.isValid()) return;

    ui->listView->setCurrentIndex(index);
    ui->treeView->clearSelection();

    OriginFile* item = (OriginFile*)index.data(Qt::UserRole + 1).value<void*>();
    if (!item) return;

    QMenu menu(this);

    if (currentDirectory == recycleBin) {
        QAction* recoverAct = menu.addAction("Recover");
        QAction* deleteAct = menu.addAction("Delete");

        connect(recoverAct, &QAction::triggered, this, [this, item]() {
            if (!item) return;
            QString path = item->getOriginalPath();
            Directory* targetParent = (Directory*)root;

            if (path != "/" && path != "") {
                QStringList parts = path.split("/", Qt::SkipEmptyParts);

                for (int i = 0; i < parts.size(); i++) {
                    bool found = false;
                    std::vector<OriginFile*> children = targetParent->getChildren();
                    for (int j = 0; j < (int)children.size(); j++) {
                        if (children[j]->getName() == parts[i] && children[j]->getIsDirectory()) {
                            targetParent = (Directory*)children[j];
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        Directory* newDir = new Directory(parts[i], targetParent);
                        targetParent->addChild(newDir);
                        targetParent = newDir;
                    }
                }
            }

            QString baseName = item->getName();
            QString ext = "";
            if (!item->getIsDirectory() && baseName.endsWith(".txt")) {
                ext = ".txt";
                baseName = baseName.left(baseName.length() - 4);
            }

            QString newName = item->getName();
            int counter = 1;
            bool duplicate = true;

            while (duplicate) {
                duplicate = false;
                std::vector<OriginFile*> children = targetParent->getChildren();
                for (int i = 0; i < (int)children.size(); i++) {
                    if (children[i]->getName() == newName) {
                        duplicate = true;
                        break;
                    }
                }
                if (duplicate) {
                    newName = baseName + " (" + QString::number(counter) + ")" + ext;
                    counter++;
                }
            }

            item->setName(newName);
            recycleBin->detachChild(item);
            item->setInRecycleBin(false);
            targetParent->addChild(item);
            item->setParent(targetParent);

            saveSystem();
            refreshUI();
        });

        connect(deleteAct, &QAction::triggered, this, &MainWindow::on_deleteb_clicked);
    } else {
        QAction* openAct = nullptr;
        QAction* editAct = nullptr;

        if (item->getIsDirectory()) {
            openAct = menu.addAction("Open");
        } else {
            editAct = menu.addAction("Edit");
        }

        QAction* favAct = nullptr;
        if (item->getIsFavorite()) {
            favAct = menu.addAction("Unfavorite");
        } else {
            favAct = menu.addAction("Favorite");
        }

        QAction* copyAct = menu.addAction("Copy");
        QAction* cutAct = menu.addAction("Cut");
        QAction* renameAct = menu.addAction("Rename");
        QAction* deleteAct = menu.addAction("Delete");

        if (openAct) {
            connect(openAct, &QAction::triggered, this, [this, index]() {
                on_listView_doubleClicked(index);
            });
        }

        if (editAct) {
            connect(editAct, &QAction::triggered, this, [this, item]() {
                if (item) {
                    Notepad* editor = new Notepad((File*)item, this);
                    editor->show();
                }
            });
        }

        if (favAct) {
            connect(favAct, &QAction::triggered, this, [this, item]() {
                if (item) {
                    item->setIsFavorite(!item->getIsFavorite());
                    saveSystem();
                    refreshUI();
                }
            });
        }

        connect(copyAct, &QAction::triggered, this, &MainWindow::on_copyb_clicked);
        connect(cutAct, &QAction::triggered, this, &MainWindow::on_cutb_clicked);
        connect(renameAct, &QAction::triggered, this, &MainWindow::on_renameb_clicked);
        connect(deleteAct, &QAction::triggered, this, &MainWindow::on_deleteb_clicked);
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

    if (currentViewMode == 2) {
        ui->listView->setViewMode(QListView::IconMode);
        ui->listView->setMovement(QListView::Static);
        ui->listView->setResizeMode(QListView::Adjust);
        ui->listView->setIconSize(QSize(64, 64));
        ui->listView->setGridSize(QSize(100, 100));
    } else {
        ui->listView->setViewMode(QListView::ListMode);
        ui->listView->setMovement(QListView::Static);
        ui->listView->setIconSize(QSize(24, 24));
        ui->listView->setGridSize(QSize());
    }

    if (currentDirectory) {
        std::vector<OriginFile*> children = currentDirectory->getChildren();
        for (size_t i = 0; i < children.size(); i++) {
            OriginFile* item = children[i];
            if (!item) continue;

            QStandardItem* listItem = new QStandardItem(item->getName());
            listItem->setData(QVariant::fromValue((void*)item));

            if (currentViewMode > 0) {
                if (item->getIsDirectory()) {
                    listItem->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
                } else {
                    listItem->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
                }
            }

            if (item->getIsDirectory()) {
                QFont font = listItem->font();
                font.setBold(true);
                listItem->setFont(font);
            }
            listModel->appendRow(listItem);
        }
    }

    QStandardItem* homeNode = new QStandardItem("Home");
    homeNode->setData(QVariant::fromValue((void*)root));

    QStandardItem* binNode = new QStandardItem("Recycle Bin");
    binNode->setData(QVariant::fromValue((void*)recycleBin));

    if (currentViewMode > 0) {
        homeNode->setIcon(style()->standardIcon(QStyle::SP_DirHomeIcon));
        binNode->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    }

    treeModel->appendRow(homeNode);
    treeModel->appendRow(binNode);

    if (root) {
        fillFavorites(root, treeModel->invisibleRootItem());
    }
}

void MainWindow::fillFavorites(OriginFile* node, QStandardItem* parentItem) {
    if (!node || !node->getIsDirectory() || !parentItem) return;

    Directory* dir = static_cast<Directory*>(node);
    std::vector<OriginFile*> children = dir->getChildren();

    for (size_t i = 0; i < children.size(); i++) {
        OriginFile* child = children[i];
        if (!child) continue;

        if (child->getIsFavorite() && !child->getInRecycleBin()) {
            QStandardItem* item = new QStandardItem(child->getName());
            item->setData(QVariant::fromValue((void*)child));

            if (currentViewMode > 0) {
                if (child->getIsDirectory()) {
                    item->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
                } else {
                    item->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
                }
            }

            parentItem->appendRow(item);
        }

        if (child->getIsDirectory()) {
            fillFavorites(child, parentItem);
        }
    }
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

void MainWindow::on_treeView_doubleClicked(const QModelIndex &index) {
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

OriginFile* MainWindow::cloneNode(OriginFile* node, Directory* parent) {
    if (!node) return nullptr;

    QByteArray buffer;
    QDataStream out(&buffer, QIODevice::WriteOnly);

    if (node->getIsDirectory()) {
        ((Directory*)node)->write(out);
    } else {
        ((File*)node)->write(out);
    }

    QDataStream in(&buffer, QIODevice::ReadOnly);
    OriginFile* clone = nullptr;

    if (node->getIsDirectory()) {
        clone = new Directory("", parent);
        ((Directory*)clone)->read(in);
    } else {
        clone = new File("", parent);
        ((File*)clone)->read(in);
    }
    clone->setParent(parent);

    return clone;
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
            for (size_t i = 0; i < children.size(); i++) {
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
                saveSystem();
                refreshUI();
            }
        }
    } else if (choice == 2) {
        bool ok;
        QString name = QInputDialog::getText(this, "New File", "Name:", QLineEdit::Normal, "", &ok);
        if (ok && !name.isEmpty()) {
            if (!name.endsWith(".txt")) name += ".txt";

            bool duplicate = false;
            std::vector<OriginFile*> children = currentDirectory->getChildren();
            for (size_t i = 0; i < children.size(); i++) {
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
                saveSystem();
                refreshUI();
            }
        }
    }
}

void MainWindow::on_deleteb_clicked() {
    QModelIndex index = ui->listView->currentIndex();
    if (!index.isValid()) index = ui->treeView->currentIndex();

    if (!index.isValid()) {
        QMessageBox::warning(this, "Selection", "Please select an item first.");
        return;
    }

    OriginFile* item = (OriginFile*)index.data(Qt::UserRole + 1).value<void*>();
    if (!item || item == root || item == recycleBin) return;

    if (currentDirectory == recycleBin) {
        recycleBin->removeChild(item);
        if (!isCutOperation && clipboard == item) {
            clipboard = nullptr;
        }
    } else {
        QString baseName = item->getName();
        QString ext = "";
        if (!item->getIsDirectory() && baseName.endsWith(".txt")) {
            ext = ".txt";
            baseName = baseName.left(baseName.length() - 4);
        }

        QString newName = item->getName();
        int counter = 1;
        bool duplicate = true;

        while (duplicate) {
            duplicate = false;
            std::vector<OriginFile*> children = recycleBin->getChildren();
            for (size_t i = 0; i < children.size(); i++) {
                if (children[i]->getName() == newName) {
                    duplicate = true;
                    break;
                }
            }
            if (duplicate) {
                newName = baseName + " (" + QString::number(counter) + ")" + ext;
                counter++;
            }
        }

        item->setName(newName);
        item->setOriginalPath(calculateFullPath(currentDirectory));
        item->setIsFavorite(false);
        if (item->getParent()) {
            Directory* p = (Directory*)item->getParent();
            p->detachChild(item);
        }
        recycleBin->addChild(item);
        item->setInRecycleBin(true);
        item->setParent(recycleBin);
    }

    saveSystem();
    refreshUI();
}

void MainWindow::on_copyb_clicked() {
    QModelIndex index = ui->listView->currentIndex();
    if (!index.isValid()) index = ui->treeView->currentIndex();

    if (!index.isValid()) {
        QMessageBox::warning(this, "Selection", "Please select an item first.");
        return;
    }

    OriginFile* item = (OriginFile*)index.data(Qt::UserRole + 1).value<void*>();
    if (!item || item == recycleBin || item == root) return;

    if (isCutOperation && clipboard) {
        delete clipboard;
    }

    clipboard = item;
    isCutOperation = false;
}

void MainWindow::on_cutb_clicked() {
    QModelIndex index = ui->listView->currentIndex();
    if (!index.isValid()) index = ui->treeView->currentIndex();

    if (!index.isValid()) {
        QMessageBox::warning(this, "Selection", "Please select an item first.");
        return;
    }

    OriginFile* item = (OriginFile*)index.data(Qt::UserRole + 1).value<void*>();
    if (!item || item == recycleBin || item == root) return;

    if (isCutOperation && clipboard) {
        delete clipboard;
    }

    clipboard = cloneNode(item, nullptr);
    isCutOperation = true;

    on_deleteb_clicked();
}

void MainWindow::on_pasteb_clicked() {
    if (!clipboard || currentDirectory == recycleBin) {
        if (currentDirectory == recycleBin) {
            QMessageBox::warning(this, "Action Not Allowed", "Cannot paste inside Recycle Bin.");
        }
        return;
    }

    if (!isCutOperation && clipboard->getIsDirectory()) {
        OriginFile* temp = currentDirectory;
        while (temp != nullptr) {
            if (temp == clipboard) {
                QMessageBox::warning(this, "Error", "Cannot paste a directory into itself.");
                return;
            }
            temp = temp->getParent();
        }
    }

    QString baseName = clipboard->getName();
    QString ext = "";
    if (!clipboard->getIsDirectory() && baseName.endsWith(".txt")) {
        ext = ".txt";
        baseName = baseName.left(baseName.length() - 4);
    }

    QString newName = clipboard->getName();
    int counter = 1;
    bool duplicate = true;

    while (duplicate) {
        duplicate = false;
        std::vector<OriginFile*> children = currentDirectory->getChildren();
        for (size_t i = 0; i < children.size(); i++) {
            if (children[i]->getName() == newName) {
                duplicate = true;
                break;
            }
        }
        if (duplicate) {
            newName = baseName + " (" + QString::number(counter) + ")" + ext;
            counter++;
        }
    }

    OriginFile* clone = cloneNode(clipboard, currentDirectory);
    if (clone) {
        clone->setName(newName);
        currentDirectory->addChild(clone);
    }

    if (isCutOperation) {
        delete clipboard;
        clipboard = nullptr;
        isCutOperation = false;
    }

    saveSystem();
    refreshUI();
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

void MainWindow::on_renameb_clicked() {
    QModelIndex index = ui->listView->currentIndex();
    if (!index.isValid()) index = ui->treeView->currentIndex();

    if (!index.isValid()) {
        QMessageBox::warning(this, "Selection", "Please select an item first.");
        return;
    }

    OriginFile* item = (OriginFile*)index.data(Qt::UserRole + 1).value<void*>();
    if (!item || item == root || item == recycleBin) return;

    bool ok;
    QString oldName = item->getName();
    QString newName = QInputDialog::getText(this, "Rename", "New Name:", QLineEdit::Normal, oldName, &ok);

    if (ok && !newName.isEmpty() && newName != oldName) {
        if (!item->getIsDirectory() && !newName.endsWith(".txt")) {
            newName += ".txt";
        }

        bool duplicate = false;
        std::vector<OriginFile*> children = currentDirectory->getChildren();
        for (size_t i = 0; i < children.size(); i++) {
            if (children[i]->getName() == newName) {
                duplicate = true;
                break;
            }
        }

        if (duplicate) {
            QMessageBox::warning(this, "Error", "A file or folder with this name already exists.");
        } else {
            item->setName(newName);
            saveSystem();
            refreshUI();
        }
    }
}

void MainWindow::on_detailsd_clicked() {
    QModelIndex index = ui->listView->currentIndex();
    if (!index.isValid()) index = ui->treeView->currentIndex();

    if (!index.isValid()) {
        QMessageBox::warning(this, "Selection", "Please select an item first.");
        return;
    }

    OriginFile* item = (OriginFile*)index.data(Qt::UserRole + 1).value<void*>();
    if (!item || item == root || item == recycleBin) return;

    if (item->getIsDirectory()) {
        long totalSize = 0;
        int fileCount = 0;

        std::vector<Directory*> stack;
        stack.push_back((Directory*)item);

        while (!stack.empty()) {
            Directory* current = stack.back();
            stack.pop_back();

            std::vector<OriginFile*> children = current->getChildren();
            for (size_t i = 0; i < children.size(); i++) {
                if (children[i]->getIsDirectory()) {
                    stack.push_back((Directory*)children[i]);
                } else {
                    fileCount++;
                    totalSize += ((File*)children[i])->getSize();
                }
            }
        }

        QString info = "Name: " + item->getName() + "\n";
        info += "Files inside: " + QString::number(fileCount) + "\n";
        info += "Total size: " + QString::number(totalSize) + " bytes";

        QMessageBox::information(this, "Properties", info);
    } else {
        File* fileItem = (File*)item;
        QString info = "Name: " + fileItem->getName() + "\n";
        info += "Size: " + QString::number(fileItem->getSize()) + " bytes";

        QMessageBox::information(this, "Properties", info);
    }
}

void MainWindow::on_actionpaste_triggered() {
    on_pasteb_clicked();
}

void MainWindow::on_actioncut_triggered() {
    on_cutb_clicked();
}

void MainWindow::on_actioncopy_triggered() {
    on_copyb_clicked();
}

void MainWindow::on_actionrename_triggered() {
    on_renameb_clicked();
}

void MainWindow::on_actiondelete_triggered() {
    on_deleteb_clicked();
}
