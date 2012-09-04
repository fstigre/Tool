#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QtGui>
#include <QtCore>

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setColor(const QColor &myCurrentColor);
    const QColor& getColor();

private slots:
    void on_pushButton_Calculate_clicked();
    void on_radioButton_Turret_clicked();
    void on_radioButton_Bystronic_clicked();
    void on_radioButton_Mazak_clicked();
    void on_radioButton_Saw_clicked();
    void on_pushButton_Reset_clicked();
    void on_actionMaterials_List_triggered();
    void on_pushButton_copyYields_clicked();
    void on_pushButton_ColorPick_clicked();
    void on_actionSave_Layout_triggered();
    void on_actionAbout_Tool_triggered();
    void on_pushButton_EditOptions_clicked();
    void on_pushButton_Dxf_clicked();
    void on_pushButton_Pdf_clicked();
    void on_pushButton_Drawing_clicked();
    void on_actionSave_Color_triggered();
    void on_actionReset_Color_triggered();

    void on_pushButton_Clone_clicked();

    void on_pushButton_Check_clicked();

private:
    Ui::MainWindow *ui;
    void calculate();
    void reset();
    void disableButtons();
    void disableOptions();
    void enableOptions();
    void drawPartsX();
    void drawPartsY();
    void loadLayout();
    void saveLayout();
    void loadColor();
    void saveColor();
    void resetColor();
    void foundIt();
    void foundItLessThan();
    void searchingFiles();
    void notFound();
    void notFoundLessThan();
    QGraphicsScene *scene;
    QGraphicsRectItem *rectangle;
    QColor partColor;
    QBrush partBrush;
    QLabel *statusLabel;
    //vars to get text value
    //float partW;
    float partL;
    float sheetL;
    float sheetW;

    float clamp ;
    float webSize;
    float kerfSize;

    float sheetWidth;
    float sheetLength;
    float partWidth;
    float partLength;
    float partWidthPlain;// this is used to draw rectangles wihtout the kerf
    float partLengthPlain;// this is used to draw rectangles wihtout the kerf
    float sheetLengthPlain;
    float sheetWidthPlain;
    float totalPartsGridX;
    float totalPartsGridY;
    int partsXgridX;
    int partsYgridX;
    int partsXgridY;
    int partsYgridY;
    int extraPartsGridX;
    int extraPartsGridY;
    float usedAreaGridX;
    float usedAreaGridY;
    float extraSpaceGridX;
    float extraSpaceGridY;
    int extraPartsXH;
    int extraPartsYH;
    int extraPartsXV;
    int extraPartsYV;
};

#endif // MAINWINDOW_H
