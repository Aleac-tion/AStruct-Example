#include "AleeParser.h"
#include <QMessageBox>
#include <QString>
#include <qfile.h>
#include <filesystem>
#include <fstream>
#include <istream>
#include <QDir>
#include <iomanip>
#include <windows.h>

namespace fs = std::filesystem;

AStruct coder_as;
AStruct cur_as;

bool startExitis = false;
bool MoveFolder(const fs::path& sourcePath, const fs::path& destPath) {
    try {
        // 一行实现文件夹转移（复制并删除源文件夹）
        fs::copy(sourcePath, destPath,
            fs::copy_options::recursive |
            fs::copy_options::overwrite_existing);// 删除源文件夹
        return true;
    }
    catch (const fs::filesystem_error& ex) {
        std::cerr << "文件夹转移失败: " << ex.what() << std::endl;
        return false;
    }
    catch (const std::exception& ex) {
        std::cerr << "发生错误: " << ex.what() << std::endl;
        return false;
    }
}
QString GetZipDir_cmd(const std::string& sourceDir, const std::string& zipPath) {
    fs::path sourcePath(sourceDir);

    // 获取父目录和文件夹名称
    QString parentDir = QString::fromStdString(sourcePath.parent_path().string());
    QString folderName = QString::fromStdString(sourcePath.filename().string());
    QString ZipPath = QString::fromStdString(zipPath);

    QString cmd = QString("tar -a -c -f %1 -C %2 %3")
        .arg("\"" + ZipPath + "\"")
        .arg("\"" + parentDir + "\"")
        .arg("\"" + folderName + "\"");

    return cmd;
}
bool CopyFiles(const std::string& sourceFile, const std::string& targetDir) {
    namespace fs = std::filesystem;

    try {
        fs::path sourcePath(sourceFile);
        fs::path targetPath(targetDir);

        // 检查源文件是否存在
        if (!fs::exists(sourcePath) || !fs::is_regular_file(sourcePath)) {
            return false;
        }

        // 创建目标目录（如果不存在）
        fs::create_directories(targetPath);

        // 构建目标文件路径
        fs::path targetFile = targetPath / sourcePath.filename();

        // 执行复制
        fs::copy_file(sourcePath, targetFile, fs::copy_options::overwrite_existing);
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}
std::vector<BYTE> loadFileToByteArray(const std::string& filename) {
    HANDLE hFile = CreateFileA(
        filename.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        throw std::runtime_error("无法打开文件: " + filename + " (错误代码: " + std::to_string(error) + ")");
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE) {
        CloseHandle(hFile);
        throw std::runtime_error("无法获取文件大小: " + filename);
    }

    std::vector<BYTE> buffer(fileSize);
    DWORD bytesRead;
    if (!ReadFile(hFile, buffer.data(), fileSize, &bytesRead, NULL) || bytesRead != fileSize) {
        CloseHandle(hFile);
        throw std::runtime_error("读取文件失败: " + filename);
    }

    CloseHandle(hFile);
    return buffer;
}
std::string generateByteArrayCode(const std::vector<unsigned char>& data, const std::string& varName, const std::string& filename) {
    std::stringstream code;

    code << "static const unsigned char " << varName << "[] = {";

    for (size_t i = 0; i < data.size(); ++i) {
        if (i % 16 == 0) {
            code << "\n    ";
        }
        code << "0x" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(data[i]);
        if (i < data.size() - 1) {
            code << ", ";
        }
    }

    code << "\n};\n";
    code << "const size_t " << varName << "_size" << "=sizeof(" << varName << ");" << "\n"
        << "std::vector<BYTE> " << varName << "_data(" << varName << "," << varName << "+" << varName << "_size);" << "\n"
        << "saveByteArrayToFile(" << varName << "_data," << "\"" << filename << "\"" << ");" << "\n";
    return code.str();
}
std::string readcpp(const std::string& filename) {
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


void AleeParser::start(AStruct* as, QString path, QString input, QTextEdit* output) {
    QString s = QString("tar -a -c -f %1 -C %2 %3").arg("\"K:\\QT_pro\\AleeUpTool\\x64\\Debug\\project\\MySetUp\\MySetUp.zip\"")
        .arg("\"K:\\QT_pro\\AleeUpTool\\x64\\Debug\\project\\MySetUp\\root\"")
        .arg("\"filepath\"");

    QString Core;
    AList Dir, filename;
    output->clear();
    output->append("Start Coding");
    output->append("Exitis:" + path);
    if (path.isEmpty()) {
        output->append("<font color='red'>Path is Empty!</font>");
        return;
    }
    {
        auto vec = AStruct::static_splittext(path.toStdString(), "\\");
        Core = QString::fromStdString(vec[vec.size() - 1]);
    }
    output->append("Set Class:" + Core);

    auto tmp_path = path + "\\" + Core + ".AleePro";

    if (!QFile::exists(tmp_path)) {
        output->append("<font color='red'>AleePro Open Failed!</font>");
        return;
    }

    coder_as.loaddata(tmp_path.toStdString());
    output->append("Setting fileArray");
    filename = AList::autoparse(AStruct::parseArray(coder_as.getvalue("Base", "files", "file")));
    Dir = AList::autoparse(AStruct::parseArray(coder_as.getvalue("Base", "files", "dir")));
    auto Root = path + "\\" + "root";
    auto Core_Path = path + "\\" + "root" + "\\" + Core;
    output->append("Starting Coding!");
    output->append("------------------------");
    output->append(QString("Clear Cache :%1.isempty ? no:clear").arg(Root));
    fs::remove_all(Root.toStdString());
    if (!fs::create_directory(Root.toStdString())) {
        output->append(QString("<font color='red'>create_directoryFailed:%1</font>").arg(Root));
        return;
    }
    output->append("create_directory root success");
    if (!fs::create_directory((Root + "\\" + Core).toStdString()));
    output->append("create_directory Core Path success");

    if (!fs::create_directory((Root + "\\" + "filepath").toStdString()));

    QString filepath = Root + "\\" + "filepath";

    output->append(QString(("filename size=%1")).arg(filename.size()));
    output->append(QString(("Dir size=%1")).arg(Dir.size()));


    for (int i = 0; i < Dir.size(); i++) {
        output->append(QString("Let %1 to %2").arg(Dir[i].operator std::string().c_str()).arg(filepath));
        MoveFolder(Dir[i].operator std::string(), filepath.toStdString());
    }
    output->append("Done!");

    for (int i = 0; i < filename.size(); i++) {
        output->append(QString("Let %1 to %2").arg(filename[i].operator std::string().c_str()).arg(Root + "\\" + Core));
        CopyFiles(filename[i].operator std::string(), (Root + "\\" + Core).toStdString());
    }
    output->append("Done!");

    output->append("Starting Exitis...");

    m_process->start(GetZipDir_cmd((Root + "\\" + Core).toStdString(), (path + "\\" + Core + ".zip").toStdString()));

    //开始等待
    {
        QEventLoop loop;
        QTimer timer;
        connect(&timer, &QTimer::timeout, &loop, [&]() {
            if (startExitis) {  // 你的条件
                loop.quit();
            }
            else {

            }
            });

        timer.start(1000);  // 每秒检查一次
        loop.exec();

    }

    if (QFile::exists(path + "\\" + Core + ".zip")) {
        output->append("Core Zipping over!");
    }
    else {
        output->append(QString("<font color='red'>[Error]:Wrong! :%1</font>").arg(path + "\\" + Core + ".zip"));
        return;
    }
    startExitis = false;

    m_process->start(GetZipDir_cmd((Root + "\\" + "filepath").toStdString(), (path + "\\" + "filepath.zip").toStdString()));

    //开始等待_文件
    {
        QEventLoop loop;
        QTimer timer;
        connect(&timer, &QTimer::timeout, &loop, [&]() {
            if (startExitis) {  
                loop.quit();
            }
            
        timer.start(1000);  // 每秒检查一次
        loop.exec();

    }
    if (QFile::exists(path + "\\" + "filepath.zip")) {
        output->append("Dir Zipping over!");
    }
    else {
        output->append(QString("<font color='red'>[Error]:Wrong! :%1</font>").arg(path + "\\" + "filepath.zip"));
        return;
    }
    startExitis = false;

    fs::create_directory(path.toStdString() + "\\" + "outputs");

    CopyFiles(path.toStdString() + "\\" + "filepath.zip", path.toStdString() + "\\" + "outputs");
    CopyFiles(path.toStdString() + "\\" + Core.toStdString() + ".zip", path.toStdString() + "\\" + "outputs");

    output->append("Ready to package...");

    fs::create_directory(AStruct::static_getDir() + "\\project\\final_out\\" + Core.toStdString());

    QString outPath = QString::fromStdString(AStruct::static_getDir()) + "\\project\\final_out\\" + Core;

    m_process->start(GetZipDir_cmd(path.toStdString() + "\\" + "outputs", path.toStdString() + "\\" + Core.toStdString() + "s.zip"));

    {
        QEventLoop loop;
        QTimer timer;
        connect(&timer, &QTimer::timeout, &loop, [&]() {
            if (startExitis) {  // 你的条件
                loop.quit();
            }
            else {

            }
            });

        timer.start(1000);  // 每秒检查一次
        loop.exec();

    }

    if (QFile::exists(path + "\\" + Core + "s.zip")) {
        output->append("package over!");
    }
    else {
        output->append(QString("<font color='red'>[Error]:Wrong! :%1</font>").arg(path + "\\" + Core + "s.zip"));
        return;
    }

    startExitis = false;

    output->append("Make all data to bytecode...");

    std::vector<unsigned char> fileData = loadFileToByteArray(path.toStdString() + "\\" + Core.toStdString() + "s.zip");

    std::string tempcode = generateByteArrayCode(fileData, "setup", Core.toStdString() + "\\.zip");

    std::string byte_path = as->getvalue("base", "template", "Standard_bytearray");

    //output->append(("Write to:" + byte_path).c_str());

    {
        std::string code = readcpp(byte_path);
        auto vec = AStruct::static_splittext(code, "\n");
        std::ostringstream oss;
        for (int i = 0; i < vec.size(); i++) {
            oss << vec[i] << "\n";
            if (AStruct::static_searchtext(vec[i], "//needinsertcode")) {
                oss << tempcode << "\n";
            }
        }

        std::ofstream out(AStruct::static_getDir() + "\\project\\final_out\\" + Core.toStdString() + "\\" + Core.toStdString() + ".cpp");
        out << oss.str();
        out.close();
    }

    output->append("Now you can Building Everything!");
    ready = true;
}

void AleeParser::onProcessOutput() {
    QByteArray output = m_process->readAllStandardOutput();
    QString text = QString::fromLocal8Bit(output);
    cmd_out->append(text);
}

void AleeParser::onProcessError() {
    QByteArray error = m_process->readAllStandardError();
    QString text = QString::fromLocal8Bit(error);
    cmd_out->append("<font color='red'>" + text + "</font>");
}

void AleeParser::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitCode == 0) {
        cmd_out->append("√okay");
        startExitis = true;
    }
    else {
        cmd_out->append("×Wrong:" + QString::number(exitCode));
        startExitis = true;
    }
}

void AleeParser::init(QTextEdit* output) {
    m_process = new QProcess(this);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &AleeParser::onProcessOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &AleeParser::onProcessError);
    connect(m_process, &QProcess::finished, this, &AleeParser::onProcessFinished);
    cmd_out = output;

}

AleeParser::~AleeParser()
{
    coder_as.loaddata("");
    // 清理资源
}
