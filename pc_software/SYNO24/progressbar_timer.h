#ifndef PROGRESSBARTIMER_H
#define PROGRESSBARTIMER_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QProgressBar>

class ProgressBarTimer : public QObject
{
    Q_OBJECT

public:
    explicit ProgressBarTimer(QProgressBar *progressBar, QObject *parent = nullptr);

    void startCountdown(int millis);

signals:
    void finished(); // Tín hiệu khi đếm ngược xong

private slots:
    void updateProgress();

private:
    QProgressBar *m_progressBar;
    QTimer m_timer;
    QElapsedTimer m_elapsedTimer;
    int m_totalTimeMillis;
};

#endif // PROGRESSBARTIMER_H
