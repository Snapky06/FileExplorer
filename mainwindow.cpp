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
#include <QLabel>
#include <QListWidget>
#include <QCoreApplication>
#include <QAbstractItemView>
#include <QCursor>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    listModel = new QStandardItemModel(this);
    listModel->setHorizontalHeaderLabels({"Name"});
    ui->listView->setModel(listModel);

    ui->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->listView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listView, &QWidget::customContextMenuRequested, this, &MainWindow::customMenu);

    treeModel = new QStandardItemModel(this);
    treeModel->setHorizontalHeaderLabels({""});
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

    ui->listView->setDragEnabled(true);
    ui->listView->setAcceptDrops(true);
    ui->listView->setDropIndicatorShown(true);
    ui->listView->setDragDropMode(QAbstractItemView::DragDrop);
    ui->listView->setDefaultDropAction(Qt::MoveAction);
    ui->listView->viewport()->installEventFilter(this);
    ui->parentb->installEventFilter(this);
    connect(ui->pathline, &QLineEdit::returnPressed, this, &MainWindow::on_enterb_clicked);
    connect(ui->search, &QLineEdit::returnPressed, this, &MainWindow::on_search_returnPressed);
    connect(ui->enterb, &QPushButton::clicked, this, [this]() {
        if (ui->search->hasFocus()) on_search_returnPressed();
        else on_enterb_clicked();
    });

    ui->treeView->setDragEnabled(false);
    ui->treeView->setAcceptDrops(false);

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
    delete root;
    delete recycleBin;
    delete ui;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if (watched == ui->parentb && event->type() == QEvent::DragEnter) {
        static_cast<QDragEnterEvent*>(event)->acceptProposedAction();
        return true;
    }

    if (watched == ui->parentb && event->type() == QEvent::Drop) {
        QModelIndexList selected = ui->listView->selectionModel()->selectedIndexes();
        if (!selected.isEmpty()) {
            QModelIndex sourceIndex = selected.first();
            OriginFile* draggedItem = reinterpret_cast<OriginFile*>(static_cast<uintptr_t>(sourceIndex.data(Qt::UserRole + 1).toULongLong()));
            if (draggedItem && draggedItem != root && draggedItem != recycleBin && currentDirectory->getParent()) {
                Directory* targetDir = (Directory*)currentDirectory->getParent();
                if (draggedItem->getParent()) ((Directory*)draggedItem->getParent())->detachChild(draggedItem);
                targetDir->addChild(draggedItem);
                draggedItem->setParent(targetDir);
                saveSystem();
                refreshUI();
            }
        }
        static_cast<QDropEvent*>(event)->accept();
        return true;
    }

    if (watched == ui->listView->viewport() && event->type() == QEvent::Drop) {
        QDropEvent *dropEvent = static_cast<QDropEvent*>(event);

        QModelIndex targetIndex = ui->listView->indexAt(dropEvent->position().toPoint());
        QModelIndexList selected = ui->listView->selectionModel()->selectedIndexes();

        if (selected.isEmpty()) {
            return QMainWindow::eventFilter(watched, event);
        }

        QModelIndex sourceIndex = selected.first();
        if (!sourceIndex.isValid() || sourceIndex == targetIndex) {
            dropEvent->setDropAction(Qt::IgnoreAction);
            dropEvent->accept();
            return true;
        }

        OriginFile* draggedItem = reinterpret_cast<OriginFile*>(static_cast<uintptr_t>(sourceIndex.data(Qt::UserRole + 1).toULongLong()));
        if (!draggedItem || draggedItem == root || draggedItem == recycleBin) {
            dropEvent->setDropAction(Qt::IgnoreAction);
            dropEvent->accept();
            return true;
        }

        Directory* targetDir = currentDirectory;

        if (targetIndex.isValid()) {
            OriginFile* targetItem = reinterpret_cast<OriginFile*>(static_cast<uintptr_t>(targetIndex.data(Qt::UserRole + 1).toULongLong()));
            if (targetItem && targetItem->getIsDirectory()) {
                targetDir = (Directory*)targetItem;
            } else {
                dropEvent->setDropAction(Qt::IgnoreAction);
                dropEvent->accept();
                return true;
            }
        } else {
            dropEvent->setDropAction(Qt::IgnoreAction);
            dropEvent->accept();
            return true;
        }

        if (targetDir && targetDir != currentDirectory && targetDir != draggedItem) {
            if (draggedItem->getIsDirectory()) {
                OriginFile* temp = targetDir;
                while (temp != nullptr) {
                    if (temp == draggedItem) {
                        dropEvent->setDropAction(Qt::IgnoreAction);
                        dropEvent->accept();
                        return true;
                    }
                    temp = temp->getParent();
                }
            }

            QString baseName = draggedItem->getName();
            QString ext = "";
            if (!draggedItem->getIsDirectory() && baseName.endsWith(".txt")) {
                ext = ".txt";
                baseName = baseName.left(baseName.length() - 4);
            }

            QString newName = draggedItem->getName();
            int counter = 1;
            bool duplicate = true;

            while (duplicate) {
                duplicate = false;
                std::vector<OriginFile*> children = targetDir->getChildren();
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

            draggedItem->setName(newName);

            if (draggedItem->getParent()) {
                Directory* oldParent = (Directory*)draggedItem->getParent();
                oldParent->detachChild(draggedItem);
            }

            targetDir->addChild(draggedItem);
            draggedItem->setParent(targetDir);

            saveSystem();
            refreshUI();
        }

        dropEvent->setDropAction(Qt::IgnoreAction);
        dropEvent->accept();
        return true;
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::on_sortb_clicked() {
    QMenu menu(this);
    QAction* mode0 = menu.addAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), "List");
    QAction* mode1 = menu.addAction(style()->standardIcon(QStyle::SP_FileDialogListView), "List with icons");
    QAction* mode2 = menu.addAction(style()->standardIcon(QStyle::SP_FileDialogContentsView), "Icons");

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

    OriginFile* item = reinterpret_cast<OriginFile*>(static_cast<uintptr_t>(index.data(Qt::UserRole + 1).toULongLong()));
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

    treeModel->setHorizontalHeaderLabels({""});
    listModel->setHorizontalHeaderLabels({"Name"});

    ui->pathline->setText(calculateFullPath(currentDirectory));

    if (currentViewMode == 2) {
        ui->listView->setViewMode(QListView::IconMode);
        ui->listView->setMovement(QListView::Free);
        ui->listView->setResizeMode(QListView::Adjust);
        ui->listView->setIconSize(QSize(64, 64));
        ui->listView->setGridSize(QSize(100, 100));
    } else {
        ui->listView->setViewMode(QListView::ListMode);
        ui->listView->setMovement(QListView::Free);
        ui->listView->setIconSize(QSize(24, 24));
        ui->listView->setGridSize(QSize());
    }

    if (currentDirectory) {
        std::vector<OriginFile*> children = currentDirectory->getChildren();
        for (size_t i = 0; i < children.size(); i++) {
            OriginFile* item = children[i];
            if (!item) continue;

            QStandardItem* listItem = new QStandardItem(item->getName());
            listItem->setData(static_cast<qulonglong>(reinterpret_cast<uintptr_t>(item)), Qt::UserRole + 1);

            if (item->getIsDirectory()) {
                listItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
            } else {
                listItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
            }

            if (currentViewMode > 0) {
                if (item->getIsDirectory()) {
                    listItem->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
                } else {
                    listItem->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
                }
            }

            if (isCutOperation && clipboard == item) {
                listItem->setForeground(QColor(160, 160, 160));
            }
            listModel->appendRow(listItem);
        }
    }

    QStandardItem* homeNode = new QStandardItem("Home");
    homeNode->setData(static_cast<qulonglong>(reinterpret_cast<uintptr_t>(root)), Qt::UserRole + 1);
    homeNode->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QStandardItem* binNode = new QStandardItem("Recycle Bin");
    binNode->setData(static_cast<qulonglong>(reinterpret_cast<uintptr_t>(recycleBin)), Qt::UserRole + 1);
    binNode->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

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
            item->setData(static_cast<qulonglong>(reinterpret_cast<uintptr_t>(child)), Qt::UserRole + 1);
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

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
    OriginFile* item = reinterpret_cast<OriginFile*>(static_cast<uintptr_t>(index.data(Qt::UserRole + 1).toULongLong()));
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
    OriginFile* item = reinterpret_cast<OriginFile*>(static_cast<uintptr_t>(index.data(Qt::UserRole + 1).toULongLong()));
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

    if (node->getIsDirectory()) {
        Directory* copy = new Directory(node->getName(), parent);
        std::vector<OriginFile*> children = ((Directory*)node)->getChildren();
        for (int i = 0; i < (int)children.size(); i++) {
            OriginFile* childCopy = cloneNode(children[i], copy);
            if (childCopy) copy->addChild(childCopy);
        }
        return copy;
    } else {
        File* copy = new File(node->getName(), parent);
        copy->setContent(((File*)node)->getContent());
        return copy;
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
            if (name.trimmed().isEmpty()) {
                QMessageBox::warning(this, "Error", "The folder name cannot contain only spaces.");
            } else if (name == "." || name == "..") {
                QMessageBox::warning(this, "Error", "The folder name '.' and '..' are not allowed.");
            } else if (name.contains('/') || name.contains('*') || name.contains('?') || name.contains('<') || name.contains('>') || name.contains('|')) {
                QMessageBox::warning(this, "Error", "The folder name contains invalid characters.");
            } else {
                if (name.endsWith(".txt")) name = name.left(name.length() - 4);
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
                currentDirectory->setModified(QDateTime::currentDateTime());
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

    OriginFile* item = reinterpret_cast<OriginFile*>(static_cast<uintptr_t>(index.data(Qt::UserRole + 1).toULongLong()));
    if (!item || item == root || item == recycleBin) return;

    if (currentDirectory == recycleBin) {
        history.purgeSubtree(item);

        OriginFile* temp = currentDirectory;
        bool currentInvalid = false;
        while (temp != nullptr) {
            if (temp == item) { currentInvalid = true; break; }
            temp = temp->getParent();
        }
        if (currentInvalid) currentDirectory = (Directory*)root;

        recycleBin->removeChild(item);
        if (!isCutOperation && clipboard == item) {
            clipboard = nullptr;
        }
    } else {
        history.purgeSubtree(item);

        OriginFile* temp = currentDirectory;
        while (temp != nullptr) {
            if (temp == item) {
                currentDirectory = item->getParent() ? (Directory*)item->getParent() : (Directory*)root;
                history.addVisit(currentDirectory);
                break;
            }
            temp = temp->getParent();
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
        currentDirectory->setModified(QDateTime::currentDateTime());
        item->setOriginalPath(calculateFullPath(currentDirectory));
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

    OriginFile* item = reinterpret_cast<OriginFile*>(static_cast<uintptr_t>(index.data(Qt::UserRole + 1).toULongLong()));
    if (!item || item == recycleBin || item == root) return;

    if (isCutOperation && clipboard) {
        sendToRecycleBin(clipboard);
        saveSystem();
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

    OriginFile* item = reinterpret_cast<OriginFile*>(static_cast<uintptr_t>(index.data(Qt::UserRole + 1).toULongLong()));
    if (!item || item == recycleBin || item == root) return;

    if (isCutOperation && clipboard && clipboard != item) {
        sendToRecycleBin(clipboard);
        saveSystem();
        refreshUI();
    }

    clipboard = item;
    isCutOperation = true;
    refreshUI();
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
            if (isCutOperation && children[i] == clipboard) continue;
            if (children[i]->getName() == newName) {
                duplicate = true;
                break;
            }
        }
        if (duplicate) {
            if (isCutOperation) {
                newName = baseName + " (" + QString::number(counter) + ")" + ext;
            } else {
                newName = counter == 1 ? baseName + " copy" + ext : baseName + " copy " + QString::number(counter) + ext;
            }
            counter++;
        }
    }

    if (isCutOperation) {
        if (clipboard->getParent()) ((Directory*)clipboard->getParent())->detachChild(clipboard);
        clipboard->setName(newName);
        currentDirectory->addChild(clipboard);
        clipboard->setParent(currentDirectory);
        clipboard = nullptr;
        isCutOperation = false;
    } else {
        OriginFile* clone = cloneNode(clipboard, currentDirectory);
        if (clone) {
            clone->setName(newName);
            currentDirectory->addChild(clone);
        }
    }

    saveSystem();
    refreshUI();
    currentDirectory->setModified(QDateTime::currentDateTime());
}

void MainWindow::on_backwardb_clicked() {
    OriginFile* prev = history.goBack();
    if (prev != nullptr && prev->getIsDirectory()) {
        currentDirectory = (Directory*)prev;
        refreshUI();
    }
}

void MainWindow::on_forwardb_clicked() {
    OriginFile* next = history.goForward();
    if (next != nullptr && next->getIsDirectory()) {
        currentDirectory = (Directory*)next;
        refreshUI();
    }
}

void MainWindow::on_parentb_clicked() {
    if (currentDirectory && currentDirectory->getParent() && currentDirectory->getParent()->getIsDirectory()) {
        currentDirectory = (Directory*)currentDirectory->getParent();
        history.addVisit(currentDirectory);
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

    OriginFile* item = reinterpret_cast<OriginFile*>(static_cast<uintptr_t>(index.data(Qt::UserRole + 1).toULongLong()));
    if (!item || item == root || item == recycleBin) return;

    bool ok;
    QString oldName = item->getName();
    QString newName = QInputDialog::getText(this, "Rename", "New Name:", QLineEdit::Normal, oldName, &ok);

    if (ok && !newName.isEmpty() && newName != oldName) {
        if (item->getIsDirectory()) {
            if (newName.trimmed().isEmpty()) {
                QMessageBox::warning(this, "Error", "The folder name cannot contain only spaces.");
                return;
            }
            if (newName == "." || newName == "..") {
                QMessageBox::warning(this, "Error", "The folder name '.' and '..' are not allowed.");
                return;
            }
            if (newName.contains('/') || newName.contains('*') || newName.contains('?') || newName.contains('<') || newName.contains('>') || newName.contains('|')) {
                QMessageBox::warning(this, "Error", "The folder name contains invalid characters.");
                return;
            }
            if (newName.endsWith(".txt")) newName = newName.left(newName.length() - 4);
        } else {
            if (!newName.endsWith(".txt")) newName += ".txt";
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
            item->setModified(QDateTime::currentDateTime());
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

    OriginFile* item = reinterpret_cast<OriginFile*>(static_cast<uintptr_t>(index.data(Qt::UserRole + 1).toULongLong()));
    if (!item || item == root || item == recycleBin) return;

    QString type = item->getIsDirectory() ? "Folder" : "File";
    QString createdDate = item->getCreated().toString("dd/MM/yyyy hh:mm:ss");
    QString modifiedDate = item->getModified().toString("dd/MM/yyyy hh:mm:ss");

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
        info += "Type: " + type + "\n";
        info += "Files inside: " + QString::number(fileCount) + "\n";
        info += "Total size: " + QString::number(totalSize) + " bytes\n";
        info += "Created: " + createdDate + "\n";
        info += "Modified: " + modifiedDate;

        QMessageBox::information(this, "Properties", info);
    } else {
        File* fileItem = (File*)item;

        QString info = "Name: " + fileItem->getName() + "\n";
        info += "Type: " + type + "\n";
        info += "Size: " + QString::number(fileItem->getSize()) + " bytes\n";
        info += "Created: " + createdDate + "\n";
        info += "Modified: " + modifiedDate;

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

void MainWindow::sendToRecycleBin(OriginFile* item) {
    if (!item) return;

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
    item->setOriginalPath(calculateFullPath(item->getParent() ? item->getParent() : root));
    if (item->getParent()) ((Directory*)item->getParent())->detachChild(item);
    recycleBin->addChild(item);
    item->setInRecycleBin(true);
    item->setParent(recycleBin);
}

void MainWindow::on_enterb_clicked() {
    QString input = ui->pathline->text().trimmed();
    if (input.isEmpty()) return;

    QStringList parts = input.split("/", Qt::SkipEmptyParts);
    Directory* target = (Directory*)root;
    bool exactMatch = true;

    for (int i = 0; i < parts.size(); i++) {
        bool found = false;
        std::vector<OriginFile*> children = target->getChildren();
        for (size_t j = 0; j < children.size(); j++) {
            if (children[j]->getName() == parts[i] && children[j]->getIsDirectory()) {
                target = (Directory*)children[j];
                found = true;
                break;
            }
        }
        if (!found) {
            exactMatch = false;
            break;
        }
    }

    if (exactMatch) {
        currentDirectory = target;
        history.addVisit(currentDirectory);
        refreshUI();
        return;
    }

    QString name = parts.isEmpty() ? input : parts.last();
    std::vector<OriginFile*> results;
    ((Directory*)root)->search(name, results);

    if (results.empty()) {
        QMessageBox::information(this, "Search", "No files or folders matching \"" + name + "\" were found.");
        refreshUI();
        return;
    }

    if (results.size() == 1) {
        OriginFile* match = results.front();
        if (match->getIsDirectory()) {
            currentDirectory = (Directory*)match;
        } else if (match->getParent() && match->getParent()->getIsDirectory()) {
            currentDirectory = (Directory*)match->getParent();
        }
        history.addVisit(currentDirectory);
        refreshUI();
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Results for \"" + name + "\"");
    dialog.setMinimumSize(380, 280);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QLabel* label = new QLabel("Multiple matches found. Select one:", &dialog);
    layout->addWidget(label);

    QListWidget* listWidget = new QListWidget(&dialog);
    listWidget->setIconSize(QSize(20, 20));

    for (size_t i = 0; i < results.size(); i++) {
        OriginFile* item = results[i];
        QListWidgetItem* listItem = new QListWidgetItem(listWidget);
        listItem->setText(item->getName() + "   →  " + calculateFullPath(item));
        listItem->setIcon(item->getIsDirectory() ? style()->standardIcon(QStyle::SP_DirIcon) : style()->standardIcon(QStyle::SP_FileIcon));
        listItem->setData(Qt::UserRole, static_cast<qulonglong>(reinterpret_cast<uintptr_t>(item)));
    }

    layout->addWidget(listWidget);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* goBtn = new QPushButton("Go", &dialog);
    goBtn->setDefault(true);
    QPushButton* cancelBtn = new QPushButton("Cancel", &dialog);
    btnLayout->addStretch();
    btnLayout->addWidget(goBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(goBtn, &QPushButton::clicked, &dialog, [&]() { if (listWidget->currentItem()) dialog.accept(); });
    connect(listWidget, &QListWidget::itemDoubleClicked, &dialog, [&](QListWidgetItem*) { dialog.accept(); });

    if (dialog.exec() != QDialog::Accepted) return;

    QListWidgetItem* selected = listWidget->currentItem();
    if (!selected) return;

    OriginFile* match = reinterpret_cast<OriginFile*>(static_cast<uintptr_t>(selected->data(Qt::UserRole).toULongLong()));
    if (!match) return;

    if (match->getIsDirectory()) {
        currentDirectory = (Directory*)match;
    } else if (match->getParent() && match->getParent()->getIsDirectory()) {
        currentDirectory = (Directory*)match->getParent();
    }
    history.addVisit(currentDirectory);
    refreshUI();
}

void MainWindow::on_search_returnPressed() {
    QString name = ui->search->text().trimmed();
    if (name.isEmpty()) return;
    ui->search->clear();

    std::vector<OriginFile*> results;
    currentDirectory->search(name, results);

    if (results.empty()) {
        QMessageBox::information(this, "Search", "No files or folders matching \"" + name + "\" were found.");
        return;
    }

    if (results.size() == 1) {
        OriginFile* match = results.front();
        if (match->getIsDirectory()) {
            currentDirectory = (Directory*)match;
        } else if (match->getParent() && match->getParent()->getIsDirectory()) {
            currentDirectory = (Directory*)match->getParent();
        }
        history.addVisit(currentDirectory);
        refreshUI();
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Search Results for \"" + name + "\"");
    dialog.setMinimumSize(380, 280);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QLabel* label = new QLabel("Multiple matches found. Select one to navigate to it:", &dialog);
    layout->addWidget(label);

    QListWidget* listWidget = new QListWidget(&dialog);
    listWidget->setIconSize(QSize(20, 20));

    for (size_t i = 0; i < results.size(); i++) {
        OriginFile* item = results[i];
        QListWidgetItem* listItem = new QListWidgetItem(listWidget);
        listItem->setText(item->getName() + "   →  " + calculateFullPath(item));
        listItem->setIcon(item->getIsDirectory() ? style()->standardIcon(QStyle::SP_DirIcon) : style()->standardIcon(QStyle::SP_FileIcon));
        listItem->setData(Qt::UserRole, static_cast<qulonglong>(reinterpret_cast<uintptr_t>(item)));
    }

    layout->addWidget(listWidget);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* goBtn = new QPushButton("Go", &dialog);
    goBtn->setDefault(true);
    QPushButton* cancelBtn = new QPushButton("Cancel", &dialog);
    btnLayout->addStretch();
    btnLayout->addWidget(goBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(goBtn, &QPushButton::clicked, &dialog, [&]() { if (listWidget->currentItem()) dialog.accept(); });
    connect(listWidget, &QListWidget::itemDoubleClicked, &dialog, [&](QListWidgetItem*) { dialog.accept(); });

    if (dialog.exec() != QDialog::Accepted) return;

    QListWidgetItem* selected = listWidget->currentItem();
    if (!selected) return;

    OriginFile* match = reinterpret_cast<OriginFile*>(static_cast<uintptr_t>(selected->data(Qt::UserRole).toULongLong()));
    if (!match) return;

    if (match->getIsDirectory()) {
        currentDirectory = (Directory*)match;
    } else if (match->getParent() && match->getParent()->getIsDirectory()) {
        currentDirectory = (Directory*)match->getParent();
    }
    history.addVisit(currentDirectory);
    refreshUI();
}
