#ifndef DELAY_H
#define DELAY_H
#include <QTime>
#include <QTimer>
#include <QCoreApplication>

class delay
{
public:
    QTimer timer;
    delay();
    void delay_ms(int millisecondsToWait );
    void delay_milliseconds(int millisecondsToWait);
};

#endif // DELAY_H
