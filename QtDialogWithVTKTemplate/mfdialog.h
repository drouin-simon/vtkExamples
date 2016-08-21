#ifndef MFDIALOG_H
#define MFDIALOG_H

#include <QDialog>

namespace Ui {
class MFDialog;
}

class MFDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MFDialog(QWidget *parent = 0);
    ~MFDialog();

private:
    Ui::MFDialog *ui;
};

#endif // MFDIALOG_H
