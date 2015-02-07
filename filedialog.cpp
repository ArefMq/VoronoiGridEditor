#include "filedialog.h"
#include "ui_filedialog.h"

FileDialog::FileDialog(QWidget *parent) :
    QDialog(parent),
    hasAccepted(false),
    ui(new Ui::FileDialog)
{
    ui->setupUi(this);
}

FileDialog::FileDialog(QString str, QWidget *parent) :
    QDialog(parent),
    hasAccepted(false),
    ui(new Ui::FileDialog)
{
    ui->setupUi(this);
    ui->lineEdit->setText(str);
}

FileDialog::~FileDialog()
{
    delete ui;
}

void FileDialog::on_buttonBox_rejected()
{
    hasAccepted = false;
}

void FileDialog::on_buttonBox_accepted()
{
    hasAccepted = true;
}

QString FileDialog::address()
{
    return ui->lineEdit->text();
}
