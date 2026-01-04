#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QTableView>
#include <QToolBar>
#include <QStatusBar>
#include <QChart>
#include <QChartView>
#include <QSplitter>
#include <QLabel>
#include <QProgressBar>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    QListWidget *m_categoryList;    // 左侧分类导航
    QTableView *m_taskTableView;    // 中间任务列表
    QWidget *m_statWidget;          // 右侧统计面板

    void initMenuBar();
    void initToolBar();
    void initCentralWidget();//布局
    void initStatusBar();
    void initStatPanel();
};

#endif // MAINWINDOW_H
