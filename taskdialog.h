#ifndef TASKDIALOG_H
#define TASKDIALOG_H

#include <QDialog>
#include "TaskDBManager.h"


namespace Ui {
class TaskDialog;
}

class TaskDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TaskDialog(bool isEdit = false, const Task& task = Task(), QWidget *parent = nullptr);
    ~TaskDialog();

    // 获取用户输入的任务数据
    Task getTask() const;

private slots:
    void on_okBtn_clicked();

    void on_cancelBtn_clicked();

private:
    Ui::TaskDialog *ui;
    bool m_isEdit;
    Task m_task;
};

#endif
