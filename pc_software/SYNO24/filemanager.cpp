#include "filemanager.h"
//#define DEBUG_FILEMANAGER

Filemanager::Filemanager()
{
    applicationDirpath = QCoreApplication::applicationDirPath();
    dataFolderDir = applicationDirpath + "/data"; // Dir chi thu muc
    systemfilePath = applicationDirpath + "/system/system.json"; // path chi duong dan cu the nao do
    valveSetting_Path = applicationDirpath + "/system/valve_setting.json"; // duong dan file setting valve
    amidtie_floatting_path = applicationDirpath + "/system/amidite_floatting.json"; //đường đẫn amidite floatting

    amidite_sequence_Dir = dataFolderDir + "/00_sequence"; // Dir chi thu muc
    protocol_Dir = dataFolderDir + "/01_protocol"; // Dir chi thu muc
    history_Dir = dataFolderDir+ "/02_history_run";

//    QString amidite_sequence_Dir;
//    QString protocol_Dir;
//    QString history_Dir;
#ifdef DEBUG_FILEMANAGER
    qDebug() << "DEBUG_FILEMANAGER init";
    qDebug() << "dataFolderDir: " << dataFolderDir;
    qDebug() << "systemfilePath: " << systemfilePath;
    qDebug() << "valveSetting_Path: " << valveSetting_Path;
    qDebug() << "amidite_sequence_Dir: " << amidite_sequence_Dir;
    qDebug() << "protocol_Dir: " << protocol_Dir;
    qDebug() << "history_Dir: " << history_Dir;
    qDebug() << "amidtie_floatting_path: "<<amidtie_floatting_path;
#endif
}
bool Filemanager::save()
{
    // Tạo đường dẫn tới thư mục "data"

    QDir dataFolder(systemfilePath);
    if (!dataFolder.exists())
    {
        // Tạo thư mục "data" nếu nó không tồn tại
        dataFolder.mkpath(".");
    }

    // Tạo đường dẫn tới file "system.json"
    //QString filePath = dataFolderDir + "/system.json";

    // Tạo một đối tượng QJsonObject để lưu trữ dữ liệu
    QJsonObject jsonObject;
    /*
    QString applicationDirpath;
    QString amidite_sequence_Path;
    QString protocol_Path;
    QString valveSetting_Path;
    QString setting_Path;
     */
    //jsonObject["applicationDirpath"] = applicationDirpath;
    jsonObject["amidite_sequence_Path"] = amidite_sequence_Path;
    jsonObject["protocol_Path"] = protocol_Path;
    //jsonObject["valveSetting_Path"] = valveSetting_Path;
    //jsonObject["setting_Path"] = setting_Path;

    // Tạo một đối tượng QJsonDocument từ QJsonObject
    QJsonDocument jsonDoc(jsonObject);

    // Mở file để ghi
    QFile file(systemfilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        // Ghi dữ liệu vào file
        file.write(jsonDoc.toJson());
        file.close();
#ifdef DEBUG_FILEMANAGER
        qDebug() << "saved to file: " << systemfilePath << "Success";
#endif
        return true;
    }
    else
    {
        return false;
#ifdef DEBUG_FILEMANAGER
        qDebug() << "Failed to save data to file: " << systemfilePath;
#endif

    }
}

bool Filemanager::load()
{
    // Tạo đường dẫn tới file "system.json"
    // Mở file để đọc
    bool error = false;
    QFile file(systemfilePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // Đọc nội dung file
        QByteArray fileData = file.readAll();
        file.close();

        // Tạo một đối tượng QJsonDocument từ dữ liệu file
        QJsonDocument jsonDoc = QJsonDocument::fromJson(fileData);
        if (!jsonDoc.isNull() && jsonDoc.isObject())
        {
            // Truy cập vào đối tượng QJsonObject chứa dữ liệu
            QJsonObject jsonObject = jsonDoc.object();
            /*
            QString applicationDirpath;
            QString amidite_sequence_Path;
            QString protocol_Path;
            QString valveSetting_Path;
            QString setting_Path;
             */
            //applicationDirpath = jsonObject["applicationDirpath"].toString();
            amidite_sequence_Path = jsonObject["amidite_sequence_Path"].toString();
            protocol_Path = jsonObject["protocol_Path"].toString();
            //valveSetting_Path = jsonObject["valveSetting_Path"].toString();
            //setting_Path = jsonObject["setting_Path"].toString();
#ifdef DEBUG_FILEMANAGER

            qDebug() << "loaded from file systemfilePath : " << systemfilePath;
#endif
            error = true;
        }
        else
        {
#ifdef DEBUG_FILEMANAGER
            qDebug() << "Failed to parse JSON data from file: " << systemfilePath;
            error = false;
#endif
        }
    }
    else
    {
#ifdef DEBUG_FILEMANAGER
        qDebug() << "Failed to load file systemfilePath: " << systemfilePath;
#endif
        error = false;
    }
    return error;
}
// kiem tra xem file da ton tai chua
bool Filemanager::fileExists(const QString& filePath) {
    QFileInfo checkFile(filePath);
    return checkFile.exists() && checkFile.isFile();
}
// tu tao file neu file da ton tai roi
void Filemanager::createFileIfNotExists() {
    if (!fileExists(systemfilePath)) {
        QFile file(systemfilePath);
        if (file.open(QIODevice::WriteOnly)) {
            qDebug() << "File created: " << systemfilePath;
            file.close();
        } else {
            qDebug() << "Failed to create file: " << systemfilePath;
        }
    } else {
        qDebug() << "File already exists: " << systemfilePath;
    }
}
