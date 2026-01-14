#include "mainwindow.h"
#include "taskdialog.h"
#include "aboutdialog.h"
#include "remindersettingdialog.h"
#include "xlsxdocument.h"
#include "taskstatistic.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QPrinter>
#include <QPieSeries>
#include <QDebug>

using namespace QXlsx;

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

    initUI();
    initMenuBar();
    initToolBar();
    initTaskTable();
    initCategoryList();
    initStatPanel();
    initReminderThread();
    onRefresh();

    if (!TaskDBManager::getInstance()->isConnected()) {
        QMessageBox::critical(this, "错误", "数据库连接失败！");
    }
}

void MainWindow::initUI()
{
    // 主布局
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    setCentralWidget(centralWidget);

    // 分割器（左：分类，中：表格，右：统计）
    QSplitter *splitter = new QSplitter(Qt::Horizontal, centralWidget);
    splitter->setHandleWidth(2);
    mainLayout->addWidget(splitter);

    // 1. 左侧分类导航
    m_categoryList = new QListWidget(this);
    m_categoryList->setMinimumWidth(70);
    m_categoryList->setMaximumWidth(100);
    splitter->addWidget(m_categoryList);

    // 2. 中间任务表格
    m_taskTableView = new QTableView(this);
    splitter->addWidget(m_taskTableView);

    // 3. 右侧统计面板
    m_statWidget = new QWidget(this);
    m_statWidget->setMinimumWidth(300);
    splitter->addWidget(m_statWidget);

    // 表格基础配置
    m_taskTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_taskTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_taskTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_taskTableView->horizontalHeader()->setStretchLastSection(true);
    m_taskTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // 状态栏
    QStatusBar *statusBar = new QStatusBar(this);
    setStatusBar(statusBar);

    QLabel *dbStatusLabel = new QLabel("数据库已连接", this);
    dbStatusLabel->setObjectName("dbStatusLabel");
    statusBar->addWidget(dbStatusLabel);

    QLabel *taskStatLabel = new QLabel("总任务：0 | 未完成：0", this);
    taskStatLabel->setObjectName("taskStatLabel");
    statusBar->addPermanentWidget(taskStatLabel);
}

MainWindow::~MainWindow()
{
    // 安全释放提醒线程
    if (m_reminderThread) {
        delete m_reminderThread;
        m_reminderThread = nullptr;
    }
}

void MainWindow::initMenuBar()
{
    QMenuBar *menuBar = this->menuBar();

    // 文件菜单
    QMenu *fileMenu = menuBar->addMenu(tr("文件(&F)"));
    QAction *exportExcelAct = fileMenu->addAction("导出Excel(&E)");
    QAction *exportPdfAct = fileMenu->addAction("导出PDF(&P)");
    fileMenu->addSeparator();
    fileMenu->addAction(tr("退出(&X)"));

    // 编辑菜单
    QMenu *editMenu = menuBar->addMenu(tr("编辑(&E)"));
    QAction *addAct = editMenu->addAction("新增任务(&N)");
    QAction *editAct = editMenu->addAction("编辑任务(&M)");
    QAction *deleteAct = editMenu->addAction("删除任务");

    // 设置菜单
    QMenu *settingMenu = menuBar->addMenu(tr("设置(&S)"));
    QAction *reminderThresholdAct = settingMenu->addAction("提醒阈值设置(&R)");
    QAction *sortAct = settingMenu->addAction("按优先级排序(&P)");

    // 帮助菜单
    QMenu *helpMenu = menuBar->addMenu(tr("帮助(&H)"));
    QAction *aboutAct = helpMenu->addAction(tr("关于(&A)"));

    connect(addAct, &QAction::triggered, this, &MainWindow::onAddTask);
    connect(editAct, &QAction::triggered, this, &MainWindow::onEditTask);
    connect(deleteAct, &QAction::triggered, this, &MainWindow::onDeleteTask);
    connect(reminderThresholdAct, &QAction::triggered, this, &MainWindow::onSetReminderThreshold);
    connect(sortAct, &QAction::triggered, this, &MainWindow::onSortByPriority);
    connect(exportExcelAct, &QAction::triggered, this, &MainWindow::onExportExcel);
    connect(exportPdfAct, &QAction::triggered, this, &MainWindow::onExportPdf);
    connect(aboutAct, &QAction::triggered, this, &MainWindow::onAbout);
}


void MainWindow::initToolBar()
{
    QToolBar *toolBar = this->addToolBar(tr("快捷操作"));
    toolBar->setIconSize(QSize(24, 24)); // 按钮图标大小

    // 工具栏按钮（使用Qt内置图标，无图标则显示文字）
    QAction *addAct = toolBar->addAction(QIcon::fromTheme("list-add"), tr("新增任务"));
    QAction *deleteAct = toolBar->addAction(QIcon::fromTheme("list-remove"), tr("删除任务"));
    QAction *editAct = toolBar->addAction("编辑任务");
    toolBar->addSeparator(); // 分隔线
    QAction *sortAct = toolBar->addAction("按优先级排序");
    QAction *refreshAct = toolBar->addAction(QIcon::fromTheme("view-refresh"), tr("刷新"));
    toolBar->addSeparator();
    QAction *reminderSettingAct = toolBar->addAction("提醒设置");
    QAction *exportAct = toolBar->addAction(QIcon::fromTheme("document-export"), tr("导出报表"));

    connect(addAct, &QAction::triggered, this, &MainWindow::onAddTask);
    connect(editAct, &QAction::triggered, this, &MainWindow::onEditTask);
    connect(deleteAct, &QAction::triggered, this, &MainWindow::onDeleteTask);
    connect(sortAct, &QAction::triggered, this, &MainWindow::onSortByPriority);
    connect(refreshAct, &QAction::triggered, this, &MainWindow::onRefresh);
    connect(reminderSettingAct, &QAction::triggered, this, &MainWindow::onSetReminderThreshold);
    connect(exportAct, &QAction::triggered, this, &MainWindow::onExportExcel);
}

void MainWindow::initTaskTable()
{
    // 使用QSqlTableModel绑定数据库表
    m_taskModel = new QSqlTableModel(this, TaskDBManager::getInstance()->getDB());
    m_taskModel->setTable("tasks");
    m_taskModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_taskModel->setSort(m_taskModel->fieldIndex("update_time"), Qt::DescendingOrder);
    if(m_taskModel->select()){
        qDebug() << "实际执行的 SQL：" << m_taskModel->query().lastQuery();
        qDebug() << "表格加载行数：" << m_taskModel->rowCount();
    }

    // 自定义表头
    m_taskModel->setHeaderData(m_taskModel->fieldIndex("id"), Qt::Horizontal, "ID");
    m_taskModel->setHeaderData(m_taskModel->fieldIndex("title"), Qt::Horizontal, "任务标题");
    m_taskModel->setHeaderData(m_taskModel->fieldIndex("category"), Qt::Horizontal, "分类");
    m_taskModel->setHeaderData(m_taskModel->fieldIndex("priority"), Qt::Horizontal, "优先级");
    m_taskModel->setHeaderData(m_taskModel->fieldIndex("deadline"), Qt::Horizontal, "截止时间");
    m_taskModel->setHeaderData(m_taskModel->fieldIndex("is_completed"), Qt::Horizontal, "完成状态");
    m_taskModel->setHeaderData(m_taskModel->fieldIndex("description"), Qt::Horizontal, "描述");

    // 绑定到表格
    m_taskTableView->setModel(m_taskModel);

    // 隐藏不需要的字段
    m_taskTableView->hideColumn(m_taskModel->fieldIndex("create_time"));
    m_taskTableView->hideColumn(m_taskModel->fieldIndex("update_time"));
}

void MainWindow::initCategoryList()
{
    m_categoryList->addItems({"全部任务", "未分类", "工作", "生活", "学习", "其他"});
    m_categoryList->setCurrentRow(0);

    connect(m_categoryList, &QListWidget::currentTextChanged, this, &MainWindow::onCategoryChanged);
}

void MainWindow::initStatPanel()
{
    QVBoxLayout *statLayout = new QVBoxLayout(m_statWidget);
    statLayout->setSpacing(15);
    statLayout->setContentsMargins(20, 20, 20, 20);

    // 1. 标题
    QLabel *titleLabel = new QLabel("任务统计", m_statWidget);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    statLayout->addWidget(titleLabel);

    // 2. 基础统计
    QWidget *baseStatWidget = new QWidget(m_statWidget);
    QVBoxLayout *baseLayout = new QVBoxLayout(baseStatWidget);
    m_totalLabel = new QLabel("总任务数：0", baseStatWidget);
    m_unfinishedLabel = new QLabel("未完成数：0", baseStatWidget);
    m_rateLabel = new QLabel("完成率：0%", baseStatWidget);
    baseLayout->addWidget(m_totalLabel);
    baseLayout->addWidget(m_unfinishedLabel);
    baseLayout->addWidget(m_rateLabel);
    statLayout->addWidget(baseStatWidget);

    // 3. 完成率进度条
    m_completionBar = new QProgressBar(m_statWidget);
    m_completionBar->setRange(0, 100);
    m_completionBar->setValue(0);
    statLayout->addWidget(m_completionBar);

    // 4. 分类饼图
    m_pieChartView = new QChartView(m_statWidget);
    m_pieChartView->setMinimumHeight(200);
    statLayout->addWidget(m_pieChartView);

    // 5. 占位符
    statLayout->addStretch();
}

void MainWindow::initReminderThread()
{
    m_reminderThread = new ReminderThread(this);
    // 绑定提醒信号到槽函数
    connect(m_reminderThread, &ReminderThread::reminder,
            this, &MainWindow::onTaskReminder);
    // 启动线程
    m_reminderThread->start();
    qDebug() << "提醒线程已启动，默认阈值：30分钟";
}

void MainWindow::updateStatusBar(int total, int unfinished)
{
    QLabel *taskStatLabel = statusBar()->findChild<QLabel*>("taskStatLabel");
    if (taskStatLabel) {
        taskStatLabel->setText(QString("总任务：%1 | 未完成：%2").arg(total).arg(unfinished));
    }
}

int MainWindow::countUnfinished(const QList<Task>& tasks)
{
    int count = 0;
    for (const Task& task : tasks) {
        if (!task.isCompleted) count++;
    }
    return count;
}

void MainWindow::refreshStatPanel()
{

    QList<Task> allTasks = TaskDBManager::getInstance()->getAllTasks();
    qDebug() << "统计时查询到的任务数：" << allTasks.size();

    // 获取统计数据
    int total, unfinished;
    TaskStatistic::statTotal(total, unfinished);
    float rate = TaskStatistic::getCompletionRate() * 100;
    QMap<QString, QPair<int, int>> statMap = TaskStatistic::statByCategory();

    // 更新基础统计
    m_totalLabel->setText(QString("总任务数：%1").arg(total));
    m_unfinishedLabel->setText(QString("未完成数：%1").arg(unfinished));
    m_rateLabel->setText(QString("完成率：%1%").arg(QString::number(rate, 'f', 1)));
    m_completionBar->setValue((int)rate);

    // 更新饼图
    QPieSeries *series = new QPieSeries();
    for (auto it = statMap.begin(); it != statMap.end(); ++it) {
        int count = it.value().first + it.value().second;
        series->append(it.key(), count);
    }

    // 饼图样式
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("任务分类占比");
    chart->legend()->setAlignment(Qt::AlignBottom);
    m_pieChartView->setChart(chart);
    m_pieChartView->setRenderHint(QPainter::Antialiasing);

    // 更新状态栏
    updateStatusBar(total, unfinished);
}



void MainWindow::onAddTask()
{
    TaskDialog dlg(false);
    if (dlg.exec() == QDialog::Accepted) {
        Task task = dlg.getTask();
        if (TaskDBManager::getInstance()->addTask(task)) {
            QMessageBox::information(this, "提示", "任务新增成功！");
            onRefresh();
        } else {
            QMessageBox::critical(this, "错误", "任务新增失败！");
        }
    }
}

void MainWindow::onEditTask()
{
    QModelIndex index = m_taskTableView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "提示", "请先选中要编辑的任务！");
        return;
    }

    int taskId = m_taskModel->data(m_taskModel->index(index.row(), 0)).toInt();
    Task task = TaskDBManager::getInstance()->getTaskById(taskId);

    TaskDialog dlg(true, task); // 编辑模式
    if (dlg.exec() == QDialog::Accepted) {
        Task updatedTask = dlg.getTask();
        if (TaskDBManager::getInstance()->updateTask(updatedTask)) {
            QMessageBox::information(this, "提示", "任务编辑成功！");
            onRefresh();
        }
    }
}

void MainWindow::onDeleteTask()
{
    QModelIndex curIndex = m_taskTableView->currentIndex();
    if (!curIndex.isValid()) {
        QMessageBox::warning(this, "提示", "请先选中要删除的任务！");
        return;
    }

    if (QMessageBox::question(this, "确认", "是否确定删除该任务？") != QMessageBox::Yes) {
        return;
    }

    int taskId = m_taskModel->data(m_taskModel->index(curIndex.row(), m_taskModel->fieldIndex("id"))).toInt();
    if (TaskDBManager::getInstance()->deleteTask(taskId)) {
        QMessageBox::information(this, "提示", "任务删除成功！");
        onRefresh();
    } else {
        QMessageBox::critical(this, "错误", "任务删除失败！");
    }
}

void MainWindow::onSortByPriority()
{
    m_taskModel->setSort(m_taskModel->fieldIndex("priority"), Qt::DescendingOrder);
    m_taskModel->select();
}

void MainWindow::onRefresh()
{
    m_taskModel->select();

    refreshStatPanel();

    // 更新状态栏统计
    QList<Task> allTasks = TaskDBManager::getInstance()->getAllTasks();
    QList<Task> uncompletedTasks = TaskDBManager::getInstance()->getUncompletedTasks();
    updateStatusBar(allTasks.size(), uncompletedTasks.size());
}

void MainWindow::onAbout()
{
    AboutDialog dlg(this);
    dlg.exec();
}

void MainWindow::onExportExcel()
{
    QString filePath = QFileDialog::getSaveFileName(this, "导出Excel", "任务统计.xlsx", "Excel文件 (*.xlsx)");
    if (filePath.isEmpty()) return;

    QXlsx::Document xlsx;
    // 表头
    xlsx.write("A1", "ID");
    xlsx.write("B1", "任务标题");
    xlsx.write("C1", "分类");
    xlsx.write("D1", "优先级");
    xlsx.write("E1", "截止时间");
    xlsx.write("F1", "完成状态");
    xlsx.write("G1", "描述");

    // 数据
    QList<Task> tasks = TaskDBManager::getInstance()->getAllTasks();
    int row = 2;
    for (const Task& task : tasks) {
        xlsx.write(row, 1, task.id);
        xlsx.write(row, 2, task.title);
        xlsx.write(row, 3, task.category);
        xlsx.write(row, 4, task.priority);
        xlsx.write(row, 5, task.deadline.toString("yyyy-MM-dd HH:mm"));
        xlsx.write(row, 6, task.isCompleted ? "已完成" : "未完成");
        xlsx.write(row, 7, task.description);
        row++;
    }

    if (xlsx.saveAs(filePath)) {
        QMessageBox::information(this, "提示", "Excel导出成功！");
    } else {
        QMessageBox::critical(this, "错误", "Excel导出失败！");
    }
}

void MainWindow::onExportPdf()
{
    QString filePath = QFileDialog::getSaveFileName(this, "导出PDF", "任务统计.pdf", "PDF文件 (*.pdf)");
    if (filePath.isEmpty()) return;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize::A4);

    QPainter painter(&printer);
    // 标题
    painter.setFont(QFont("Microsoft YaHei", 18, QFont::Bold));
    painter.drawText(100, 50, "任务统计报表");

    // 表头
    painter.setFont(QFont("Microsoft YaHei", 12, QFont::Bold));
    int x = 50, y = 100;
    painter.drawText(x, y, "ID");
    painter.drawText(x + 80, y, "任务标题");
    painter.drawText(x + 200, y, "分类");
    painter.drawText(x + 300, y, "优先级");
    painter.drawText(x + 380, y, "截止时间");
    painter.drawText(x + 500, y, "完成状态");

    // 数据
    painter.setFont(QFont("Microsoft YaHei", 10));
    y += 30;
    QList<Task> tasks = TaskDBManager::getInstance()->getAllTasks();
    for (const Task& task : tasks) {
        painter.drawText(x, y, QString::number(task.id));
        painter.drawText(x + 80, y, task.title);
        painter.drawText(x + 200, y, task.category);
        painter.drawText(x + 300, y, QString::number(task.priority));
        painter.drawText(x + 380, y, task.deadline.toString("yyyy-MM-dd HH:mm"));
        painter.drawText(x + 500, y, task.isCompleted ? "已完成" : "未完成");
        y += 25;
    }

    painter.end();
    QMessageBox::information(this, "提示", "PDF导出成功！");
}

void MainWindow::onCategoryChanged(const QString& category)
{
    if (category == "全部任务") {
        m_taskModel->setFilter(""); // 清空筛选
    } else {
        m_taskModel->setFilter(QString("category = '%1'").arg(category));
    }
    m_taskModel->select();

    // 更新统计
    if (category == "全部任务") {
        QList<Task> allTasks = TaskDBManager::getInstance()->getAllTasks();
        QList<Task> uncompletedTasks = TaskDBManager::getInstance()->getUncompletedTasks();
        updateStatusBar(allTasks.size(), uncompletedTasks.size());
    } else {
        QList<Task> categoryTasks = TaskDBManager::getInstance()->getTasksByCategory(category);
        updateStatusBar(categoryTasks.size(), countUnfinished(categoryTasks));
    }
}

void MainWindow::onTaskReminder(const QString& msg)
{
    QMessageBox::information(this, "任务提醒", msg);
}

void MainWindow::onSetReminderThreshold()
{
    // 1. 获取当前提醒线程的阈值
    int currentThreshold = m_reminderThread->getReminderThreshold(); // 先给ReminderThread加get方法

    // 2. 打开设置弹窗
    ReminderSettingDialog dlg(currentThreshold, this);
    if (dlg.exec() == QDialog::Accepted) {
        // 3. 获取用户设置的新阈值并更新到提醒线程
        int newThreshold = dlg.getNewThreshold();
        m_reminderThread->setReminderThreshold(newThreshold);

        // 4. 提示用户设置成功
        QMessageBox::information(this, "设置成功",
                                 QString("提醒阈值已设置为：%1 分钟").arg(newThreshold));
    }
}




