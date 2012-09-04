#include "materialslistdialog.h"
#include "ui_materialslistdialog.h"
#include <QFile>
#include <QString>
#include <QDebug>
#include <QTextStream>
#include <QMessageBox>
#include <QDir>
#include <QInputDialog>

MaterialsListDialog::MaterialsListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MaterialsListDialog){
    ui->setupUi(this);


    ui->textEdit_MaterialsList->setReadOnly(true);
    ui->pushButton_SaveChanges->setDisabled(true);

    //ui->textEdit_MaterialsList->setStyleSheet("color:green");
    //QString file= "/Users/dolphin/My_Applications/QtApplications/MaterialRef.html";
    //QString file= "X:/Kalculator/docs/MaterialRef.html";
    QString file= "X:/Engineering_Utilities/Applications/docs/MaterialRef.html";


    QFile sFile(file);
    if(!sFile.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::information(this,"File Open","Could not open file for reading access");
        return;
    }
    mFilename = file;
    QTextStream in(&sFile);
    QString text = in.readAll();

    //ui->textEdit_MaterialsList->setPlainText(text);
    ui->textEdit_MaterialsList->setHtml(text);

    sFile.flush();
    sFile.close();
}

MaterialsListDialog::~MaterialsListDialog()
{
    delete ui;
}


void MaterialsListDialog::on_pushButton_EditMaterials_clicked()
{
    bool ok;
    QString text = QInputDialog::getText(this,
                                         tr("Edit Materials"),
                                         tr("Enter Password:"),
                                         QLineEdit::Password,
                                         "",
                                         &ok);

    if(text == "")
    {
        // do nothing
    }
    else if (ok && text == "eng1")
    {
        editSheet();
    }
    else
    {
        QMessageBox::information(this,"Information","Wrong Password");
    }
}

// this fuction will only be called if user typed the right password
void MaterialsListDialog::editSheet()
{
    ui->textEdit_MaterialsList->setReadOnly(false);
    ui->pushButton_SaveChanges->setDisabled(false);
    ui->pushButton_EditMaterials->setDisabled(true);
    ui->textEdit_MaterialsList->setStyleSheet("background-color:#aaa");
}



void MaterialsListDialog::on_pushButton_SaveChanges_clicked()
{
    ui->pushButton_SaveChanges->setDisabled(true);
    ui->textEdit_MaterialsList->setReadOnly(true);
    ui->pushButton_EditMaterials->setDisabled(false);
    QMessageBox::information(this, "Information","Your changes have been saved");


    QFile sFile(mFilename);
    if(sFile.open(QFile::WriteOnly | QFile::Text))
    {
        QTextStream out(&sFile);
        out << ui->textEdit_MaterialsList->toHtml();

        sFile.flush();
        sFile.close();

    }
}
