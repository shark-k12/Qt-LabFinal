#include "TaskDBManager.h"

// 静态单例初始化
TaskDBManager* TaskDBManager::m_instance = nullptr;

TaskDBManager::TaskDBManager(QObject *parent) : QObject(parent)
{
    //设置数据库路径
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbPath = "E:/qt_test/QtFinal/QtFinal.db";
    m_db.setDatabaseName(dbPath);

    // 打开数据库
    if (!m_db.open()) {
        qDebug() << "数据库打开失败：" << m_db.lastError().text();
        return;
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

bool TaskDBManager::initTables()
{
    // 执行建表SQL
    QSqlQuery query(m_db);

    // 1. 创建任务表
    QString createTableSql = R"(
        CREATE TABLE IF NOT EXISTS tasks (
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

    // 2. 创建索引
    QStringList indexSqls = {
        "CREATE INDEX IF NOT EXISTS idx_tasks_category ON tasks(category);",
        "CREATE INDEX IF NOT EXISTS idx_tasks_priority ON tasks(priority);",
        "CREATE INDEX IF NOT EXISTS idx_tasks_completed ON tasks(is_completed);",
        "CREATE INDEX IF NOT EXISTS idx_tasks_deadline ON tasks(deadline);"
    };
    for (const QString& sql : indexSqls) {
        if (!query.exec(sql)) {
            qWarning() << "索引创建失败：" << query.lastError().text();
        }
    }

    return true;
}

bool TaskDBManager::isConnected() const
{
    return m_db.isOpen();
}

