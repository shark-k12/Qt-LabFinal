#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "taskdbmanager.h"
#include "reminderthread.h"
#include "remindersettingdialog.h"
#include <QMainWindow>
#include <QSqlTableModel>
#include <QTableView>
#include <QSplitter>
#include <QListWidget>
#include <QProgressBar>
#include <QChart>
#include <QLabel>
#include <QChartView>
#include <QPieSeries>
#include <QSystemTrayIcon>
#include <QMenu>

#include <windows.h>
#include <shellapi.h>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QSqlTableModel *m_taskModel;
    QListWidget *m_categoryList;    // 左侧分类导航
    QTableView *m_taskTableView;    // 中间任务列表
    QWidget *m_statWidget;          // 右侧统计面板
    QProgressBar *m_completionBar;        // 完成率进度条
    QLabel *m_totalLabel, *m_unfinishedLabel, *m_rateLabel; // 统计标签
    QChartView *m_pieChartView;           // 饼图
    ReminderThread *m_reminderThread;
    QSystemTrayIcon *m_systemTray; // 系统托盘



    void initUI();                        // 整体UI
    void initMenuBar();                   // 菜单栏
    void initToolBar();                   // 工具栏
    void initTaskTable();                 // 任务表格
    void initCategoryList();              // 分类导航
    void initStatPanel();                 // 统计面板
    void initReminderThread();            // 提醒线程

    void updateStatusBar(int total, int unfinished);
    int countUnfinished(const QList<Task>& tasks);
    void refreshStatPanel();
    void updateLatestTaskStatus();
    void initSystemTray(); // 初始化托盘

private slots:
    void onAddTask();
    void onEditTask();
    void onDeleteTask();
    void onSortByPriority();
    void onRefresh();
    void onAbout();
    void onExportExcel();
    void onExportPdf();
    void onCategoryChanged(const QString& category);
    void onTaskReminder(const QString& msg); // 接收提醒信号的槽函数
    void onSetReminderThreshold(); // 打开阈值设置弹窗
};


#endif
