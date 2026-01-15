#ifndef TASKDBMANAGER_H
#define TASKDBMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QList>
#include <QDateTime>
#include <QDebug>

struct Task {
    int id = -1;                      // 主键（-1表示未入库）
    QString title;                    // 标题
    QString category = "未分类";      // 分类
    int priority = 3;                 // 优先级（1-5）
    QDateTime deadline;               // 截止时间
    bool isCompleted = false;         // 完成状态
    QString description;              // 描述
    QDateTime createTime;             // 创建时间
    QDateTime updateTime;             // 更新时间

    // 结构体转QVariantMap（核心修改：去掉T，统一格式）
    QVariantMap toMap() const {
        QVariantMap map;
        map["title"] = title;
        map["category"] = category;
        map["priority"] = priority;
        // 替换Qt::ISODate为自定义格式，去掉T分隔符
        map["deadline"] = deadline.toString("yyyy-MM-dd HH:mm:ss");
        map["is_completed"] = isCompleted ? 1 : 0;
        map["description"] = description;
        map["create_time"] = createTime.toString("yyyy-MM-dd HH:mm:ss");
        map["update_time"] = updateTime.toString("yyyy-MM-dd HH:mm:ss");
        return map;
    }

    static Task fromMap(const QVariantMap& map) {
        Task task;
        task.id = map["id"].toInt();
        task.title = map["title"].toString();
        task.category = map["category"].toString();
        task.priority = map["priority"].toInt();

        // 兼容处理：先替换T为空格，再解析
        QString deadlineStr = map["deadline"].toString().replace("T", " ");
        // 优先按无T格式解析（用静态函数QDateTime::fromString）
        QDateTime deadline = QDateTime::fromString(deadlineStr, "yyyy-MM-dd HH:mm:ss");
        if (!deadline.isValid()) { // 解析失败则回退到ISODate
            deadline = QDateTime::fromString(deadlineStr, Qt::ISODate);
        }
        task.deadline = deadline;

        task.isCompleted = (map["is_completed"].toInt() == 1);
        task.description = map["description"].toString();

        // 创建时间兼容处理
        QString createTimeStr = map["create_time"].toString().replace("T", " ");
        QDateTime createTime = QDateTime::fromString(createTimeStr, "yyyy-MM-dd HH:mm:ss");
        if (!createTime.isValid()) {
            createTime = QDateTime::fromString(createTimeStr, Qt::ISODate);
        }
        task.createTime = createTime;

        // 更新时间兼容处理
        QString updateTimeStr = map["update_time"].toString().replace("T", " ");
        QDateTime updateTime = QDateTime::fromString(updateTimeStr, "yyyy-MM-dd HH:mm:ss");
        if (!updateTime.isValid()) {
            updateTime = QDateTime::fromString(updateTimeStr, Qt::ISODate);
        }
        task.updateTime = updateTime;

        return task;
    }
};


class TaskDBManager : public QObject
{
    Q_OBJECT
private:
    explicit TaskDBManager(QObject *parent = nullptr);
    ~TaskDBManager() override;

    QSqlDatabase m_db;  // 数据库连接

    // 初始化表结构
    bool initTables();
    bool isTableExists(const QString& tableName);

public:
    static TaskDBManager* getInstance();// 单例获取
    bool isConnected() const;// 数据库连接状态
    QSqlDatabase getDB(){return m_db;}

    bool addTask(Task& task);
    bool updateTask(const Task& task);
    bool deleteTask(int taskId);
    QList<Task> getAllTasks();
    QList<Task> getUncompletedTasks();
    Task getTaskById(int taskId);
    QList<Task> getTasksByCategory(const QString& category);
    Task getLatestTask();


private:
    static TaskDBManager* m_instance;  // 单例实例
};

#endif
