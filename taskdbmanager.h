#ifndef TASKDBMANAGER_H
#define TASKDBMANAGER_H

#include <QObject>

class TaskDBManager : public QObject
{
    Q_OBJECT
public:
    explicit TaskDBManager(QObject *parent = nullptr);

signals:
};

#endif // TASKDBMANAGER_H
