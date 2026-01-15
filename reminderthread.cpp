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
    qDebug() << "🔹 线程析构函数执行";
    // 停止线程事件循环
    quit();
    // 等待线程结束（最多3秒）
    if (wait(3000)) {
        qDebug() << "🔹 线程正常退出";
    } else {
        terminate();
        wait();
        qDebug() << "🔹 线程强制终止";
    }
    // 此时定时器已在run()中释放，无需重复删除
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

// 线程入口：启动定时器
void ReminderThread::run()
{
    qDebug() << "🔹 线程run()函数开始执行";

    // 1. 子线程内创建定时器（关键！避免跨线程定时器问题）
    m_checkTimer = new QTimer();
    m_checkTimer->setInterval(10000); // 10秒检测一次（测试用）

    // 2. 子线程内绑定定时器信号（DirectConnection，同线程直接调用）
    connect(m_checkTimer, &QTimer::timeout,
            this, &ReminderThread::checkTasks,
            Qt::DirectConnection);

    // 3. 启动定时器
    m_checkTimer->start();
    qDebug() << "🔹 子线程定时器启动，间隔10秒";

    // 4. 启动线程事件循环（必须！否则线程执行完run()就退出）
    exec();

    // 5. 事件循环退出后清理定时器
    m_checkTimer->stop();
    delete m_checkTimer;
    m_checkTimer = nullptr;
    qDebug() << "🔹 线程run()函数执行完毕";
}

// 核心：检测即将到期/已逾期任务
void ReminderThread::checkTasks()
{
    qDebug() << "🔍 开始检测任务（当前阈值：" << m_reminderThreshold << "分钟）";

    // 数据库连接校验
    if (!TaskDBManager::getInstance()->isConnected()) {
        qDebug() << "❌ 数据库连接失败，跳过任务检测";
        return;
    }
    QList<Task> allTasks = TaskDBManager::getInstance()->getAllTasks();
    qDebug() << "查询到的任务总数：" << allTasks.size();

    // 初始化变量
    QDateTime now = QDateTime::currentDateTime();
    bool taskExpired = false;
    QStringList reminderMsgs;
    // 用静态Map记录每个任务的提醒状态（避免重复弹窗）
    static QMap<int, QSet<int>> taskRemindFlag; // key:任务ID，value:已触发的提醒类型（1=阈值提醒，2=截止提醒）

    for (const Task& task : allTasks) {
        // 跳过已完成任务，重置提醒标记
        if (task.isCompleted) {
            qDebug() << "跳过已完成任务：" << task.title;
            taskRemindFlag.remove(task.id);
            continue;
        }

        // 计算剩余时间（秒转分钟）
        qint64 diffSeconds = now.secsTo(task.deadline);
        qint64 diffMinutes = diffSeconds / 60;
        qDebug() << "任务：" << task.title
                 << " | 截止时间：" << task.deadline.toString("yyyy-MM-dd HH:mm:ss")
                 << " | 当前时间：" << now.toString("yyyy-MM-dd HH:mm:ss")
                 << " | 剩余分钟：" << diffMinutes;

        // 逾期任务：重置标记+标记状态
        if (diffMinutes < 0) {
            taskExpired = true;
            taskRemindFlag.remove(task.id); // 逾期后重置提醒标记
            qDebug() << "⚠️ 任务「" << task.title << "」已逾期 " << -diffMinutes << " 分钟";
            continue;
        }

        // ===== 核心逻辑：仅阈值时间、截止时间弹窗（各一次）=====
        // 1. 阈值时间提醒
        if (diffMinutes == m_reminderThreshold && !taskRemindFlag[task.id].contains(1)) {
            reminderMsgs.append(QString("【提醒】任务「%1」将在 %2 分钟后截止！").arg(task.title).arg(diffMinutes));
            taskRemindFlag[task.id].insert(1); // 标记已触发阈值提醒
            qDebug() << "🚨 触发阈值提醒：" << task.title;
        }

        // 2. 截止时间提醒
        if (diffMinutes == 0 && !taskRemindFlag[task.id].contains(2)) {
            reminderMsgs.append(QString("【提醒】任务「%1」已到截止时间！").arg(task.title));
            taskRemindFlag[task.id].insert(2); // 标记已触发截止提醒
            qDebug() << "🚨 触发截止提醒：" << task.title;
        }
    }

    // 发射提醒信号
    if (!reminderMsgs.isEmpty()) {
        qDebug() << "📢 发射提醒信号：" << reminderMsgs.join("\n");
        emit reminder(reminderMsgs.join("\n"));
        emit taskStatusChanged(); // 更新状态栏
    } else {
        qDebug() << "ℹ️ 无待提醒任务（未到阈值/截止时间）";
    }

    // 有逾期任务时更新状态栏
    if (taskExpired) {
        emit taskStatusChanged();
    }
}
