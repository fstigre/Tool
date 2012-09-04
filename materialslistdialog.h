#ifndef MATERIALSLISTDIALOG_H
#define MATERIALSLISTDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class MaterialsListDialog;
}

class MaterialsListDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit MaterialsListDialog(QWidget *parent = 0);
    ~MaterialsListDialog();
    
private slots:

    void on_pushButton_EditMaterials_clicked();
    void on_pushButton_SaveChanges_clicked();

private:
    Ui::MaterialsListDialog *ui;
    QString mFilename;
    void editSheet();
};

#endif // MATERIALSLISTDIALOG_H
