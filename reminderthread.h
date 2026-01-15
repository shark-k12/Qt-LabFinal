#ifndef REMINDERTHREAD_H
#define REMINDERTHREAD_H

#include <QThread>
#include <QTimer>
#include <QList>
#include "taskdbmanager.h"

// 任务提醒线程：异步检测即将到期/逾期任务并弹出提醒
class ReminderThread : public QThread
{
    Q_OBJECT
public:
    explicit ReminderThread(QObject *parent = nullptr);
    ~ReminderThread() override;

    // 设置提醒阈值（分钟）
    void setReminderThreshold(int minutes);
    // 获取当前提醒阈值（用于设置弹窗回显）
    int getReminderThreshold() const;

protected:
    void run() override; // 线程入口函数

signals:
    // 检测到待提醒任务时触发（传递提醒文本）
    void reminder(const QString& msg);
    void taskStatusChanged();

private slots:
    // 定时检测任务
    void checkTasks();

private:
    QTimer *m_checkTimer = nullptr;       // 定时检测定时器
    int m_reminderThreshold;    // 提醒阈值（分钟，默认30）
};

#endif
