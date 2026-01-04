#ifndef TASKMODEL_H
#define TASKMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include "TaskDBManager.h"

class TaskModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit TaskModel(QObject *parent = nullptr);

    // 重写Model核心方法
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // 自定义刷新数据方法
    void refreshData(const QList<Task>& tasks);

private:
    QList<Task> m_taskList;
    QStringList m_headers = {"ID", "标题", "分类", "优先级", "截止时间", "完成状态", "描述"};
};

#endif // TASKMODEL_H
