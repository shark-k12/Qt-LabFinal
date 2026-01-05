#include "taskdialog.h"
#include "ui_taskdialog.h"
#include <QMessageBox>
#include <QDateTime>

// 构造函数
TaskDialog::TaskDialog(bool isEdit, const Task& task, QWidget *parent)
    : QDialog(parent), ui(new Ui::TaskDialog), m_isEdit(isEdit), m_task(task)
{
    ui->setupUi(this);
    setWindowTitle(m_isEdit ? "编辑任务" : "新增任务");


    ui->categories->addItems({"未分类", "工作", "生活", "学习", "其他"});
    ui->priority->addItems({"1", "2", "3", "4", "5"});

    initDateTimeEdit();

    if (m_isEdit) {
        ui->title->setText(m_task.title);
        ui->categories->setCurrentText(m_task.category);
        ui->priority->setCurrentText(QString::number(m_task.priority));
        ui->deadline->setDateTime(m_task.deadline);
        ui->iscomplete->setChecked(m_task.isCompleted);
        ui->description->setPlainText(m_task.description);
    }
}

TaskDialog::~TaskDialog()
{
    delete ui;
}

void TaskDialog::initDateTimeEdit()
{
    ui->deadline->setCalendarPopup(true);

    if (!m_isEdit) {
        QDateTime defaultTime = QDateTime::currentDateTime();
        ui->deadline->setDateTime(defaultTime);
    }

    ui->deadline->setDisplayFormat("yyyy-MM-dd HH:mm:ss");

    ui->deadline->setMinimumDateTime(QDateTime::currentDateTime());
}

Task TaskDialog::getTask() const
{
    return m_task;
}

void TaskDialog::on_okBtn_clicked()
{
    QString title = ui->title->text().trimmed();
    if (title.isEmpty()) {
        QMessageBox::warning(this, "提示", "任务标题不能为空！");
        return;
    }

    m_task.title = title;
    m_task.category = ui->categories->currentText();
    m_task.priority = ui->priority->currentText().toInt();
    m_task.deadline = ui->deadline->dateTime();
    m_task.isCompleted = ui->iscomplete->isChecked();
    m_task.description = ui->description->toPlainText().trimmed();

    if (!m_isEdit) {
        m_task.id = -1;
        m_task.createTime = QDateTime::currentDateTime();
        m_task.updateTime = QDateTime::currentDateTime();
    } else {
        m_task.updateTime = QDateTime::currentDateTime();
    }

    accept();
}

void TaskDialog::on_cancelBtn_clicked()
{
    reject();
}
