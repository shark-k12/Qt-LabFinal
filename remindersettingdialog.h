#ifndef REMINDERSETTINGDIALOG_H
#define REMINDERSETTINGDIALOG_H

#include <QDialog>
#include <QSpinBox>

class ReminderSettingDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ReminderSettingDialog(int currentThreshold, QWidget *parent = nullptr);
    ~ReminderSettingDialog() override = default;

    // 获取用户设置的新阈值
    int getNewThreshold() const;

private:
    QSpinBox *m_thresholdSpin; // 阈值输入框
};

#endif
