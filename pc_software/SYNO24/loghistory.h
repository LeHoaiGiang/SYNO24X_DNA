#ifndef LOGHISTORY_H
#define LOGHISTORY_H
/***
 * LeHoaiGiang
 *
 */

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QFileInfo>
#include <QDebug>

class LogHistory {
public:
    // Constructor
    // Constructor với tham số ProtocolName
    explicit LogHistory(const QString &protocolName);

    // Phương thức để nối thêm nội dung vào file log
    void appendToLog(const QString &message);

    // Phương thức để đặt quyền chỉ đọc (read-only) cho file
    bool setLogFileReadOnly();

    // Phương thức để lấy đường dẫn file log
    QString getLogFilePath() const;

private:
    QString logDir;      // Đường dẫn thư mục lưu trữ log
    QString logFileName; // Tên file log
    QString logFilePath; // Đường dẫn đầy đủ đến file log
};

#endif // LOGHISTORY_H
