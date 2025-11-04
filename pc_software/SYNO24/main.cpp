#include "syno24.h"
#include "delay.h"
#include <QApplication>
#include <QSplashScreen>
#include <QTimer>
#include <QApplication>
#include <QMovie>
#include <QtGui>
#include "QLabel"
#include "unistd.h"
//delay delay_;
QString windows_name = "SYNO24X DNA 09-09-2025";
/*
 * 16-03-25 Cập nhật tính toán thời gian chạy
 * cập nhật tính toán hóa chất
 * sửa lỗi giới hạn base tại sub 3
 * copy sub protocol như SYNO96
 * insert step in protocol 10-04-2025
 *
 *
 * */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SYNO24 w;
    QDateTime now = QDateTime::currentDateTime();
    QString str_time = now.toString("dd/MM/yyyy"); // Or any other Qt format string
    w.setWindowTitle(windows_name);
//    QPixmap pixmap(":/splash/splash/GIF-SYNO24.gif");
//    QSplashScreen splash(pixmap);
//    QLabel label(&splash);
//    QMovie mv(":/splash/splash/GIF-SYNO24.gif");
//    label.setMovie(&mv);
//    mv.start();
//    splash.show();
//    delay_.delay_ms(1000);
    w.show();
//      splash.close();
    return a.exec();
}
