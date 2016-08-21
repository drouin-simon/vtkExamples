#include "mfdialog.h"
#include "ui_mfdialog.h"

MFDialog::MFDialog(QWidget *parent) : QDialog(parent), ui(new Ui::MFDialog)
{
    ui->setupUi(this);
}

MFDialog::~MFDialog()
{
    delete ui;
}
