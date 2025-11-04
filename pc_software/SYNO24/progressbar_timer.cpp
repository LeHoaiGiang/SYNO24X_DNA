#include "progressbar_timer.h"

ProgressBarTimer::ProgressBarTimer(QProgressBar *progressBar, QObject *parent)
    : QObject(parent), m_progressBar(progressBar)
{
    connect(&m_timer, &QTimer::timeout, this, &ProgressBarTimer::updateProgress);
}

void ProgressBarTimer::startCountdown(int millis)
{
    if (millis <= 0 || !m_progressBar)
        return;

    if (m_timer.isActive())
        m_timer.stop();

    m_totalTimeMillis = millis;
    m_elapsedTimer.start();

    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);

    m_timer.start(10); // Cập nhật mỗi 10ms
}

void ProgressBarTimer::updateProgress()
{
    int elapsed = m_elapsedTimer.elapsed();
    if (elapsed >= m_totalTimeMillis) {
        m_progressBar->setValue(100);
        m_timer.stop();
        emit finished(); // Gửi tín hiệu khi hoàn tất
        return;
    }
    int progress = (elapsed * 100) / m_totalTimeMillis;
    m_progressBar->setValue(progress);
}
