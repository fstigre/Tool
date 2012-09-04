#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtGui>
#include <QtCore>
#include <QMessageBox>
#include <QString>
#include <QDebug>
#include "materialslistdialog.h"
#include <QClipboard>
#include <QColor>
#include <QColorDialog>
#include <QBrush>
#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // this changes the background color of the widget.
    //ui->tabWidget->setStyleSheet(QString::fromUtf8("background-color: rgb(170, 170, 170);"));
    // initial values for input fields
    ui->doubleSpinBox_SheetWidth->setRange(0,100);
    ui->doubleSpinBox_SheetLength->setRange(0,150);
    ui->doubleSpinBox_SheetWidth->setValue(60);
    ui->doubleSpinBox_SheetLength->setValue(120);
    ui->doubleSpinBox_Clamp->setValue(3);
    ui->doubleSpinBox_Kerf->setValue(.250);
    ui->doubleSpinBox_Web->setValue(.750);
    ui->label_InfoScreen->setText("Turrets");
    ui->label_InfoScreen->setStyleSheet("QLabel {color:#ccc;}");
    // make labels mouse selectable
    ui->label_TotalParts->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ui->label_Yields->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ui->label_Message->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ui->label_Message->setStyleSheet("QLabel {color:#aaa}");

    ui->pushButton_Clone->setStyleSheet("color:#666");

    ui->label_Yields->setStyleSheet("QLabel {color:#000;  font-size:22px; font-weight:none;}");
    ui->label_TotalParts->setStyleSheet("QLabel {color:#333;  font-size:22px; font-weight:none;}");

    //disable buttons at starting
    ui->pushButton_Reset->setDisabled(true);
    ui->pushButton_copyYields->setDisabled(true);
    //painting sheet
    ui->graphicsView_Sheet->setStyleSheet("background-color:#777");

    disableOptions();

    //this will assign a shortcut to the calculate button
    //shortcut for the return key
    QShortcut *returnShortcut = new QShortcut(QKeySequence("Return"), ui->groupBox);
    QObject::connect(returnShortcut, SIGNAL(activated()), ui->pushButton_Calculate, SLOT(click()));
    //shortcut for the enter key
    QShortcut *enterShortcut = new QShortcut(QKeySequence("Enter"), ui->groupBox);
    QObject::connect(enterShortcut, SIGNAL(activated()), ui->pushButton_Calculate, SLOT(click()));

    // statusbar label
    statusLabel = new QLabel(this);
    statusLabel->setText("   Status");
    ui->statusBar->addWidget(statusLabel);
    ui->statusBar->setStyleSheet("color:#333");

    //************* FINDER ******************************

    ui->graphicsView_Drawing->setStyleSheet("background-color:#FFD700");
    ui->graphicsView_Pdf->setStyleSheet("background-color:#FFD700");
    ui->graphicsView_Dxf->setStyleSheet("background-color:#FFD700");

    //************* END FINDER *************************

    // load last saved settings
    loadLayout();
    loadColor();

  }// MainWindow

MainWindow::~MainWindow()
{
    delete ui;
}//~MainWindow deconstructor

// CUSTOM FUNCTIONS ///

// ********* CALCULATE *********************
void MainWindow::calculate()
{    
    // *********** VERY IMPORTANT ****************
    // if label_Yields other than cero remove scene
    // this is for memory management purposes
    if(ui->label_Yields->text()!= "0"){
        scene->setParent(NULL);
        scene->deleteLater();
    }// end if, no else, I tried adding the else and I got an ERROR

    //getting inputs as text
    QString partW = ui->lineEdit_PartWidth->text();
    QString partL = ui->lineEdit_PartLength->text();
    QString sheetW = ui->doubleSpinBox_SheetWidth->text();
    QString sheetL = ui->doubleSpinBox_SheetLength->text();
    QString clamp = ui->doubleSpinBox_Clamp->text();
    QString kerf = ui->doubleSpinBox_Kerf->text();
    QString web = ui->doubleSpinBox_Web->text();

     //converting input text as numbers
    partWidthPlain = partW.toFloat();// this is used to draw rectangles wihtout the kerf and to do conparisons
    partLengthPlain = partL.toFloat();// this is used to draw rectangles wihtout the kerf and to do conparisons
    kerfSize = kerf.toFloat();// convert kerf to a number
    webSize = web.toFloat();
    sheetLengthPlain = sheetL.toFloat();
    sheetWidth = (sheetW.toFloat() - clamp.toFloat()) - .5;


    // VALIDATING FIELDS BEFORE ANY CALCULATION

    // if fields are empty show the fillowing message
    if ((partW == "" || partL == "")||(sheetW == "" || sheetL == ""))
        {
             QMessageBox::information(this, "Warning", "One of the input fields is empty");
             ui->label_TotalParts->setText("0");
             ui->label_Yields->setText("0");
             disableButtons();
        }

    // if PART dimension fields are less than .100" get the following message
    // I had to convert to numbers before validation
    else if(partWidthPlain <= .1 || partLengthPlain <= .1)
    {
        QMessageBox::information(this, "Warning", "Please enter a value greater than .100 inches in the part dimension fields");
         ui->label_TotalParts->setText("0");
         ui->label_Yields->setText("0");
         disableButtons();
    }

    // part dimensions are bigger than sheet dimensions show the following message
    // note that part dimensions include kerf and sheet includes clamp, web etc
    else if(((partWidthPlain + kerfSize) > (sheetWidth) ) || ((partLengthPlain + kerfSize) > (sheetLengthPlain - (webSize* 2))))
    {
        QMessageBox::information(this, "Warning", "Part it's bigger than sheet size");
         ui->label_TotalParts->setText("0");
         ui->label_Yields->setText("0");
         disableButtons();
    }

    // if width is bigger than length swap numbers
    else if(partWidthPlain > partLengthPlain)
    {
              QString text = ui->lineEdit_PartLength->text();
              QString text2 = ui->lineEdit_PartWidth->text();
               ui->lineEdit_PartWidth->setText(text);
               ui->lineEdit_PartLength->setText(text2);
               disableButtons();
               calculate();
    }

    // if all fields contain numbers calculate it
    else
    {

        //activating buttons
        ui->pushButton_copyYields->setDisabled(false);
        ui->pushButton_Reset->setDisabled(false);

        // set button color
        ui->pushButton_Reset->setStyleSheet("color: red");
        ui->pushButton_copyYields->setStyleSheet("color:green");

        // set yield label to original state
        ui->label_Yields->setStyleSheet("QLabel {color:#000;  font-size:22px; font-weight:none;}");


    partWidth = partWidthPlain + kerfSize;
    partLength = partLengthPlain + kerfSize;
    sheetLength = sheetLengthPlain - (webSize* 2);
    // -------------  CALCULATIONS ---------------------

    //-----------------------------------------------------
    //--------- CALCULATION FOR GRID X ---------
    //-----------------------------------------------------
    partsXgridX = sheetLength /partLength;
    partsYgridX = sheetWidth / partWidth;    
    totalPartsGridX = partsXgridX * partsYgridX;
    usedAreaGridX = partsXgridX * partLength;
    extraSpaceGridX = sheetLength - usedAreaGridX;

    if (extraSpaceGridX >= partWidth)    {
        //local variables to hold extra parts
        extraPartsXH = extraSpaceGridX / partWidth;
        extraPartsYH = sheetWidth / partLength;

        extraPartsGridX = extraPartsXH * extraPartsYH ;
        totalPartsGridX = totalPartsGridX + extraPartsGridX; // total parts

        ui->label_TotalParts->setText(QString::number(totalPartsGridX));
    }
    else
    {
        ui->label_TotalParts->setText(QString::number(totalPartsGridX));

    }

    //-----------------------------------------------------
    //--------- CALCULATIONS FOR GRID Y -------
    //-----------------------------------------------------

    partsYgridY = sheetWidth / partLength;
    partsXgridY = sheetLength / partWidth ;
    totalPartsGridY = partsYgridY * partsXgridY;
    usedAreaGridY = partsYgridY * partLength;
    extraSpaceGridY = sheetWidth - usedAreaGridY;

    if (extraSpaceGridY >= partWidth)
    {
        //local variables to hold extra parts
        extraPartsYV = extraSpaceGridY / partWidth;
        extraPartsXV = sheetLength / partLength;

        extraPartsGridY = extraPartsXV * extraPartsYV ;
        totalPartsGridY = totalPartsGridY + extraPartsGridY; // total parts
     }


    //DISPLAY THE GREATES QTY
    if(totalPartsGridX >= totalPartsGridY)
    {
        float yieldsX = 1 / totalPartsGridX;
        ui->label_TotalParts->setText(QString::number(totalPartsGridX));
        // here I'm using the 'f' to specify the format type, otherwise I would get a number like 0.0000e-00
        ui->label_Yields->setText(QString::number(yieldsX,'f',8));
         drawPartsX();// draw parts only after checking if fields contain numbers
         // status message
         ui->statusBar->showMessage(" Calculating yields...", 500);
    }
    else
    {
        float yieldsY = 1 / totalPartsGridY;
        ui->label_TotalParts->setText(QString::number(totalPartsGridY));
        // here I'm using the 'f' to specify the format type, otherwise I would get a number like 0.0000e-00
        ui->label_Yields->setText(QString::number(yieldsY,'f',8));
        drawPartsY();
        // status message
        ui->statusBar->showMessage("  Calculating yields...", 500);
    }


    }// end main else

}// end calculate

//-----------------------------------------------------
//--------- DRAW PARTS ON SCREEN    -------
//-----------------------------------------------------

void MainWindow::drawPartsX()
{
    scene = new QGraphicsScene(webSize ,webSize ,360,180);

    ui->graphicsView_Sheet->setScene(scene);
    // reminder resize QgraphicsScene at the same time
    //ui->graphicsView_Sheet->resize((sheetLength +10), (sheetWidth +10)); // RESIZE SAMPLE

    for (int i=0; i< partsXgridX; i++)
    {
        for(int j=0; j<partsYgridX; j++)
        {
             QBrush partBrush(partColor);
             QPen partPen(Qt::darkGray);
              partPen.setWidth(0);
              rectangle = scene->addRect(((partLength)*i), ((partWidth)*j), partLengthPlain, partWidthPlain, partPen, partBrush);
              rectangle->setScale(3.1);
        }
    }// end first for loop

    if(extraSpaceGridX >= partWidth) {

    for (int i=0; i< extraPartsXH; i++)
    {
        for(int j=0; j<extraPartsYH; j++)
        {

             QBrush partBrush(partColor);
             QPen partPen(Qt::darkGray);
              partPen.setWidth(0);
              rectangle = scene->addRect((((partWidth)*i)+usedAreaGridX), ((partLength)*j), partWidthPlain, partLengthPlain, partPen, partBrush);
              rectangle->setScale(3.1);
        }
    }// end second for loop
} // end IF
}// end drawPartsX

// ****************** Y GRID ********
void MainWindow::drawPartsY()
{
    scene = new QGraphicsScene(webSize ,webSize ,360,180);

    ui->graphicsView_Sheet->setScene(scene);
    // reminder resize QgraphicsScene at the same time
    //ui->graphicsView_Sheet->resize((sheetLength +10), (sheetWidth +10)); // RESIZE SAMPLE

    for (int i=0; i< partsXgridY; i++)
    {
        for(int j=0; j<partsYgridY; j++)
        {
             QBrush partBrush(partColor);
             QPen partPen(Qt::darkGray);
              partPen.setWidth(0);
              rectangle = scene->addRect(((partWidth)*i), ((partLength)*j), partWidthPlain, partLengthPlain, partPen, partBrush);
              rectangle->setScale(3.1);
        }
    }// end first for loop

    if(extraSpaceGridY >= partWidth) {

    for (int i=0; i< extraPartsXV; i++)
    {
        for(int j=0; j<extraPartsYV; j++)
        {
             QBrush partBrush(partColor);
             QPen partPen(Qt::darkGray);
              partPen.setWidth(0);
              rectangle = scene->addRect(((partLength)*i), (((partWidth)*j) + usedAreaGridY ), partLengthPlain, partWidthPlain, partPen, partBrush);
              rectangle->setScale(3.1);
        }
    }// end second for loop
} // end IF




}// end drawPartsY

void MainWindow::disableButtons()
{
    //disable buttons to prevent from resetting twice
    ui->pushButton_Reset->setDisabled(true);
    ui->pushButton_copyYields->setDisabled(true);
    //change color
    ui->pushButton_Reset->setStyleSheet("color: gray");
    ui->pushButton_copyYields->setStyleSheet("color:gray");
}


void MainWindow::disableOptions()
{
    ui->doubleSpinBox_Clamp->setDisabled(true);
    ui->doubleSpinBox_Kerf->setDisabled(true);
    ui->doubleSpinBox_Web->setDisabled(true);
    ui->label_Clamp->setDisabled(true);
    ui->label_Kerf->setDisabled(true);
    ui->label_Web->setDisabled(true);
}

void MainWindow::enableOptions()
{
    ui->doubleSpinBox_Clamp->setEnabled(true);
    ui->doubleSpinBox_Kerf->setEnabled(true);
    ui->doubleSpinBox_Web->setEnabled(true);
    ui->label_Clamp->setEnabled(true);
    ui->label_Kerf->setEnabled(true);
    ui->label_Web->setEnabled(true);
}


void MainWindow::reset(){
    qDebug()<<"resetting fields";

    ui->radioButton_Turret->click();// clicking turret radio button
    ui->statusBar->showMessage("  Resetting fields...", 400);
    //resetting fields
    ui->label_TotalParts->setText("0");
    ui->label_Yields->setText("0");
    ui->lineEdit_PartLength->setText("");
    ui->lineEdit_PartWidth->setText("");
    ui->doubleSpinBox_SheetLength->setValue(120);
    ui->doubleSpinBox_SheetWidth->setValue(60);
    ui->label_Yields->setStyleSheet("QLabel {color:#666;  font-size:22px; font-weight:none;}");
    disableButtons();
    disableOptions();

    //partColor = QColor(85, 170, 0, 110);

     // deleting scene from memory and from screen
    if(scene->isActive()){
        scene->setParent(NULL);
        scene->deleteLater();
    }

}

// FUNCTION FOR CALCULATE BUTTON
void MainWindow::on_pushButton_Calculate_clicked()
{
    calculate();
}//  end - on_pushButton_Calculate_clicked()

// RADIO BUTTON FUNTIONS
void MainWindow::on_radioButton_Turret_clicked()
{
    if(ui->radioButton_Turret->isChecked())
    {
        ui->label_InfoScreen->setText("Turrets");
        ui->doubleSpinBox_Clamp->setValue(3);
        ui->doubleSpinBox_Kerf->setValue(.250);
        ui->doubleSpinBox_Web->setValue(.750);
        calculate();

    }

}

void MainWindow::on_radioButton_Bystronic_clicked()
{
    if(ui->radioButton_Bystronic->isChecked())    {
        ui->label_InfoScreen->setText("Bystronics");
        ui->doubleSpinBox_Clamp->setValue(.5);
        ui->doubleSpinBox_Kerf->setValue(.125);
        ui->doubleSpinBox_Web->setValue(.750);
        calculate();
    }

}

void MainWindow::on_radioButton_Mazak_clicked()
{
    if(ui->radioButton_Mazak->isChecked())
    {
        ui->label_InfoScreen->setText("Mazak");
        ui->doubleSpinBox_Clamp->setValue(1.5);
        ui->doubleSpinBox_Kerf->setValue(.125);
        ui->doubleSpinBox_Web->setValue(.750);
        calculate();       
    }
}

void MainWindow::on_radioButton_Saw_clicked()
{
    if(ui->radioButton_Saw->isChecked())
    {
        ui->label_InfoScreen->setText("Saw");
        ui->doubleSpinBox_Clamp->setValue(.5);
        ui->doubleSpinBox_Kerf->setValue(.156);
        ui->doubleSpinBox_Web->setValue(.750);
        calculate();
    }
}
// RADIO BUTTONS END HERE

//reset button
void MainWindow::on_pushButton_Reset_clicked()
{
    reset();
}
// display material list dialog
void MainWindow::on_actionMaterials_List_triggered()
{
        MaterialsListDialog mDialog;
        mDialog.setModal(true);
        mDialog.exec();
}

void MainWindow::on_pushButton_copyYields_clicked()
{
    // check if yields are set to cero, show message
    // this may not be needed since buttons is desable most of the time
    if(ui->label_Yields->text() == "0")
    {
        QMessageBox::information(this, "Warning","There is nothing to copy");
    }
    // if yields are set to other than 0 do the following
    else
    {
        ui->statusBar->showMessage("  Copying yields...", 400);
    //copy text from labelYields to the clipbord
    QClipboard *clip = QApplication::clipboard();
    QString input = ui->label_Yields->text();
    clip->setText(input);

    // format text
   ui->label_Yields->setStyleSheet("QLabel {color:green;  font-size:25px; font-weight:bold;}");
   ui->label_Message->setStyleSheet("QLabel {color:orange;}");

    //copy yields to the message label
   QString message = ui->label_Yields->text();
   ui->label_Message->setText(message);
    }// end if
}

void MainWindow::on_pushButton_ColorPick_clicked()
{
    partColor = QColorDialog::getColor(QColor(partColor), this);
}
void MainWindow::saveLayout()
{
    ui->statusBar->showMessage("  Saving layout...", 400);
    QSettings mySettings("tuDesign", "Tool");
    mySettings.beginGroup("MainWindow");
    mySettings.setValue("geometry", saveGeometry());
    mySettings.endGroup();
    QMessageBox::information(this, "Save Layout", "Layout has been saved.");
}

void MainWindow::loadLayout()
{
    QSettings mySettings("tuDesign", "Tool");
    mySettings.beginGroup("MainWindow");
    restoreGeometry(mySettings.value("geometry").toByteArray());
    mySettings.endGroup();
}

void MainWindow::saveColor()
{
    ui->statusBar->showMessage("  Saving color...", 400);
    QSettings mySettings("tuDesign", "Tool");
    mySettings.beginGroup("MainWindow");
    mySettings.setValue("MyColor", getColor().rgba());
    mySettings.endGroup();
    QMessageBox::information(this, "Save Color", "The selected color has been saved");
}

void MainWindow::loadColor()
{
    QSettings mySettings("tuDesign", "Tool");
    mySettings.beginGroup("MainWindow");
    if(mySettings.contains("MyColor"))
    {
        partColor.setRgba(mySettings.value("MyColor").toUInt());
        setColor(partColor);
    }
    else
    {
       //default color
       //partColor = QColor(85, 170, 0, 110);
       partColor = QColor(116, 155, 189, 255);
    }

    mySettings.endGroup();
}

void MainWindow::resetColor()
{
    ui->statusBar->showMessage("  Resetting color...", 400);
    QSettings mySettings("tuDesign", "Tool");
    mySettings.beginGroup("MainWindow");
    //default color
    QColor resetColor = QColor(116, 155, 189, 255);
    setColor(resetColor);// set color
    mySettings.setValue("MyColor", getColor().rgba());//save color

    QMessageBox::information(this, "Color Reset", "Part color has been reset to the default color");
    mySettings.endGroup();

}

void MainWindow::on_actionSave_Color_triggered()
{
    saveColor();
}

void MainWindow::on_actionReset_Color_triggered()
{
    resetColor();
}


void MainWindow::setColor(const QColor &myCurrentColor)
{
  partColor = myCurrentColor;
}

const QColor& MainWindow::getColor()
{
  return partColor;
}

void MainWindow::on_actionSave_Layout_triggered()
{
    saveLayout();
}

void MainWindow::on_actionAbout_Tool_triggered()
{
    QMessageBox::about(this, tr("About Tool"),
                       tr("<img src=':/Icons/images/toolLogo-269x128.png'/>"
                           "<h2>Tool</h2>"
                          "<p>Version 2.0 </p>"
    "<p>Copyright &copy; 2012 tuDesign. All Rights Reserved.</p>"
    "<p>Tool was exclusively designed for the engineers at Kenall Manufacturing!</p>"

      ));
}

void MainWindow::on_pushButton_EditOptions_clicked()
{
    enableOptions();
}

void MainWindow::on_pushButton_Clone_clicked()
{
    if(ui->lineEdit_PartLength->text() == "")
    {
    QString text2 = ui->lineEdit_PartWidth->text();
    ui->lineEdit_PartLength->setText(text2);
    disableButtons();
    calculate();
    }

    else if (ui->lineEdit_PartWidth->text() == "")
    {
    QString text = ui->lineEdit_PartLength->text();
    ui->lineEdit_PartWidth->setText(text);
    disableButtons();
    calculate();
    }
    else
    {
     QString text2 = ui->lineEdit_PartWidth->text();
     ui->lineEdit_PartLength->setText(text2);
     disableButtons();
     calculate();
    }
}// CLONE ENDS HERE

/**********************************************************
||************************  FINDR **************************
***********************************************************/

void MainWindow::on_pushButton_Drawing_clicked()
{
    QString fileNumber;
    fileNumber = ui->lineEdit_FindPart->text().trimmed();

    if(fileNumber == "0")
    {
        QMessageBox::information(this, "Enter a valid number", "Enter a number other than 0");
    }
    else if(fileNumber == "")
    {
        QMessageBox::information(this, "Empty", "Enter a number");
    }
    else if(fileNumber.contains("A")|| fileNumber.contains("a"))
    {
        findDrawing("A---/");
    }//IF A ends here

    // B doesnt use custom funtions
    else if(fileNumber.contains("B")|| fileNumber.contains("b"))
    {
        //REMOVE LETERS
        if(fileNumber.contains("B"))
        {
            fileNumber.remove("B");
        }
        else if(fileNumber.contains("b"))
        {
            fileNumber.remove("b");
        }

        // CONVERT TO INT BEFORE CHECKING VALUE
        int partNumber = fileNumber.toInt();

        if (partNumber <= 10000)// if part number is between 1 and 10000 use the static directory AND ADD DASH (-)
        {
            searchingFiles();
            //static directory
            QString directoryRange = "B-0001-B10000";
            // assigned to a variable to be able to check the status
            bool linkToDrawing = QDesktopServices::openUrl(QUrl("file:///N:/SolidWorksDrawing/B---/" + directoryRange + "/B" + fileNumber + ".slddrw") );

            if(linkToDrawing !=true)
            {
                notFound();
                QMessageBox::information(this, "Not Found", "The drawing you are looking for was not found in this directory");
            }
            else
            {
                foundIt();
            }
        }

        else
        {
            int directoryMin = (partNumber /1000)* (1000) + 1;// rounding number
            int directoryMax = directoryMin + 999;
            searchingFiles();
            //converting ints to strings and concatinating them
            QString directoryRange = "B"+ QString::number(directoryMin)+ "-" + "B" + QString::number(directoryMax);
            bool linkToDrawing = QDesktopServices::openUrl(QUrl("file:///N:/SolidWorksDrawing/B---/" + directoryRange + "/b" + fileNumber + ".slddrw") );

            if(linkToDrawing !=true)
            {
                notFound();
                QMessageBox::information(this, "Not Found", "The drawing you are looking for was not found in this directory");
            }
            else
            {
                foundIt();
            }

        }
    }//IF B ends here
    else if(fileNumber.contains("C")|| fileNumber.contains("c"))
    {
        findDrawing("C---/");
    }//IF C
    else if(fileNumber.contains("D")|| fileNumber.contains("d"))
    {
        findDrawing("D---/");
    }//IF D ends here
    else if(fileNumber.contains("E")|| fileNumber.contains("e"))
    {
        findDrawing("E---/");
    }//IF E ends here
    else if(fileNumber.contains("F")|| fileNumber.contains("f"))
    {
        findDrawing("F---/");
    }//IF F ends here
    else if(fileNumber.contains("G")|| fileNumber.contains("g"))
    {
        findDrawing("G/");// no dashes
    }//IF G ends here
    else if(fileNumber.contains("L")|| fileNumber.contains("l"))
    {
        findDrawing("L---/");
    }//IF L ends here
    else if(fileNumber.contains("N")|| fileNumber.contains("n"))
    {
        findDrawing("N---/");
    }//IF N ends here
    else if(fileNumber.contains("R")|| fileNumber.contains("r"))
    {
        findDrawing("R---/");
    }//IF R ends here
    else if(fileNumber.contains("X")|| fileNumber.contains("x"))
    {
        findDrawing("X---/");
    }//IF X ends here

    else if(fileNumber.contains("750"))
    {
        findDrawing("75XXXX/");
    }//IF 750XXX ends here
    else
    {
        notFound();
         QMessageBox::information(this, "Not Found", "You can only search for A, B, C, D, E, F, G, L, N, R, X or 750xxx numbers!");
    }

}//DRAWING ENDS HERE

void MainWindow::on_pushButton_Pdf_clicked()
{
    QString fileNumber;
    fileNumber = ui->lineEdit_FindPart->text().trimmed();

    if(fileNumber == "" )
    {

        QMessageBox::information(this, "Empty", "Enter a number");
    }

    else if(fileNumber == "0" )
    {

        QMessageBox::information(this, "Enter a valid number", "Enter a number other than 0");
    }

    else if(fileNumber.contains("A")|| fileNumber.contains("a"))
    {
        findPdf("A---/");
    }//IF A ends here

    else if(fileNumber.contains("B")|| fileNumber.contains("b"))
    {
        findPdf("B---/");
    }//IF B ends here


    else if(fileNumber.contains("C")|| fileNumber.contains("c"))
    {
        findPdf("C---/");
    }//IF C ends here

    else if(fileNumber.contains("D")|| fileNumber.contains("d"))
    {
       findPdf("D---/");
    }//IF D ends here

    else if(fileNumber.contains("E")|| fileNumber.contains("e"))
    {
        findPdf("E---/");
    }//IF E ends here

    else if(fileNumber.contains("F")|| fileNumber.contains("f"))
    {
        findPdf("F---/");
    }//IF F ends here

    else if(fileNumber.contains("G")|| fileNumber.contains("g"))
    {
        findPdf("G/");
    }//IF G ends here

    else if(fileNumber.contains("L")|| fileNumber.contains("l"))
    {
        findPdf("L---/");
    }//IF L ends here

    else if(fileNumber.contains("N")|| fileNumber.contains("n"))
    {
        findPdf("N---/");
    }//IF N ends here

    else if(fileNumber.contains("R")|| fileNumber.contains("r"))
    {
        findPdf("R---/");
    }//IF R ends here

    else if(fileNumber.contains("X")|| fileNumber.contains("x"))
    {
        findPdf("X---/");
    }//IF X ends here

    else if(fileNumber.contains("750")|| fileNumber.contains("x"))
    {
        findPdf("75XXXX/");
    }//IF 750xxx ends here
    else
    {
        notFound();
        QMessageBox::information(this, "Not Found", "You can only search for A, B, C, D, E, F, G, L, N, R, X or 750xxx numbers!");
    }


}//PDF ENDS HERE

//when looking for dxf there are no restrictions on file numbers
void MainWindow::on_pushButton_Dxf_clicked()
{
    QString fileNumber;
    fileNumber = ui->lineEdit_FindPart->text().trimmed();

    if(fileNumber == "" )
    {
        QMessageBox::information(this, "Empty", "Enter a number");
    }
    else if(fileNumber == "0" )
    {
        QMessageBox::information(this, "Enter a valid number", "Enter a number other than 0");
    }

    else
    {
        searchingFiles();
        // assigned to a variable to be able to check the status
         bool linkToDxf = QDesktopServices::openUrl(QUrl("file:///P:/SW_Track/" + fileNumber + ".dxf") );

         if(linkToDxf !=true)
         {
             notFound();
             QMessageBox::information(this, "Not Found", "The DXF you are looking for was not found in this directory");
         }
         else
         {
             foundIt();
         }
    }
}//DXF ENDS HERE

void MainWindow::on_pushButton_Check_clicked()
{
    fileNumber = ui->lineEdit_FindPart->text().trimmed();

    // SHOW PART NUMBER
    ui->label_LastChecked->setText(fileNumber);

    if(fileNumber == "0")
    {
        QMessageBox::information(this, "Enter a valid number", "Enter a number other than 0");
    }
    else if(fileNumber == "")
    {
        QMessageBox::information(this, "Empty", "Enter a number");
    }
    else if(fileNumber.contains("A")|| fileNumber.contains("a"))
    {
        doesExists("A---/");
    }// IF A ends here
    // B doesnt use custom funtions
    else if(fileNumber.contains("B")|| fileNumber.contains("b"))
    {
        if(fileNumber.contains("B"))
        {
            fileNumber.remove("B");
        }
        else if(fileNumber.contains("b"))
        {
            fileNumber.remove("b");
        }

        int partNumber = fileNumber.toInt();

     if (partNumber <= 10000)// if part number is between 1 and 10000 use the static directory
            {
                QFile drawing("N:/SolidWorksDrawing/B---/B-0001-B10000/B" + fileNumber + ".slddrw"); // added B since it was removed
                QFile pdf("P:/S_Works-Dwgs/B---/B" + fileNumber + ".pdf");// added B since it was removed
                QFile dxf("P:/SW_Track/B" + fileNumber + ".dxf");// added B since it was removed

                if(drawing.exists())
                {
                   ui->graphicsView_Drawing->setStyleSheet("background-color:#00ff00");
                }
                else
                {
                  ui->graphicsView_Drawing->setStyleSheet("background-color:#ff0000");
                }

                // PDF
                if(pdf.exists())
                {
                   ui->graphicsView_Pdf->setStyleSheet("background-color:#00ff00");
                }
                else
                {
                  ui->graphicsView_Pdf->setStyleSheet("background-color:#ff0000");
                }

                // DXF
                if(dxf.exists())
                {
                   ui->graphicsView_Dxf->setStyleSheet("background-color:#00ff00");
                }
                else
                {
                  ui->graphicsView_Dxf->setStyleSheet("background-color:#ff0000");
                }
            }

            else
            {
                int partNumber = fileNumber.toInt();
                int directoryMin = (partNumber /1000)* (1000) + 1;// rounding number
                int directoryMax = directoryMin + 999;

                QString directoryRange = "B" + QString::number(directoryMin)+ "-" +  "B" + QString::number(directoryMax);
                QFile drawing("N:/SolidWorksDrawing/B---/" + directoryRange + "/B" + fileNumber + ".slddrw");
                QFile pdf("P:/S_Works-Dwgs/B---/B" + fileNumber + ".pdf");
                QFile dxf("P:/SW_Track/B" + fileNumber + ".dxf");

                if(drawing.exists())
                {
                   ui->graphicsView_Drawing->setStyleSheet("background-color:#00ff00");
                }
                else
                {
                  ui->graphicsView_Drawing->setStyleSheet("background-color:#ff0000");
                }

                // PDF
                if(pdf.exists())
                {
                   ui->graphicsView_Pdf->setStyleSheet("background-color:#00ff00");
                }
                else
                {
                  ui->graphicsView_Pdf->setStyleSheet("background-color:#ff0000");
                }

                // DXF
                if(dxf.exists())
                {
                   ui->graphicsView_Dxf->setStyleSheet("background-color:#00ff00");
                }
                else
                {
                  ui->graphicsView_Dxf->setStyleSheet("background-color:#ff0000");
                }

            }

    }//IF B ends here
    else if(fileNumber.contains("C")|| fileNumber.contains("c"))
    {
        doesExists("C---/");
    }//IF C ends here
    else if(fileNumber.contains("D")|| fileNumber.contains("d"))
    {
        doesExists("D---/");
    }//IF D ends here
    else if(fileNumber.contains("E")|| fileNumber.contains("e"))
    {
         doesExists("E---/");
    }//IF E ends here
    else if(fileNumber.contains("F")|| fileNumber.contains("f"))
    {
         doesExists("F---/");
    }//IF F ends here
    else if(fileNumber.contains("G")|| fileNumber.contains("g"))
    {
        doesExists("G/");
    }//IF G ends here
    else if(fileNumber.contains("L")|| fileNumber.contains("l"))
    {
         doesExists("L---/");
    }//IF L ends here
    else if(fileNumber.contains("N")|| fileNumber.contains("n"))
    {
         doesExists("N---/");
    }//IF N ends here
    else if(fileNumber.contains("R")|| fileNumber.contains("r"))
    {
         doesExists("R---/");
    }//IF R ends here
    else if(fileNumber.contains("X")|| fileNumber.contains("x"))
    {
         doesExists("X---/");
    }//IF X ends here
    else if(fileNumber.contains("750")|| fileNumber.contains("x"))
    {
         doesExists("75XXXX/");
    }//IF 750XXX ends here
    else
    {
        notFound();
        QMessageBox::information(this, "Not Found", "You can only search for A, B, C, D, E, F, G, L, N, R, X or 750xxx numbers!");
    }

}//CHECK ENDS HERE

void MainWindow::foundIt()
{
    QString partNum = "             " + ui->lineEdit_FindPart->text() + ", was successfully found";
    ui->statusBar->showMessage(partNum,10000);
}
void MainWindow::notFound()
{
    QString partNum = "             " + ui->lineEdit_FindPart->text() + ", was not found";
    ui->statusBar->showMessage(partNum,5000);
}
void MainWindow::searchingFiles()
{
    QString partNum = "            Searching for " + ui->lineEdit_FindPart->text()+ "...";
     ui->statusBar->showMessage(partNum);
}

// CUSTOM FUNCTION, this function is used in the check funtion
void MainWindow::doesExists(QString sufix)
{
    QString folder = sufix;
    QFile drawing("N:/SolidWorksDrawing/"+folder + fileNumber + ".slddrw");
    QFile pdf("P:/S_Works-Dwgs/"+folder + fileNumber + ".pdf");
    QFile dxf("P:/SW_Track/" + fileNumber + ".dxf");


    if(drawing.exists())
    {
       ui->graphicsView_Drawing->setStyleSheet("background-color:#00ff00");
    }
    else
    {
      ui->graphicsView_Drawing->setStyleSheet("background-color:#ff0000");
    }

    // PDF
    if(pdf.exists())
    {
       ui->graphicsView_Pdf->setStyleSheet("background-color:#00ff00");
    }
    else
    {
      ui->graphicsView_Pdf->setStyleSheet("background-color:#ff0000");
    }

    // DXF
    if(dxf.exists())
    {
       ui->graphicsView_Dxf->setStyleSheet("background-color:#00ff00");
    }
    else
    {
      ui->graphicsView_Dxf->setStyleSheet("background-color:#ff0000");
    }
}
// CUSTOM FUNCTION, this function is used in the dawing function
void MainWindow::findDrawing(QString partDirectory)
{
   QString partDir = partDirectory;

   // without assigning this here it doesnt capture the part number entered
   fileNumber = ui->lineEdit_FindPart->text().trimmed();
   searchingFiles();
   //converting ints to strings and concatinating them
    bool linkToDrawing = QDesktopServices::openUrl(QUrl("file:///N:/SolidWorksDrawing/" + partDir + fileNumber + ".slddrw") );

   if(linkToDrawing !=true)
   {
       notFound();
       QMessageBox::information(this, "Not Found", "The drawing you are looking for was not found in this directory");
   }
   else
   {
       foundIt();
   }

}
void MainWindow::findPdf(QString partDirectoryDxf)
{
    QString partDirDxf   = partDirectoryDxf;

    // without assigning this here it doesnt capture the part number entered
    fileNumber = ui->lineEdit_FindPart->text().trimmed();

    searchingFiles();
    // assigned to a variable to be able to check the status
    bool linkToPdf = QDesktopServices::openUrl(QUrl("file:///P:/S_Works-Dwgs/"+ partDirDxf + fileNumber + ".pdf") );

    if(linkToPdf!= true)
    {
        notFound();
        QMessageBox::information(this, "Not Found", "The PDF you are looking for was not found in this directory");
    }
    else
    {
        foundIt();
    }
} // find pd
