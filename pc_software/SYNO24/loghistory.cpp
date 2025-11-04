#include "LogHistory.h"

// Constructor với tham số ProtocolName
LogHistory::LogHistory(const QString &protocolName) {
    // Tạo thư mục lưu trữ log nếu chưa tồn tại
    logDir = QCoreApplication::applicationDirPath() + "/data/02_history_run";
    QDir dir(logDir);
    if (!dir.exists()) {
        dir.mkpath("."); // Tạo thư mục nếu chưa có
    }

    // Tạo tên file dựa trên ProtocolName và thời gian hiện tại
    logFileName = protocolName + "_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".txt";
    logFilePath = logDir + "/" + logFileName;
}

// Phương thức để nối thêm nội dung vào file log
void LogHistory::appendToLog(const QString &message) {
    // Mở file để ghi (nếu file chưa tồn tại, nó sẽ được tạo mới)
    QFile file(logFilePath);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << QDateTime::currentDateTime().toString("[yyyy-MM-dd HH:mm:ss] ") << message << "\n";
        file.close();
    } else {
        qWarning() << "Không thể mở file log để ghi:" << logFilePath;
    }
}

// Phương thức để đặt quyền chỉ đọc (read-only) cho file
bool LogHistory::setLogFileReadOnly() {
    QFile file(logFilePath);

    if (file.exists()) {
        // Đặt quyền chỉ đọc
        return file.setPermissions(QFile::ReadOwner | QFile::ReadUser | QFile::ReadGroup | QFile::ReadOther);
    } else {
        qWarning() << "File không tồn tại:" << logFilePath;
        return false;
    }
}

// Phương thức để lấy đường dẫn file log
QString LogHistory::getLogFilePath() const {
    return logFilePath;
}
