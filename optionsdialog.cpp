#include "optionsdialog.h"
#include "ui_optionsdialog.h"

OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);
    ui->comboBox_Colors->addItem("Green");
    ui->comboBox_Colors->addItem("Gray");
    ui->comboBox_Colors->addItem("White");
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}
