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

    // 结构体转QVariantMap
    QVariantMap toMap() const {
        QVariantMap map;
        map["title"] = title;
        map["category"] = category;
        map["priority"] = priority;
        map["deadline"] = deadline.toString(Qt::ISODate);
        map["is_completed"] = isCompleted ? 1 : 0;
        map["description"] = description;
        map["create_time"] = createTime.toString(Qt::ISODate);
        map["update_time"] = updateTime.toString(Qt::ISODate);
        return map;
    }

    // QVariantMap转结构体
    static Task fromMap(const QVariantMap& map) {
        Task task;
        task.id = map["id"].toInt();
        task.title = map["title"].toString();
        task.category = map["category"].toString();
        task.priority = map["priority"].toInt();
        task.deadline = QDateTime::fromString(map["deadline"].toString(), Qt::ISODate);
        task.isCompleted = (map["is_completed"].toInt() == 1);
        task.description = map["description"].toString();
        task.createTime = QDateTime::fromString(map["create_time"].toString(), Qt::ISODate);
        task.updateTime = QDateTime::fromString(map["update_time"].toString(), Qt::ISODate);
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
    // 单例获取
    static TaskDBManager* getInstance();

    // 数据库连接状态
    bool isConnected() const;


private:
    static TaskDBManager* m_instance;  // 单例实例
};

#endif // TASKDBMANAGER_H
