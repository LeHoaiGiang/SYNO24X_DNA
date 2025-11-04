#include "delay.h"

delay::delay()
{
    //connect(&timer, &QTimer::timeout, this, &delay::onTimeout);
}

void delay::delay_milliseconds(int millisecondsToWait)
{
    QTimer timer;
    QEventLoop loop;
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(millisecondsToWait);
    loop.exec();
}

void delay::delay_ms(int millisecondsToWait )
{

    QTime dieTime = QTime::currentTime().addMSecs( millisecondsToWait );
     while( QTime::currentTime() < dieTime )
     {
         QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
     }

}

