#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#include "QString"
#include <QCoreApplication>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include "QDebug"
class Filemanager
{
public:
    QString applicationDirpath;
    QString amidite_sequence_Path;
    QString protocol_Path;
    QString valveSetting_Path;
    QString setting_Path;
    QString amidtie_floatting_path;

    QString amidite_sequence_Dir;
    QString protocol_Dir;
    QString history_Dir;

    Filemanager();
    bool save();
    bool load();
    bool fileExists(const QString& filePath);
    void createFileIfNotExists();
private:
    QString systemfilePath;
    QString dataFolderDir;
};

#endif // FILEMANAGER_H
