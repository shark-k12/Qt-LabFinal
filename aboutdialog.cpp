#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    this->setWindowModality(Qt::ApplicationModal);
    this->setWindowTitle("关于个人任务管理系统");
    this->setFixedSize(400, 300);

    QString styleSheet = R"(
        QDialog {
            background-color: #f5f7fa;
            border-radius: 8px;
        }

        QLabel#titleLabel {
            font-size: 21px;
            font-weight: bold;
            color: #2c3e50;
        }
        QLabel {
            font-size: 14px;
            color: #34495e;
        }
        QLabel#authorLabel, QLabel#emailLabel {
            color: #3498db;
            text-decoration: underline;
        }
        QLabel#authorLabel:hover, QLabel#emailLabel:hover {
            color: #2980b9;
            cursor: pointer;
        }

        QPushButton#author, QPushButton#email {
            background-color: #3498db;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 4px 12px;
            font-size: 14px;
        }

        QPushButton#author:hover, QPushButton#email:hover {
            background-color: #2980b9;
        }
        QPushButton#yes {
            background-color: #3498db;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 6px 16px;
            font-size: 14px;
        }
        QPushButton#yes:hover {
            background-color: #2980b9;
        }
    )";
    this->setStyleSheet(styleSheet);

    ui->author->setToolTip("点击跳转github地址");
    ui->email->setToolTip("点击复制邮箱");
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::on_author_clicked()
{
    QDesktopServices::openUrl(QUrl("https://github.com/shark-k12/Qt-LabFinal", QUrl::TolerantMode));
}

void AboutDialog::on_email_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(ui->email->text());
    QMessageBox::information(this, "提示", "邮箱已复制到剪贴板");
}

void AboutDialog::on_yes_clicked()
{
    this->close();
}

