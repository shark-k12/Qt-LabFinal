#ifndef TASKSTATISTIC_H
#define TASKSTATISTIC_H

#include <QMap>
#include <QList>
#include "taskdbManager.h"

class TaskStatistic
{
public:
    static QMap<QString, QPair<int, int>> statByCategory();
    static float getCompletionRate();
    static void statTotal(int& total, int& unfinished);
};

#endif
