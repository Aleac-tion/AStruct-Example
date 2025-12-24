#include "AleeUpTool.h"
#include "AleeParser.h"
#include "AleeHighlighter.h"
#include <QMessageBox>
#include <QDir>
#include <QInputDialog>
#include <filesystem>
#include <qDebug>
#include <QFileDialog>
#include <AStruct.h>
#pragma comment(lib,"AStruct.lib")

namespace fs = std::filesystem;

std::string readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("无法打开文件: " + filename);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    if (file.fail() && !file.eof()) {
        throw std::runtime_error("读取文件失败: " + filename);
    }

    return buffer.str();
}

AleeUpTool::AleeUpTool(QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);


    //alee-init(ui.cmd_out);
    //初始化格式
    {
        ui.tabWidget->setStyleSheet("QTabBar::tab{min-width:250px;max-width:400px}");
        as.loaddata(AStruct::static_getDir() + "/config.Astruct");
        this->setWindowIcon(QIcon(as.getvalue("base", "config", "ico").c_str()));
        clarray = QString::fromStdString(as.getvalue("base", "config", "cl"));
        ui.small_output->setStyleSheet("QTextEdit { color: white; background-color: black; }");
        ui.output->setStyleSheet("QTextEdit { color: white; background-color: grey; }");
        new AleeHighlighter(ui.textEdit_2->document());


    }
    //初始化QTimer
    {
        barTimer = new QTimer(this);
        connect(barTimer, &QTimer::timeout, this, &AleeUpTool::Update_smallBar);
    }
    //初始化model
    {
        files_model = new QStandardItemModel(0, 2);
        files_model->setHorizontalHeaderItem(0, new QStandardItem("文件"));//360
        files_model->setHorizontalHeaderItem(1, new QStandardItem("路径"));

        ui.files_table->setModel(files_model);
        ui.files_table->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui.files_table->setSelectionMode((QAbstractItemView::SingleSelection));
        ui.files_table->horizontalHeader()->resizeSection(0, 70);
        ui.files_table->horizontalHeader()->resizeSection(1, 311);
    }


}

void AleeUpTool::on_config_button_clicked() {
    ui.small_output->clear();
    bool varsave = false;
    bool clsave = false;

    AList list;
    list = AList::autoparse(AStruct::parseArray(clarray.toStdString()));
    ui.small_output->append("checking environment!");
    ui.small_output->append(("base path = " + list[0].operator std::string()).c_str());
    ui.small_output->append("Ready to check cl.exe Environment!");
    if (QFile::exists(list[1].operator std::string().c_str())) {
        ui.small_output->append(("Environment is right Path:" + list[1].operator std::string()).c_str());
        Clexe = list[1].operator std::string().c_str();
        clsave = true;
    }
    else {
        ui.small_output->append(("<font color='red'>[Error]:Environment isnt Ready! Path:" +
            list[1].operator std::string() + "</font>").c_str());
    }

    if (QFile::exists(list[2].operator std::string().c_str())) {
        ui.small_output->append(("Environment varTool is right Path:" + list[2].operator std::string()).c_str());
        varsave = true;
        varTool = list[2].operator std::string().c_str();
    }
    else {
        ui.small_output->append(("<font color='red'>[Error]:Environment varTool isnt Ready! Path:" +
            list[2].operator std::string() + "</font>").c_str());
    }

    if (varsave && clsave) {
        std::ostringstream oss;
        oss << "---AleeToolsOut---\n"
            << "---Environment Checks---\n"
            << "VarTool:" << varTool.toStdString() << "\n"
            << "Cl.exe:" << Clexe.toStdString() << "\n"
            << "---Environment Checks SucessFully---\n";
        ui.small_output->append(oss.str().c_str());
    }
    else {
        std::ostringstream oss;
        oss << "---AleeToolsOut---\n"
            << "---Environment Checks---\n"
            << "VarTool:" << list[2].operator std::string().c_str() << "\n"
            << "Cl.exe:" << list[1].operator std::string().c_str() << "\n";



        ui.small_output->append(oss.str().c_str());
        ui.small_output->append("<font color='red'>---Environment isnt Work---</font>");

    }

}

void AleeUpTool::on_create_button_clicked() {
    ui.small_output->clear();
    bool issaved = false;
    bool ok;

    if (files_model->rowCount() == 0) {
        QMessageBox::information(this, "提示", "无法在没有内容的情况下创建项目");
        ui.small_output->append("<font color='red'>[Error]:The tableview is empty!</font>");
        return;
    }

    QString text = QInputDialog::getText(
        this,
        tr("创建项目"),
        tr("请输入安装包项目内容标题:"),
        QLineEdit::Normal,
        "MySetUp",
        &ok
    );
    std::string RootPath = as.getvalue("base", "currentpath", "rootpath");
    if (ok)
    {
        for (const auto& entry : fs::directory_iterator(RootPath)) {
            if (entry.path().filename().string() == text.toStdString()) {
                issaved = true;
                break;
            }
        }

        if (issaved) {
            QMessageBox::information(this, "Error", "此项目已经存在");
            ui.small_output->append("<font color='red'>[Error]:This project Name is illegal</font>");
            ui.small_output->append("<font color='red'>[Error]:It is saved or illegal's ProjectName</font>");
            ui.small_output->append(("[OutPut]:" + RootPath + "\\" + text.toStdString()).c_str());
            ui.small_output->append("<font color='gold'>[Warning]:Path Output:</font>");
            for (const auto& entry : fs::directory_iterator(RootPath)) {
                ui.small_output->append(entry.path().filename().string().c_str());
            }
        }
        else {
            std::string path = RootPath + "\\" + text.toStdString();
            if (fs::create_directory(path)) ui.small_output->append("Create dir success!");

            WorkPath = path.c_str();
            ui.small_output->append("project Name:" + WorkPath);
            ui.small_output->append(("Create Script From:" + as.getvalue("base", "template", "Standard")).c_str());
            StartUpScript();
        }
    }
}

void AleeUpTool::StartUpScript() {
    std::vector<std::string> vec_file;

    ui.small_output->append(("Read Template:" + as.getvalue("base", "template", "Standard")).c_str());
    QString tmp_Script = QString::fromStdString(readFile(as.getvalue("base", "template", "Standard")));
    auto vec = AStruct::static_splittext(WorkPath.toStdString(), "\\");
    auto vec_temp = AStruct::static_splittext(tmp_Script.toStdString(), "\n");
    Core = QString::fromStdString(vec[vec.size() - 1]);
    ui.small_output->append(("Setting Class:" + vec[vec.size() - 1]).c_str());
    ui.small_output->append("Setting Script Titie:" + Core);
    QString qstr = Core + ".AleePro";
    ui.small_output->append("Create Core file:" + WorkPath + "\\" + qstr);

    ui.small_output->append("Start reading Files table");
    for (int i = 0; i < files_model->rowCount(); i++) {
        QModelIndex file_table = files_model->index(i, 1);
        QVariant file_data = files_model->data(file_table, Qt::DisplayRole);
        ui.small_output->append(QString("Getfile:%1").arg(file_data.toString()));
        vec_file.push_back(file_data.toString().toStdString());
    }
    ui.small_output->append("Create file Array...");

    AList dir, file;
    for (const std::string& cur : vec_file) {
        if (AStruct::static_searchtext(cur, ".")) {
            file << cur;
        }
        else {
            dir << cur;
        }
    }
    auto p = as.getvalue("base", "template", "Standard_config");
    ui.small_output->append(QString("copy template:%1").arg(p.c_str()));
    ui.small_output->append("Run Exists...");
    QString config_path = WorkPath + "\\" + Core + ".AleePro";
    {
        std::string str = readFile(p);
        std::ofstream out(config_path.toStdString());
        out << str;
        out.close();
    }


    bool is = QFile::exists(config_path);
    if (is) {
        ui.small_output->append("Create " + config_path + " Successfully");
        ui.small_output->append("Exists Successfully");
    }
    else {
        ui.small_output->append(QString("<font color='red'>[Error]:Core file:%1 Memory is Null!</font>").arg(p.c_str()));
        ui.small_output->append(QString("<font color='gold'>[Warning]:Start cleaning old Dir %1</font>").arg(WorkPath));
        QDir temp_dir(WorkPath);
        temp_dir.removeRecursively();
        if (!temp_dir.exists()) {
            ui.small_output->append("done!");
            return;
        }
        ui.small_output->append("clear failed!");
        ui.small_output->append(QString("please clean dir yourself:%1").arg(WorkPath));
        return;
    }


    ui.small_output->append("Create AleeScript...");
    ui.small_output->append("start Modify Config");

    core.loaddata(config_path.toStdString());


    core.changeValue("Base", "config", "path", WorkPath.toStdString());
    core.changeValue("Base", "config", "core", Core.toStdString());

    //替换
    std::string str_a = "";
    {
        if (file.size() != 0) {
            std::ostringstream oss;
            for (int i = 0; i < file.size() - 1; i++) {
                oss << file[i].operator std::string() << ",";
            }
            oss << file[file.size() - 1].operator std::string();
            str_a = oss.str();
            ui.small_output->append(QString("Set files:%1").arg(str_a.c_str()));
            core.changeValue("Base", "files", "file", file.toArray());
        }
        else {
            core.changeValue("Base", "files", "file", "@array@[]");
        }
    }
    std::string str_b = "";
    {
        if (dir.size() != 0) {
            std::ostringstream oss;
            for (int i = 0; i < dir.size() - 1; i++) {
                oss << dir[i].operator std::string() << ",";
            }
            oss << dir[dir.size() - 1].operator std::string();
            str_b = oss.str();
            ui.small_output->append(QString("Set Dir:%1").arg(str_b.c_str()));
            core.changeValue("Base", "files", "dir", dir.toArray());
        }
        else {
            core.changeValue("Base", "files", "dir", "@array@[]");
        }
    }



    std::ostringstream oss;
    for (int i = 0; i < vec_temp.size(); i++) {

        if (AStruct::static_searchtext(vec_temp[i], "Base Script Titie:")) {
            oss << QString::fromStdString(vec_temp[i]).replace("Class", Core).toStdString();
        }
        else if (AStruct::static_searchtext(vec_temp[i], "Target filename:")) {
            oss << QString::fromStdString(vec_temp[i]).replace("filesarray", str_a.c_str()).toStdString();
        }
        else if (AStruct::static_searchtext(vec_temp[i], "Target Dir:")) {
            oss << QString::fromStdString(vec_temp[i]).replace("dirarray", str_b.c_str()).toStdString();
        }
        else {
            oss << vec_temp[i];
        }
    }

    ui.textEdit_2->setText(QString::fromStdString(oss.str()));
    ui.small_output->append("Done!");
    std::ostringstream os;
    os << "---AleeTools---\n"
        << "---Building Successfully---";
    ui.small_output->append(os.str().c_str());
    std::ofstream out(WorkPath.toStdString() + "\\" + Core.toStdString() + ".Alee");
    out << ui.textEdit_2->toPlainText().toStdString();
    out.close();
    ui.tabWidget->setCurrentIndex(1);

    ui.lineEdit->setText(WorkPath);
}

void AleeUpTool::Update_smallBar() {
    ui.progressBar->setValue(BarSize(15, 100));
    if (ui.progressBar->value() == 100) {
        ui.progressBar->setVisible(false);
        barTimer->stop();
    }
}

void AleeUpTool::on_addon_button_clicked() {
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("选择目标文件夹"),
        QDir::currentPath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    dir = QDir::toNativeSeparators(dir);

    auto vec = AStruct::static_splittext(dir.toStdString(), "\\");

    for (int i = 0; i < files_model->rowCount(); i++) {
        QModelIndex file_table = files_model->index(i, 1);
        QVariant file_data = files_model->data(file_table, Qt::DisplayRole);
        if (dir == file_data.toString()) {
            QMessageBox::information(this, "警告", "不允许重复!");
            return;
        }
    }

    if (!dir.isEmpty()) {
        files_model->setItem(files_model->rowCount(), 0, new QStandardItem(vec[vec.size() - 1].c_str()));
        files_model->setItem(files_model->rowCount() - 1, 1, new QStandardItem(dir));
    }
}

void AleeUpTool::on_addon_button_2_clicked() {

    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("选择可执行文件"),
        QDir::currentPath(),  // 初始目录
        tr("所有文件 (*.*)")
    );

    fileName = QDir::toNativeSeparators(fileName);

    auto vec = AStruct::static_splittext(fileName.toStdString(), "\\");

    for (int i = 0; i < files_model->rowCount(); i++) {
        QModelIndex file_table = files_model->index(i, 1);
        QVariant file_data = files_model->data(file_table, Qt::DisplayRole);
        if (fileName == file_data.toString()) {
            QMessageBox::information(this, "警告", "不允许重复!");
            return;
        }
    }


    if (!fileName.isEmpty()) {
        files_model->setItem(files_model->rowCount(), 0, new QStandardItem(vec[vec.size() - 1].c_str()));
        files_model->setItem(files_model->rowCount() - 1, 1, new QStandardItem(fileName));
    }
}

void AleeUpTool::on_del_button_clicked() {
    QModelIndex curr_index = ui.files_table->currentIndex();
    files_model->removeRow(curr_index.row());
}

void AleeUpTool::on_test_button_clicked() {
    AleeParser* al = new AleeParser(this);
    al->init(ui.cmd_out);
    al->start(&as, WorkPath, ui.textEdit_2->toPlainText(), ui.output);

}

void AleeUpTool::on_read_button_clicked() {
    AStruct t_as;

    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("选择项目"),
        AStruct::static_getDir().c_str(),  // 初始目录
        tr("打开AleePro(*.AleePro);")
    );

    if (fileName != "") {
        t_as.loaddata(fileName.toStdString());
    }

    ui.textEdit_2->clear();
    ui.output->clear();

    ui.output->append(QString("Reading Project:%1").arg(fileName));
    ui.output->append("Starting Exists!");
    if (QFile::exists(fileName)) {
        ui.output->append("Is ready");
    }
    else {
        ui.output->append("<font color='red'>Target Project is Wrong!</font>");
        return;
    }
    ui.tabWidget->setCurrentIndex(1);
    ui.output->append("Reading Config!");
    ui.textEdit_2->setText(QString::fromStdString(readFile(t_as.getvalue("Base", "config", "path") + "\\" + "MySetUp.Alee")));
    Core = QString::fromStdString(t_as.getvalue("Base", "config", "core"));
    ui.output->append(QString("Set Class:%1").arg(Core));
    WorkPath = QString::fromStdString(t_as.getvalue("Base", "config", "path"));
    ui.output->append(QString("Set WorkPath:%1").arg(WorkPath));

    ui.output->append("Done!");
    std::ostringstream os;
    os << "---AleeTools---\n"
        << "---Building Successfully---";
    ui.lineEdit->setText(WorkPath);
}

void AleeUpTool::on_save_button_clicked() {
    if (!ui.textEdit_2->hasFocus()) {
        ui.textEdit_2->setFocus();  // 聚焦到编辑器
    }

    if (!WorkPath.isEmpty() && !Core.isEmpty()) {
        std::ofstream out(WorkPath.toStdString() + "\\" + Core.toStdString() + ".Alee");
        out.clear();
        out << ui.textEdit_2->toPlainText().toStdString();
        out.close();
    }
    ui.output->append("已保存");
}

void AleeUpTool::keyPressEvent(QKeyEvent* event) {
    if (event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_S) {
        on_save_button_clicked();
        event->accept();  // 标记事件已处理
    }
    else {
        QWidget::keyPressEvent(event);  // 其他按键交给父类处理
    }
}

void AleeUpTool::on_build_button_clicked() {
    auto vec = AStruct::static_splittext(ui.lineEdit->text().toStdString(), "\\");

    Core = QString::fromStdString(vec[vec.size() - 1]);

    QString root_path;
    {
        std::ostringstream oss;
        for (int i = 0; i < vec.size() - 1; i++) {
            oss << vec[i] << "\\";
        }
        root_path = QString::fromStdString(oss.str());
    }

    AleeParser* par = new AleeParser(this);
    par->init(ui.cmd_out);
    {
        if (varTool.isEmpty() || Clexe.isEmpty()) {
            QMessageBox::information(this, "提示", "请先配置!");
            return;
        }

        QString cmd;
        QString vartool = varTool;
        QString clpath = Clexe;
        QString path = root_path + "final_out\\" + Core + "\\" + Core + ".cpp";
        QString outpath = root_path + "final_out\\" + Core + "\\" + Core + ".exe";

        cmd = "\"" + vartool + "\"" + " x64 && " + clpath + " /MD " + " /Fe\"" + outpath + "\" \"" + path + "\"";

        QString batchContent = cmd;
        QString batchPath = QDir::tempPath() + "/compile_temp.bat";
        QFile batchFile(batchPath);

        if (batchFile.open(QIODevice::WriteOnly)) {
            batchFile.write(batchContent.toUtf8());
            batchFile.close();
        }
        par->m_process->start(batchPath);     
    }

}

AleeUpTool::~AleeUpTool()
{
}


