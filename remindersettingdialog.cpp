#include "remindersettingdialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>

ReminderSettingDialog::ReminderSettingDialog(int currentThreshold, QWidget *parent)
    : QDialog(parent)
{
    // 弹窗基础设置
    setWindowTitle("提醒阈值设置");
    setFixedSize(300, 150);
    setModal(true);

    // 布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    // 提示文字
    QLabel *tipLabel = new QLabel("设置任务到期前提醒的时间（分钟）：", this);
    mainLayout->addWidget(tipLabel);

    // 阈值输入框（1-1440分钟，即1分钟~24小时）
    m_thresholdSpin = new QSpinBox(this);
    m_thresholdSpin->setRange(1, 1440);
    m_thresholdSpin->setValue(currentThreshold);
    m_thresholdSpin->setSuffix(" 分钟");
    m_thresholdSpin->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_thresholdSpin);

    // 按钮组
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *okBtn = new QPushButton("确定", this);
    QPushButton *cancelBtn = new QPushButton("取消", this);
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    mainLayout->addLayout(btnLayout);

    // 绑定按钮
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

int ReminderSettingDialog::getNewThreshold() const
{
    return m_thresholdSpin->value();
}
