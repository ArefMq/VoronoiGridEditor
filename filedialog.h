#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <QDialog>

namespace Ui {
class FileDialog;
}

class FileDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit FileDialog(QWidget *parent = 0);
    explicit FileDialog(QString str, QWidget *parent = 0);
    ~FileDialog();
    
    bool hasAccepted;
    QString address();

private slots:
    void on_buttonBox_rejected();
    void on_buttonBox_accepted();

private:
    Ui::FileDialog *ui;
};

#endif // FILEDIALOG_H
