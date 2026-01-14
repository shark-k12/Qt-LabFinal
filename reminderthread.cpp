#include "ReminderThread.h"
#include <QDateTime>
#include <QDebug>

ReminderThread::ReminderThread(QObject *parent)
    : QThread(parent), m_reminderThreshold(30) // 默认提前30分钟提醒
{
    // 定时器移到子线程（run()中初始化）
    m_checkTimer = new QTimer();
    m_checkTimer->setParent(nullptr); // 避免线程销毁时的父子关系问题
    connect(m_checkTimer, &QTimer::timeout, this, &ReminderThread::checkTasks);
}

ReminderThread::~ReminderThread()
{
    // 安全停止线程并释放资源
    quit();
    wait(3000); // 等待3秒，超时则强制终止
    if (isRunning()) {
        terminate();
        wait();
    }
    m_checkTimer->stop();
    delete m_checkTimer;
    qDebug() << "提醒线程已释放";
}

// 设置提醒阈值（分钟，确保值为正）
void ReminderThread::setReminderThreshold(int minutes)
{
    if (minutes > 0) {
        m_reminderThreshold = minutes;
        qDebug() << "提醒阈值已更新为：" << minutes << "分钟";
    }
}

// 获取当前提醒阈值
int ReminderThread::getReminderThreshold() const
{
    return m_reminderThreshold;
}

// 线程入口：启动定时器（每隔1分钟检测一次）
void ReminderThread::run()
{
    m_checkTimer->start(60 * 1000); // 60秒检测一次任务
    exec(); // 启动线程事件循环
}

// 核心：检测即将到期/已逾期任务
void ReminderThread::checkTasks()
{
    QList<Task> allTasks = TaskDBManager::getInstance()->getAllTasks();
    QDateTime now = QDateTime::currentDateTime();
    QStringList reminderMsgs;

    for (const Task& task : allTasks) {
        // 跳过已完成的任务
        if (task.isCompleted) continue;

        // 计算当前时间与截止时间的差值（分钟）
        qint64 diffMinutes = now.secsTo(task.deadline) / 60;

        // 1. 已逾期（截止时间 < 当前时间）
        if (diffMinutes < 0) {
            reminderMsgs.append(QString("【逾期提醒】任务「%1」已逾期 %2 分钟！")
                                    .arg(task.title).arg(-diffMinutes));
        }
        // 2. 即将到期（在提醒阈值内）
        else if (diffMinutes <= m_reminderThreshold && diffMinutes > 0) {
            reminderMsgs.append(QString("【到期提醒】任务「%1」将在 %2 分钟后截止！")
                                    .arg(task.title).arg(diffMinutes));
        }
    }

    // 有提醒内容则发送信号（主线程接收并弹窗）
    if (!reminderMsgs.isEmpty()) {
        emit reminder(reminderMsgs.join("\n"));
    }
}
