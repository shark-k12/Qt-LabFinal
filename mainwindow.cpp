#include "mainwindow.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QAction>
#include <QHeaderView>
#include <QPieSeries>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QApplication>
#include <QStandardItemModel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("个人工作与任务管理系统");
    setMinimumSize(1000, 600); // 最小尺寸避免布局错乱
    setStyleSheet(R"(
        /* 全局样式 */
        QWidget { font-family: "Microsoft YaHei"; font-size: 14px; }
        /* 列表选中样式 */
        QListWidget::item:selected { background-color: #2196F3; color: white; }
        QListWidget::item { padding: 8px; }
        /* 表格样式 */
        QTableView { gridline-color: #E0E0E0; }
        QHeaderView::section { background-color: #F5F5F5; padding: 6px; }
        /* 进度条样式 */
        QProgressBar { border: 1px solid #E0E0E0; border-radius: 5px; text-align: center; }
        QProgressBar::chunk { background-color: #4CAF50; }
        /* 分割线样式 */
        QSplitter::handle { background-color: #E0E0E0; }
    )");

    initMenuBar();
    initToolBar();
    initCentralWidget();
    initStatusBar();
}

MainWindow::~MainWindow() = default;


void MainWindow::initMenuBar()
{
    QMenuBar *menuBar = this->menuBar();

    // 文件菜单
    QMenu *fileMenu = menuBar->addMenu(tr("文件(&F)"));
    fileMenu->addAction(tr("导出Excel报表(&E)"));
    fileMenu->addAction(tr("导出PDF报表(&P)"));
    fileMenu->addSeparator();
    fileMenu->addAction(tr("退出(&X)"));

    // 编辑菜单
    QMenu *editMenu = menuBar->addMenu(tr("编辑(&E)"));
    editMenu->addAction(tr("新增任务(&N)"));
    editMenu->addAction(tr("修改任务(&M)"));
    editMenu->addAction(tr("删除任务(&D)"));

    // 设置菜单
    QMenu *settingMenu = menuBar->addMenu(tr("设置(&S)"));
    settingMenu->addAction(tr("提醒阈值设置(&R)"));
    settingMenu->addAction(tr("优先级设置(&P)"));

    // 帮助菜单
    QMenu *helpMenu = menuBar->addMenu(tr("帮助(&H)"));
    QAction *aboutAct = helpMenu->addAction(tr("关于(&A)"));

    connect(aboutAct, &QAction::triggered, this, &MainWindow::onAbout);
}


void MainWindow::initToolBar()
{
    QToolBar *toolBar = this->addToolBar(tr("快捷操作"));
    toolBar->setIconSize(QSize(24, 24)); // 按钮图标大小

    // 工具栏按钮（使用Qt内置图标，无图标则显示文字）
    toolBar->addAction(QIcon::fromTheme("list-add"), tr("新增任务"));
    toolBar->addAction(QIcon::fromTheme("list-remove"), tr("删除任务"));
    toolBar->addAction(QIcon::fromTheme("document-edit"), tr("修改任务"));
    toolBar->addSeparator(); // 分隔线
    toolBar->addAction(QIcon::fromTheme("view-sort"), tr("按优先级排序"));
    toolBar->addAction(QIcon::fromTheme("view-refresh"), tr("刷新"));
    toolBar->addSeparator();
    toolBar->addAction(QIcon::fromTheme("document-export"), tr("导出报表"));
}


void MainWindow::initCentralWidget()
{
    // 核心容器
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *centralLayout = new QHBoxLayout(centralWidget);
    centralLayout->setSpacing(5);
    centralLayout->setContentsMargins(5, 5, 5, 5);
    setCentralWidget(centralWidget);

    // 分割器（支持拖拽调整宽度）
    QSplitter *splitter = new QSplitter(Qt::Horizontal, centralWidget);
    splitter->setHandleWidth(2);
    centralLayout->addWidget(splitter);

    // --------------------- 左侧：分类导航 ---------------------
    m_categoryList = new QListWidget(splitter);
    m_categoryList->setMinimumWidth(150);
    m_categoryList->setMaximumWidth(200);
    QStringList categories = {"全部任务", "未分类", "工作", "生活", "学习", "其他"};
    m_categoryList->addItems(categories);
    // 默认选中第一个
    m_categoryList->setCurrentRow(0);



    // --------------------- 中间：任务列表 ---------------------
    m_taskTableView = new QTableView(splitter);
    // 表格配置
    m_taskTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_taskTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_taskTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_taskTableView->horizontalHeader()->setStretchLastSection(true);
    m_taskTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    // 设置表头
    QStringList headers = {"ID", "任务标题", "分类", "优先级", "截止时间", "完成状态", "描述"};
    // 临时创建空Model用于显示表头
    auto *tempModel = new QStandardItemModel(this);
    tempModel->setHorizontalHeaderLabels(headers);
    m_taskTableView->setModel(tempModel);


    // --------------------- 右侧：统计面板 ---------------------
    m_statWidget = new QWidget(splitter);
    m_statWidget->setMinimumWidth(200);
    m_statWidget->setMaximumWidth(300);
    initStatPanel();
}

// 初始化右侧统计面板
void MainWindow::initStatPanel()
{
    QVBoxLayout *statLayout = new QVBoxLayout(m_statWidget);
    statLayout->setSpacing(10);
    statLayout->setContentsMargins(10, 10, 10, 10);

    // 1. 标题
    QLabel *titleLabel = new QLabel(tr("任务完成统计"), m_statWidget);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    statLayout->addWidget(titleLabel);

    // 2. 统计数字面板
    QWidget *numWidget = new QWidget(m_statWidget);
    QVBoxLayout *numLayout = new QVBoxLayout(numWidget);
    numLayout->setSpacing(5);
    numLayout->addWidget(new QLabel(tr("总任务数：0"), numWidget));
    numLayout->addWidget(new QLabel(tr("未完成数：0"), numWidget));
    numLayout->addWidget(new QLabel(tr("完成率：0%"), numWidget));
    statLayout->addWidget(numWidget);

    // 3. 完成率进度条
    QProgressBar *rateBar = new QProgressBar(m_statWidget);
    rateBar->setRange(0, 100);
    rateBar->setValue(0);
    statLayout->addWidget(rateBar);

    // 4. 分类占比饼图
    QChart *pieChart = new QChart();
    pieChart->setTitle(tr("任务分类占比"));
    QPieSeries *pieSeries = new QPieSeries();
    pieChart->addSeries(pieSeries);
    QChartView *chartView = new QChartView(pieChart, m_statWidget);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(200);
    statLayout->addWidget(chartView);

    // 5. 占位符（拉伸底部）
    statLayout->addStretch();
}

void MainWindow::onAbout()
{
    AboutDialog dlg(this);
    dlg.exec();
}

// 初始化状态栏
void MainWindow::initStatusBar()
{
    QStatusBar *statusBar = this->statusBar();

    // 左侧：数据库状态
    QLabel *dbStatusLabel = new QLabel(tr("数据库已连接"), statusBar);
    dbStatusLabel->setStyleSheet("color: #4CAF50;");
    statusBar->addWidget(dbStatusLabel);

    // 右侧：任务统计
    QLabel *taskStatLabel = new QLabel(tr("总任务：0 | 未完成：0"), statusBar);
    statusBar->addPermanentWidget(taskStatLabel);
}
