#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlTableModel>
#include <QTableView>
#include <QSplitter>
#include <QListWidget>
#include <QProgressBar>
#include <QChart>
#include <QChartView>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    QSqlTableModel *m_taskModel;
    QListWidget *m_categoryList;    // 左侧分类导航
    QTableView *m_taskTableView;    // 中间任务列表
    QWidget *m_statWidget;          // 右侧统计面板

    void initUI();                        // 整体UI
    void initMenuBar();                   // 菜单栏
    void initToolBar();                   // 工具栏
    void initTaskTable();                 // 任务表格
    void initCategoryList();              // 分类导航
    void initStatPanel();                 // 统计面板

private slots:
    void onAbout();
};


#endif
