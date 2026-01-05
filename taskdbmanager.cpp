#include "TaskDBManager.h"
#include <QCoreApplication>

// 静态单例初始化
TaskDBManager* TaskDBManager::m_instance = nullptr;

TaskDBManager::TaskDBManager(QObject *parent) : QObject(parent)
{
    //设置数据库路径
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbPath = "E:/qt_test/QtFinal/QtFinal.db";
    m_db.setDatabaseName(dbPath);
    qDebug() << "程序实际连接的数据库路径：" << dbPath;

    // 打开数据库
    if (!m_db.open()) {
        qCritical() << "数据库打开失败：" << m_db.lastError().text();
        return;
    }
    else{
        qDebug() << "数据库打开成功";
    }

    // 初始化表结构
    if (!initTables()) {
        qCritical() << "表初始化失败！";
    }
}

TaskDBManager::~TaskDBManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

TaskDBManager *TaskDBManager::getInstance()
{
    if (!m_instance) {
        m_instance = new TaskDBManager();
    }
    return m_instance;
}

bool TaskDBManager::isTableExists(const QString& tableName)
{
    if (!isConnected()) return false;

    // SQLite 系统表 sqlite_master 存储所有表/索引信息
    QSqlQuery query(m_db);
    query.prepare("SELECT name FROM sqlite_master WHERE type='table' AND name=:tableName");
    query.bindValue(":tableName", tableName);

    if (!query.exec()) {
        qCritical() << "检查表存在性失败：" << query.lastError().text();
        return false;
    }

    // 有结果 → 表存在
    return query.next();
}

bool TaskDBManager::initTables()
{
    // 先检查 tasks 表是否存在
    if (isTableExists("tasks")) {
        qDebug() << "tasks 表已存在，无需初始化";
        return true;
    }

    // 表不存在 → 执行建表+建索引
    QSqlQuery query(m_db);

    // 1. 创建任务表
    QString createTableSql = R"(
        CREATE TABLE tasks (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            category TEXT NOT NULL DEFAULT '未分类',
            priority INTEGER NOT NULL DEFAULT 3,
            deadline TEXT NOT NULL,
            is_completed INTEGER NOT NULL DEFAULT 0,
            description TEXT,
            create_time TEXT NOT NULL,
            update_time TEXT NOT NULL
        );
    )";
    if (!query.exec(createTableSql)) {
        qCritical() << "建表失败：" << query.lastError().text();
        return false;
    }
    qDebug() << "tasks 表创建成功";

    // 2. 创建索引（索引也可检查存在性，可选）
    QStringList indexSqls = {
        "CREATE INDEX idx_tasks_category ON tasks(category);",
        "CREATE INDEX idx_tasks_priority ON tasks(priority);",
        "CREATE INDEX idx_tasks_completed ON tasks(is_completed);",
        "CREATE INDEX idx_tasks_deadline ON tasks(deadline);"
    };
    for (const QString& sql : indexSqls) {
        if (!query.exec(sql)) {
            qDebug() << "索引创建失败：" << query.lastError().text();
        } else {
            qDebug() << "索引创建成功：" << sql.split(" ")[3];
        }
    }

    return true;
}

bool TaskDBManager::isConnected() const
{
    return m_db.isOpen();
}

QList<Task> TaskDBManager::getAllTasks()
{
    QList<Task> tasks;
    if (!isConnected()) return tasks;

    QSqlQuery query(m_db);
    if (!query.exec("SELECT * FROM tasks")) {
        qCritical() << "查询tasks表失败：" << query.lastError().text();
        return tasks;
    }

    while (query.next()) {
        QVariantMap map;
        map["id"] = query.value("id");
        map["title"] = query.value("title");
        map["category"] = query.value("category");
        map["priority"] = query.value("priority");
        map["deadline"] = query.value("deadline");
        map["is_completed"] = query.value("is_completed");
        map["description"] = query.value("description");
        map["create_time"] = query.value("create_time");
        map["update_time"] = query.value("update_time");
        tasks.append(Task::fromMap(map));
    }

    qDebug() << "读取到的任务数：" << tasks.size();
    return tasks;
}

