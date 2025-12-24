#include "AleaCOPY.h"
#include <AStruct.h>
#pragma comment(lib,"AStruct.lib")

AStruct as;

AleaCOPY::AleaCOPY(QWidget* parent)
    : QWidget(parent), clipboard(QApplication::clipboard()), timer(new QTimer(this))
{
    ui.setupUi(this);
    as.loaddata(AStruct::static_getDir() + "\\configs.Astruct");
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setFixedSize(464, 647);

    QSystemTrayIcon* trayIcon = new QSystemTrayIcon(this);
    QScreen* screen = QGuiApplication::primaryScreen();
    lastText = clipboard->text();

    connect(timer, &QTimer::timeout, this, &AleaCOPY::checkClipboard);
    timer->start(1000);

    trayIcon->setIcon(QIcon((AStruct::static_getDir() + "\\texture\\title.png").c_str()));
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();
    int x = screenWidth - this->width();
    int y = screenHeight - this->height();
    this->move(x, y);
    this->setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint);
    trayIcon->setToolTip("AleacCopyBoard");
    QMenu* trayIconMenu = new QMenu(this);
    QAction* restoreAction = new QAction("显示内容", this);
    QAction* quitAction = new QAction("退出", this);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->show();
    connect(restoreAction, &QAction::triggered, [=]() {
        this->show();                    // 显示主窗口
        this->activateWindow();          // 激活窗口（获得焦点）
        });
    connect(quitAction, &QAction::triggered, [=]() {
        // 可选：弹出确认对话框
        int ret = QMessageBox::question(this, "确认", "确定要退出吗？",
            QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            QApplication::quit();
        }
        });
    ui.tableView->setModel(model);

    ui.tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    model->setHorizontalHeaderItem(0, new QStandardItem(u8"编辑粘贴板历史记录"));
    ui.tableView->horizontalHeader()->resizeSection(0, 331);
    ui.tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    for (int i = 0; i < as.getKeyCount("config", "content"); i++) {
        model->setItem(i, 0, new QStandardItem(as.getvalue("config", "content", i).c_str()));
    }
}

void AleaCOPY::on_button_close_clicked() {
    this->hide();
}

void AleaCOPY::on_button_copy_clicked() {
    timer->stop();
    QItemSelectionModel* selectionModel = ui.tableView->selectionModel();
    QModelIndexList selectedIndexes = selectionModel->selectedIndexes();
    if (!selectedIndexes.isEmpty()) {
        QModelIndex selectedIndex = selectedIndexes.first();
        QVariant data = ui.tableView->model()->data(selectedIndex);
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(data.toString());
    }
    timer->start(800);
}

void AleaCOPY::on_button_edit_clicked() {
    QWidget* tempWindow = new QWidget(nullptr, Qt::Window);
    tempWindow->setWindowTitle("编辑器");
    tempWindow->resize(789, 544);

    QVBoxLayout* layout = new QVBoxLayout(tempWindow);

    QLabel* label = new QLabel("此为编辑内容窗口", tempWindow);
    label->setAlignment(Qt::AlignCenter);

    QTextEdit* textEdit = new QTextEdit(tempWindow);
    textEdit->setMinimumHeight(451);
    textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);


    QPushButton* overEditBtn = new QPushButton("完成编辑", tempWindow);
    overEditBtn->setFixedHeight(51);

    layout->addWidget(textEdit);
    layout->addWidget(overEditBtn);
    layout->addWidget(label);
    tempWindow->setWindowFlags(Qt::FramelessWindowHint);

    QItemSelectionModel* selectionModel = ui.tableView->selectionModel();
    QModelIndexList selectedIndexes = selectionModel->selectedIndexes();
    if (!selectedIndexes.isEmpty()) {
        QModelIndex selectedIndex = selectedIndexes.first(); 

        QVariant data = ui.tableView->model()->data(selectedIndex);
        textEdit->setPlainText(data.toString());
        tempWindow->show();
        tempWindow->setAttribute(Qt::WA_DeleteOnClose);
    }
    else {
        QMessageBox::information(this, "错误！", "请选择一个内容！");
    }
    QObject::connect(overEditBtn, &QPushButton::clicked, [=]() {
        QModelIndex selectedIndex = selectedIndexes.first();
        QString editedText = textEdit->toPlainText();
        model->setItem(selectedIndex.row(), 0, new QStandardItem(editedText));
        as.changevalue("config", "content", (int)selectedIndex, editedText.toStdString());
        tempWindow->close();
        });
}

void AleaCOPY::doSomethingFromUI() {

    //实际上逻辑早被AStruct转移简化，无用
}

void AleaCOPY::on_button_delete_clicked() {
    std::string targetPath = AStruct::static_getDir() + "/configs";
    for (const auto& entry : std::filesystem::directory_iterator(targetPath)) {
        std::filesystem::remove_all(entry.path());
    }

    as.DelHeader("config", "content");
    as.AppendHeader("config", "content", "1", "key");
    as.changeValue("config", "content", "1", "key", "configs/1.txt", true);
    model->setRowCount(0);
}

void AleaCOPY::checkClipboard() {
    QString currentText = clipboard->text();
    std::ostringstream filename_;
    if (currentText != lastText) {
        lastText = currentText;
        model->setItem(model->rowCount(), new QStandardItem(lastText));
        copy_index = model->rowCount();
        QModelIndex curr_table = model->index(copy_index - 1, 0);
        QVariant curr_data = ui.tableView->model()->data(curr_table);
        as.addkey("config", "content", std::to_string(as.getKeyCount("config", "content") + 1), "value");
        filename_ << "configs\\" << as.getKeyCount("config", "content") << ".txt";
        as.changeValue("config", "content", as.getKeyCount("config", "content"), lastText.toStdString(), filename_.str().c_str(), true);
    }
}

AleaCOPY::~AleaCOPY()
{
}

