#include "taskstatistic.h"

QMap<QString, QPair<int, int>> TaskStatistic::statByCategory()
{
    QMap<QString, QPair<int, int>> statMap;
    QList<Task> tasks = TaskDBManager::getInstance()->getAllTasks();

    for (const Task& task : tasks) {
        if (!statMap.contains(task.category)) statMap[task.category] = {0, 0};

        if (task.isCompleted) statMap[task.category].first++;
        else statMap[task.category].second++;
    }

    return statMap;
}

float TaskStatistic::getCompletionRate()
{
    int total, unfinished;
    statTotal(total, unfinished);
    if (total == 0) return 0.0f;
    return (float)(total - unfinished) / total;
}

void TaskStatistic::statTotal(int& total, int& unfinished)
{
    QList<Task> tasks = TaskDBManager::getInstance()->getAllTasks();
    total = tasks.size();
    unfinished = 0;

    for (const Task& task : tasks) {
        if (!task.isCompleted) unfinished++;
    }
}
