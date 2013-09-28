/*
    This file is a part of the AID (Another Image Debugger) project.

    Copyright (C) 2013  Olinski Krzysztof E.

    This program is free software: you can redistribute it 
    and/or modify it under the terms of the GNU General Public License 
    as published by the Free Software Foundation, either version 3 of the License, 
    or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
    or FITNESS FOR A PARTICULAR PURPOSE. 
    See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with this program.  
    If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef QWAUXDIALOGS_H
#define QWAUXDIALOGS_H

#include "CImgContext.h"
#include "Threads.h"
#include "globals.h"


#include <QDoubleValidator>

#include <QtGlobal>

#include <QSharedPointer>
#include <QUrl>
#include <QTextBrowser>
#include <QCheckBox>
#include <QMutex>

#ifdef QT4_HEADERS
    #include <QWidgetAction>
    #include <QButtonGroup>
    #include <QRadioButton>
    #include <QLineEdit>
    #include <QSlider>
    #include <QTextEdit>
    #include <QPushButton>
    #include <QVBoxLayout>
    #include <QHBoxLayout>
    #include <QGridLayout>
    #include <QMenu>
    #include <QLabel>
    #include <QMessageBox>
    #include <QComboBox>
#elif QT5_HEADERS
    #include <QtWidgets/QWidgetAction>
    #include <QtWidgets/QButtonGroup>
    #include <QtWidgets/QRadioButton>
    #include <QtWidgets/QLineEdit>
    #include <QtWidgets/QSlider>
    #include <QtWidgets/QTextEdit>
    #include <QtWidgets/QPushButton>
    #include <QtWidgets/QVBoxLayout>
    #include <QtWidgets/QHBoxLayout>
    #include <QtWidgets/QGridLayout>
    #include <QtWidgets/QMenu>
    #include <QtWidgets/QLabel>
    #include <QtWidgets/QMessageBox>
    #include <QtWidgets/QComboBox>
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////
/*!
 * \brief The qwSWButton class.
 *        A wrapper for the QPushButton class.
 *        This calss extenses the QPushButton class by adding signal/slot necessary to provide GUIfunctionality.
 */

class qwSWButton : public QPushButton
{
   Q_OBJECT

public:
    qwSWButton(QIcon myIco, QWidget* _parent, quint32 _myID): myID(_myID), myParent(_parent)
    {
        Q_ASSERT(_parent);

        setIcon(myIco);
        setContentsMargins(0, 0, 0, 0);
        if(!(myID&0x100))
          setCheckable(true);
        setFlat(true);

        connect(this, SIGNAL(clickedIHaveBeen(quint32, qint32)),
                myParent, SLOT(reciveSWButtonEvent(quint32, qint32)));
    }

    void sendClickedIHaveBeen()
    {
        emit clickedIHaveBeen(myID, (qint32)isChecked());
    }

signals:
    void clickedIHaveBeen(quint32 myID, qint32 state);

protected:
  void  mousePressEvent(QMouseEvent *event)
  {
      if(menu()==NULL)
      {
          if(isCheckable())
              setChecked(!isChecked());
          else
              QPushButton::mousePressEvent(event);

          emit clickedIHaveBeen(myID, (qint32)isChecked());
      }
      else
      {
            QPushButton::mousePressEvent(event);
      }
  }

private:
    quint32  myID;
    QWidget* myParent;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/*!
 * \brief The qwMenu class.
 *        A drop-down menu for linear filter.
 */
class qwMenu : public QMenu
{
    Q_OBJECT

protected:
  void mousePressEvent(QMouseEvent*){}
  void mouseReleaseEvent(QMouseEvent*){}
  void keyPressEvent(QKeyEvent*){}
};

class qwDialogLT : public QWidget
{
   Q_OBJECT

public:

    qwDialogLT(qwSWButton* _myGrandParent, QWidget* parent = NULL) : QWidget(parent), myGrandParent(_myGrandParent)
    {
        QFont font = QFont( "Helvetica", 6 );

        myParent = (QMenu*)parent;

        headLabel.setText("Cy = mul*Cx/div + bias\n"
                          "Note: This is only a render data\n"
                          "clamped value channel scaling. For a source\n"
                          "data manipulation use 'Recast source data' tool.");
        headLabel.setFont(font);
        headLabel.setWordWrap(true);

        mulLabel.setText("mul ");
        divLabel.setText("div ");
        biasLabel.setText("bias ");

        slidersLayout = new QGridLayout();

        mulSlider = new QSlider(Qt::Horizontal, this);
        mulEdit = new QLineEdit(this);
        mulEdit->setMaximumWidth(50);
        mulEdit->setInputMask("999");
        mulEdit->setValidator(new QIntValidator(0,255,0));
        mulSlider->setMaximum(255);
        mulSlider->setMinimum(0);
        mulSlider->setValue(1);
        mulSlider->setTickInterval(1);
        slidersLayout->addWidget(&mulLabel, 0, 0);
        slidersLayout->addWidget(mulSlider, 0, 1);
        slidersLayout->addWidget(mulEdit, 0, 2);

        divSlider = new QSlider(Qt::Horizontal, this);
        divEdit = new QLineEdit(this);
        divEdit->setMaximumWidth(50);
        divEdit->setInputMask("999");
        divEdit->setValidator(new QIntValidator(1,255,0));
        divSlider->setMaximum(255);
        divSlider->setMinimum(1);
        divSlider->setValue(1);
        divSlider->setTickInterval(1);
        slidersLayout->addWidget(&divLabel, 1, 0);
        slidersLayout->addWidget(divSlider, 1, 1);
        slidersLayout->addWidget(divEdit, 1, 2);

        biasSlider = new QSlider(Qt::Horizontal, this);
        biasEdit = new QLineEdit(this);
        biasEdit->setMaximumWidth(50);
        biasEdit->setInputMask("999");
        biasEdit->setValidator(new QIntValidator(0,255,0));
        biasSlider->setMaximum(255);
        biasSlider->setMinimum(0);
        biasSlider->setValue(0);
        biasSlider->setTickInterval(1);
        slidersLayout->addWidget(&biasLabel, 2, 0);
        slidersLayout->addWidget(biasSlider, 2, 1);
        slidersLayout->addWidget(biasEdit, 2, 2);

        redRB.setText("Red");
        greenRB.setText("Green");
        blueRB.setText("Blue");
        rgbRB.setText("RGB");
        channelGroup.addButton(&redRB);
        channelGroup.addButton(&greenRB);
        channelGroup.addButton(&blueRB);
        channelGroup.addButton(&rgbRB);

        bottomLine = new QHBoxLayout();
        bottomLine->addWidget(&redRB);
        bottomLine->addWidget(&greenRB);
        bottomLine->addWidget(&blueRB);
        bottomLine->addWidget(&rgbRB);

        applyButton.setText("Apply");
        applyAndHideButton.setText("Apply && hide");
        cancelButton.setText("Cancel");

        bottomLine->addWidget(&applyButton);
        bottomLine->addWidget(&applyAndHideButton);
        bottomLine->addWidget(&cancelButton);

        myLayout = new QVBoxLayout();
        myLayout->addWidget(&headLabel);
        myLayout->addLayout(slidersLayout);
        myLayout->addLayout(bottomLine);

        setLayout(myLayout);

        connect(&cancelButton, SIGNAL(clicked()), this, SLOT(clickedCancel()));
        connect(&applyButton, SIGNAL(clicked()), this, SLOT(clickedApply()));
        connect(&applyAndHideButton, SIGNAL(clicked()), this, SLOT(clickedApplyAndHide()));
        connect(this, SIGNAL(sendHideMenu()), myParent, SLOT(close()));

        connect(mulSlider, SIGNAL(valueChanged(int)), this, SLOT(mulSliderChanged(int)));
        connect(divSlider, SIGNAL(valueChanged(int)), this, SLOT(divSliderChanged(int)));
        connect(biasSlider, SIGNAL(valueChanged(int)), this, SLOT(biasSliderChanged(int)));

        connect(mulEdit, SIGNAL(editingFinished()), this, SLOT(mulEditChanged()));
        connect(divEdit, SIGNAL(editingFinished()), this, SLOT(divEditChanged()));
        connect(biasEdit, SIGNAL(editingFinished()), this, SLOT(biasEditChanged()));

        connect(&redRB, SIGNAL(clicked()), this, SLOT(rbButtonClicked()));
        connect(&greenRB, SIGNAL(clicked()), this, SLOT(rbButtonClicked()));
        connect(&blueRB, SIGNAL(clicked()), this, SLOT(rbButtonClicked()));
        connect(&rgbRB, SIGNAL(clicked()), this, SLOT(rbButtonClicked()));

        redParams[0] = greenParams[0] = blueParams[0] = rgbParams[0] = 1;
        redParams[1] = greenParams[1] = blueParams[1] = rgbParams[1] = 1;
        redParams[2] = greenParams[2] = blueParams[2] = rgbParams[1] = 0;
        redRB.setChecked(true);
    }

    void setRParams(quint8 mul, quint8 div, quint8 bias)
    {redParams[0] = mul; redParams[1] = div, redParams[2] = bias;
     rgbParams[0] = mul; rgbParams[1] = div, rgbParams[2] = bias;
     restoreValues();}
    void setGParams(quint8 mul, quint8 div, quint8 bias)
    {greenParams[0] = mul; greenParams[1] = div, greenParams[2] = bias;
     restoreValues();}
    void setBParams(quint8 mul, quint8 div, quint8 bias)
    {blueParams[0] = mul; blueParams[1] = div, blueParams[2] = bias;
     restoreValues();}

    void getRParams(quint8 &mul, quint8 &div, quint8 &bias)
    {
        if(rgbRB.isChecked())
        {mul = rgbParams[0]; div = rgbParams[1]; bias = rgbParams[2];}
        else
        {mul = redParams[0]; div = redParams[1]; bias = redParams[2];}
    }
    void getGParams(quint8 &mul, quint8 &div, quint8 &bias)
    {
        if(rgbRB.isChecked())
        {mul = rgbParams[0]; div = rgbParams[1]; bias = rgbParams[2];}
        else
        {mul = greenParams[0]; div = greenParams[1]; bias = greenParams[2];}
    }
    void getBParams(quint8 &mul, quint8 &div, quint8 &bias)
    {
        if(rgbRB.isChecked())
        {mul = rgbParams[0]; div = rgbParams[1]; bias = rgbParams[2];}
        else
        {mul = blueParams[0]; div = blueParams[1]; bias = blueParams[2];}
    }


signals:

    void sendHideMenu();

public slots:
    void closeMe()
    {
        emit sendHideMenu();
    }

private slots:
    void clickedCancel()
    {
        myGrandParent->setChecked(false);
        myGrandParent->sendClickedIHaveBeen();
        emit sendHideMenu();
    }

    void clickedApply()
    {
        myGrandParent->setChecked(true);
        myGrandParent->sendClickedIHaveBeen();

        Globals::sharedMul[0]  = (rgbRB.isChecked())?rgbParams[0]:redParams[0];
        Globals::sharedDiv[0]  = (rgbRB.isChecked())?rgbParams[1]:redParams[1];
        Globals::sharedBias[0] = (rgbRB.isChecked())?rgbParams[2]:redParams[2];

        Globals::sharedMul[1]  = (rgbRB.isChecked())?rgbParams[0]:greenParams[0];
        Globals::sharedDiv[1]  = (rgbRB.isChecked())?rgbParams[1]:greenParams[1];
        Globals::sharedBias[1] = (rgbRB.isChecked())?rgbParams[2]:greenParams[2];

        Globals::sharedMul[2]  = (rgbRB.isChecked())?rgbParams[0]:blueParams[0];
        Globals::sharedDiv[2]  = (rgbRB.isChecked())?rgbParams[1]:blueParams[1];
        Globals::sharedBias[2] = (rgbRB.isChecked())?rgbParams[2]:blueParams[2];
    }

    void clickedApplyAndHide()
    {
        myGrandParent->setChecked(true);
        myGrandParent->sendClickedIHaveBeen();

        Globals::sharedMul[0]  = (rgbRB.isChecked())?rgbParams[0]:redParams[0];
        Globals::sharedDiv[0]  = (rgbRB.isChecked())?rgbParams[1]:redParams[1];
        Globals::sharedBias[0] = (rgbRB.isChecked())?rgbParams[2]:redParams[2];

        Globals::sharedMul[1]  = (rgbRB.isChecked())?rgbParams[0]:greenParams[0];
        Globals::sharedDiv[1]  = (rgbRB.isChecked())?rgbParams[1]:greenParams[1];
        Globals::sharedBias[1] = (rgbRB.isChecked())?rgbParams[2]:greenParams[2];

        Globals::sharedMul[2]  = (rgbRB.isChecked())?rgbParams[0]:blueParams[0];
        Globals::sharedDiv[2]  = (rgbRB.isChecked())?rgbParams[1]:blueParams[1];
        Globals::sharedBias[2] = (rgbRB.isChecked())?rgbParams[2]:blueParams[2];
        emit sendHideMenu();
    }

    void rbButtonClicked()
    {
        restoreValues();
    }

    void mulSliderChanged(int value){mulEdit->setText(QString::number(value)); storeValues();}
    void divSliderChanged(int value){divEdit->setText(QString::number(value)); storeValues();}
    void biasSliderChanged(int value){biasEdit->setText(QString::number(value)); storeValues();}

    void mulEditChanged(){mulSlider->setSliderPosition(mulEdit->text().toInt()); storeValues();}
    void divEditChanged(){divSlider->setSliderPosition(divEdit->text().toInt()); storeValues();}
    void biasEditChanged(){biasSlider->setSliderPosition(biasEdit->text().toInt()); storeValues();}

protected:
    void showEvent (QShowEvent *event)
    {
      restoreValues();
      QWidget::showEvent(event);
    }

private:

    QPointer<QGridLayout> slidersLayout;
    QPointer<QHBoxLayout> bottomLine;
    QPointer<QVBoxLayout> myLayout;
    qwSWButton*           myGrandParent;
    QMenu*                myParent;

    QPointer<QSlider> mulSlider, divSlider, biasSlider;
    QPointer<QLineEdit> mulEdit, divEdit, biasEdit;

    QButtonGroup channelGroup;
    QRadioButton redRB, greenRB, blueRB, rgbRB;

    QPushButton applyButton, applyAndHideButton, cancelButton;
    QLabel headLabel, mulLabel, divLabel, biasLabel;

    quint8 redParams[3], greenParams[3], blueParams[3], rgbParams[3];

    void storeValues()
    {
        if(rgbRB.isChecked())
        {
            rgbParams[0] = mulSlider->value();
            rgbParams[1] = divSlider->value();
            rgbParams[2] = biasSlider->value();
        }
        else if(redRB.isChecked())
        {
            redParams[0] = mulSlider->value();
            redParams[1] = divSlider->value();
            redParams[2] = biasSlider->value();
        }
        else if(greenRB.isChecked())
        {
            greenParams[0] = mulSlider->value();
            greenParams[1] = divSlider->value();
            greenParams[2] = biasSlider->value();
        }
        else if(blueRB.isChecked())
        {
            blueParams[0] = mulSlider->value();
            blueParams[1] = divSlider->value();
            blueParams[2] = biasSlider->value();
        }
    }

    void restoreValues()
    {
        QString tmp;

        mulSlider->blockSignals(true);
        divSlider->blockSignals(true);
        biasSlider->blockSignals(true);

        if(redRB.isChecked())
        {
            mulSlider->setValue(redParams[0]);
            mulEdit->setText(tmp.setNum(redParams[0]));
            divSlider->setValue(redParams[1]);
            divEdit->setText(tmp.setNum(redParams[1]));
            biasSlider->setValue(redParams[2]);
            biasEdit->setText(tmp.setNum(redParams[2]));
        }
        else if(greenRB.isChecked())
        {
            mulSlider->setValue(greenParams[0]);
            mulEdit->setText(tmp.setNum(greenParams[0]));
            divSlider->setValue(greenParams[1]);
            divEdit->setText(tmp.setNum(greenParams[1]));
            biasSlider->setValue(greenParams[2]);
            biasEdit->setText(tmp.setNum(greenParams[2]));
        }
        else if(blueRB.isChecked())
        {
            mulSlider->setValue(blueParams[0]);
            mulEdit->setText(tmp.setNum(blueParams[0]));
            divSlider->setValue(blueParams[1]);
            divEdit->setText(tmp.setNum(blueParams[1]));
            biasSlider->setValue(blueParams[2]);
            biasEdit->setText(tmp.setNum(blueParams[2]));
        }
        else if(rgbRB.isChecked())
        {
            mulSlider->setValue(rgbParams[0]);
            mulEdit->setText(tmp.setNum(rgbParams[0]));
            divSlider->setValue(rgbParams[1]);
            divEdit->setText(tmp.setNum(rgbParams[1]));
            biasSlider->setValue(rgbParams[2]);
            biasEdit->setText(tmp.setNum(rgbParams[2]));
        }

        mulSlider->blockSignals(false);
        divSlider->blockSignals(false);
        biasSlider->blockSignals(false);
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/*!
 * \brief The qwNoteBox class.
 *        Defines an edit window for an image notes.
 */

class qwNoteBox : public QWidget
{
    Q_OBJECT

public:

    void setCurrentImage(const QSharedPointer<CImgContext> &imc)
    {
        setVisible(false);
        currentImgContextPtr = imc;
    }

    void showIfNecessary()
    {
        if(!currentImgContextPtr.isNull())
        {
            if(!isVisible())
            {
                if(currentImgContextPtr->notesLock.tryLock())
                    setVisible(true);
            }
            else
            {
                setWindowState( (windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
                raise();
                activateWindow();
            }
        }
    }

    qwNoteBox()
    {
        myLayout.addWidget(&myEditBox);
        myLayout.setContentsMargins(0, 0, 0, 0);
        myEditBox.setAcceptRichText(false);
        setWindowIcon(QIcon(":/icos/aid.png"));
        setWindowTitle("Image notes");
        resize(max(UI_NOTEBOX_MIN_WIDTH, Globals::mainWindowPtr->width()/2),
               max(UI_NOTEBOX_MIN_HEIGHT, Globals::mainWindowPtr->height()/2));
        setLayout(&myLayout);
    }


protected:

    void showEvent(QShowEvent *event)
    {
        if(currentImgContextPtr!=NULL)
        {
            myEditBox.setText(currentImgContextPtr->myNotes);
            this->setWindowTitle("["+currentImgContextPtr->getMyName()+"] notes");
        }
        QWidget::showEvent(event);
    }

    void hideEvent(QHideEvent *event)
    {
        if(!currentImgContextPtr.isNull())
        {
            currentImgContextPtr->myNotes = myEditBox.toPlainText();
            currentImgContextPtr->notesLock.unlock();
        }
        QWidget::hideEvent(event);
    }

private:

    QTextEdit     myEditBox;
    QVBoxLayout   myLayout;
    QSharedPointer<CImgContext>  currentImgContextPtr;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
/*!
 * \brief The qwPopUpMenuThumbnailList class.
 *        Defines an edit window for an image notes.
 */
class qwPopUpMenuThumbnailList : public QMenu
{
    Q_OBJECT

public:

    qwPopUpMenuThumbnailList(QWidget* parent): QMenu(parent)
    {

        actRename = addAction(QIcon(":/icos/label.png"),"Rename");
        actSaveAs = addAction(QIcon(":/icos/save.png"),"Save as");
        addSeparator();
        actDelete = addAction(QIcon(":/icos/trash.png"),"Delete");

        connect(this, SIGNAL(triggered(QAction*)), this, SLOT(dispatch(QAction*)));

        connect(this, SIGNAL(popupRename()), parent, SLOT(popupRename()));
        connect(this, SIGNAL(popupSaveAs()), parent, SLOT(popupSaveAs()));
        connect(this, SIGNAL(popupDelete()), parent, SLOT(popupDelete()));
    }

    void hideMe()
    {
        hide();
    }


signals:
    void popupRename();
    void popupSaveAs();
    void popupDelete();

private slots:
    void dispatch(QAction* actPtr)
    {
        if(actPtr == actRename) {emit popupRename();   return;}
        if(actPtr == actSaveAs) {emit popupSaveAs();   return;}
        if(actPtr == actDelete) {emit popupDelete();   return;}
    }

private:
    QAction* actRename;
    QAction* actSaveAs;
    QAction* actDelete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////
/*!
 * \brief The qwAboutDialog class.
 *        About dialog class.
 *
 */

class qwAboutDialog : public QWidget
{
    Q_OBJECT
public:

    static qwAboutDialog* myHandler;

    qwAboutDialog() : QWidget(0, Qt::Dialog)
    {
        setMinimumWidth(600);
        setWindowFlags(windowFlags()&~Qt::WindowContextHelpButtonHint);

        setAttribute( Qt::WA_DeleteOnClose, true );

        setWindowIcon(QIcon(":/icos/aid.png"));
        setWindowTitle("About");
        imageLabel.setPixmap(QPixmap(":/icos/logo.png"));
        imageLabel.setAlignment(Qt::AlignCenter);
        textLabel.setSource(QUrl("qrc:/text/info.html"));
        closeBtn.setText("Close");
        closeBtn.setFlat(true);
        imageLabel.setMargin(0);
        connect(&closeBtn, SIGNAL(clicked()), this, SLOT(close()));
        myLayout.addWidget(&imageLabel);
        myLayout.addWidget(&textLabel);
        myLayout.addWidget(&closeBtn);
        myLayout.setContentsMargins(0, 0, 0, 0);
        setLayout(&myLayout);

    }

protected:
    void showEvent(QShowEvent *){if(myHandler) myHandler->close(); myHandler = this;}
    void closeEvent(QCloseEvent *){if(myHandler==this) myHandler=0;}

private:
    QPixmap logoImg;
    QLabel imageLabel;
    QTextBrowser textLabel;
    QPushButton closeBtn;
    QVBoxLayout myLayout;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/*!
 * \brief The qwReinterpretDialog class.
 *        Reinterpret dialog class.
 *
 */

class qwReinterpretDialog : public QWidget
{
    Q_OBJECT
public:

    static qwReinterpretDialog *myHandler;

    qwReinterpretDialog(QWidget *parent, const QSharedPointer<CImgContext> &imgCtx) :QWidget(parent, Qt::Dialog)
    {
        this->imgCtx = imgCtx;

        setWindowFlags(windowFlags()&~Qt::WindowContextHelpButtonHint);

        setAttribute( Qt::WA_DeleteOnClose, true );
        setWindowIcon(QIcon(":/icos/aid.png"));
        setWindowTitle("Reinterpret source data");

        QFrame *upperLine = new QFrame(this);
        upperLine->setFrameShape(QFrame::HLine);
        upperLine->setFrameShadow(QFrame::Sunken);

        QFrame *bottomLine = new QFrame(this);
        bottomLine->setFrameShape(QFrame::HLine);
        bottomLine->setFrameShadow(QFrame::Sunken);

        generalInfo.setAlignment(Qt::AlignLeft);
        generalInfo.setText("This tool allows you to reinterpret the loaded yet source data."
                            "\nYou can change the image size, pixel format and adjust channel gain and bias."
                            "\nResulting visual data will be available as a new image.");

        cancelBtn.setText("Cancel");
        goBtn.setText("Go");

        dataSizeEdit.setDisabled(true);

        gLayout.addWidget(new QLabel("Width:"), 0, 0, Qt::AlignRight);
        gLayout.addWidget(&widthEdit, 0, 1);

        gLayout.addWidget(new QLabel("Height:"), 1, 0, Qt::AlignRight);
        gLayout.addWidget(&heightEdit, 1, 1);

        gLayout.addWidget(new QLabel("Name:"), 2, 0, Qt::AlignRight);
        gLayout.addWidget(&nameEdit, 2, 1);

        gLayout.addWidget(new QLabel("Pixel format:"), 3, 0, Qt::AlignRight);
        gLayout.addWidget(&pixelFormatEdit, 3, 1);

        gLayout.addWidget(new QLabel("Row stride in bits:"), 4, 0, Qt::AlignRight);
        gLayout.addWidget(&rowStrideInBitsEdit, 4, 1);

        gLayout.addWidget(new QLabel("Channel gain:"), 5, 0, Qt::AlignRight);
        QGridLayout *gainPanel = new QGridLayout();
        gainPanel->addWidget(new QLabel("R"),  0, 0);
        gainPanel->addWidget(new QLabel("G"),  0, 1);
        gainPanel->addWidget(new QLabel("B"),  0, 2);
        gainPanel->addWidget(new QLabel("A"),  0, 3);
        gainPanel->addWidget(&gainEdit[0],     1, 0);
        gainPanel->addWidget(&gainEdit[1],     1, 1);
        gainPanel->addWidget(&gainEdit[2],     1, 2);
        gainPanel->addWidget(&gainEdit[3],     1, 3);
        gLayout.addLayout(gainPanel, 5, 1, Qt::AlignRight);

        gLayout.addWidget(new QLabel("Channel bias:"), 6, 0, Qt::AlignRight);
        QGridLayout *biasPanel = new QGridLayout();
        biasPanel->addWidget(&biasEdit[0],     0, 0);
        biasPanel->addWidget(&biasEdit[1],     0, 1);
        biasPanel->addWidget(&biasEdit[2],     0, 2);
        biasPanel->addWidget(&biasEdit[3],     0, 3);
        gLayout.addLayout(biasPanel, 6, 1, Qt::AlignRight);

        gLayout.addWidget(new QLabel("Normalize:"), 7, 0, Qt::AlignRight);
        gLayout.addWidget(&autoGainAndBiasChckBox, 7, 1);


        gLayout.addWidget(new QLabel("Data size:"), 8,0, Qt::AlignRight);
        gLayout.addWidget(&dataSizeEdit, 8,1);

        if(!imgCtx.isNull())
        {
            widthEdit.setText(QString::number(imgCtx->getIWidth()));
            heightEdit.setText(QString::number(imgCtx->getIHeight()));
            nameEdit.setText("re_" + imgCtx->getMyName());
            pixelFormatEdit.setText(imgCtx->myPixelFormat);
            rowStrideInBitsEdit.setText(QString::number(imgCtx->rowStrideInBits));

            gainEdit[0].setText(QString::number(imgCtx->pgain[0],'f',3));
            gainEdit[1].setText(QString::number(imgCtx->pgain[1],'f',3));
            gainEdit[2].setText(QString::number(imgCtx->pgain[2],'f',3));
            gainEdit[3].setText(QString::number(imgCtx->pgain[3],'f',3));

            biasEdit[0].setText(QString::number(imgCtx->pbias[0],'f',3));
            biasEdit[1].setText(QString::number(imgCtx->pbias[1],'f',3));
            biasEdit[2].setText(QString::number(imgCtx->pbias[2],'f',3));
            biasEdit[3].setText(QString::number(imgCtx->pbias[3],'f',3));

            if(!imgCtx->nativeDataPtr.isNull())
                dataSizeEdit.setText(QString::number(imgCtx->nativeDataPtr->getData().size()) + "KB");
            else
                dataSizeEdit.setText("Not attached");
        }

        widthEdit.setValidator(new QIntValidator(1, MAX_IMAGE_SIZE, this));
        heightEdit.setValidator(new QIntValidator(1, MAX_IMAGE_SIZE, this));
        rowStrideInBitsEdit.setValidator(new QIntValidator(0, 255, this));
        nameEdit.setMaxLength(MAX_IMG_NAME_LENGTH);

        QPointer<QDoubleValidator> qdv[8];
        for(int i=0; i<8; i++) {qdv[i] = QPointer<QDoubleValidator>(new QDoubleValidator(this)); qdv[i]->setLocale(QLocale::C);}

        gainEdit[0].setValidator(qdv[0]);
        gainEdit[1].setValidator(qdv[1]);
        gainEdit[2].setValidator(qdv[2]);
        gainEdit[3].setValidator(qdv[3]);
        biasEdit[0].setValidator(qdv[4]);
        biasEdit[1].setValidator(qdv[5]);
        biasEdit[2].setValidator(qdv[6]);
        biasEdit[3].setValidator(qdv[7]);

        QWidget *filler = new QWidget(this);
        filler->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        btnBox.addWidget(filler);
        btnBox.addWidget(&goBtn, 0, Qt::AlignLeft);
        btnBox.addWidget(&cancelBtn, 0, Qt::AlignLeft);

        myLayout.addWidget(&generalInfo);
        myLayout.addWidget(upperLine);
        myLayout.addLayout(&gLayout);
        myLayout.addWidget(bottomLine);
        myLayout.addLayout(&btnBox);
        myLayout.setSizeConstraint(QLayout::SetFixedSize);
        setLayout(&myLayout);

        connect(&goBtn, SIGNAL(clicked()), this, SLOT(goPressed()));
        connect(&cancelBtn, SIGNAL(clicked()), this, SLOT(close()));

        pixelFormatEdit.setFocus();
     }


public slots:
    void goPressed()
    {
       QString name  = nameEdit.text();
       QString notes = imgCtx->myNotes;

       float pgain[4] = { gainEdit[0].text().toFloat(),
                          gainEdit[1].text().toFloat(),
                          gainEdit[2].text().toFloat(),
                          gainEdit[3].text().toFloat(),
                         };

       float pbias[4] = {biasEdit[0].text().toFloat(),
                         biasEdit[1].text().toFloat(),
                         biasEdit[2].text().toFloat(),
                         biasEdit[3].text().toFloat()};

       if(!Globals::isValidName(name))
       {
           name = "img"+QString::number(Globals::imgCountAbs+1);
           notes += "\nInvalid name has been selected. Generic one is assigned.";
       }


       CWorker_loadFromNativeData* newWorker = new CWorker_loadFromNativeData(NULL,
                                                                              imgCtx,
                                                                              max(widthEdit.text().toInt(),1),
                                                                              max(heightEdit.text().toInt(),1),
                                                                              pixelFormatEdit.text(),
                                                                              max(rowStrideInBitsEdit.text().toInt(), 0),
                                                                              name,
                                                                              notes,
                                                                              pgain,
                                                                              pbias,
                                                                              autoGainAndBiasChckBox.isChecked()?FILTER_FLAG_AUTO_GAIN_BIAS:0
                                                                              );
       newWorker->selfStart();
       close();
    }


protected:
    void showEvent(QShowEvent *){if(myHandler) myHandler->close(); myHandler = this;}
    void closeEvent(QCloseEvent *){if(myHandler==this) myHandler=0;}


private:

    QLabel      generalInfo;
    QLineEdit   widthEdit;
    QLineEdit   heightEdit;
    QLineEdit   nameEdit;
    QLineEdit   pixelFormatEdit;
    QLineEdit   rowStrideInBitsEdit;
    QLineEdit   dataSizeEdit;
    //QLineEdit   autoGainAndBiasEdit;
    QLineEdit   gainEdit[4];
    QLineEdit   biasEdit[4];
    QVBoxLayout myLayout;
    QHBoxLayout btnBox;
    QGridLayout gLayout;
    QCheckBox   autoGainAndBiasChckBox;
    QPushButton cancelBtn;
    QPushButton goBtn;
    QSharedPointer<CImgContext> imgCtx;

};


///////////////////////////////////////////////////////////////////////////////////////////////////
/*!
 * \brief The qwRecastDialog class.
 *        Image recaster dialog class.
 *
 */

class qwRecastDataDialog : public QWidget
{
    Q_OBJECT
public:

    static qwRecastDataDialog *myHandler;

    qwRecastDataDialog(QWidget *parent, const QSharedPointer<CImgContext> &imgCtx) :QWidget(parent, Qt::Dialog)
    {
        this->imgCtx = imgCtx;

        setWindowFlags(windowFlags()&~Qt::WindowContextHelpButtonHint);


        setAttribute( Qt::WA_DeleteOnClose, true );
        setWindowIcon(QIcon(":/icos/aid.png"));
        setWindowTitle("Recast source data");

        QFrame *upperLine = new QFrame(this);
        upperLine->setFrameShape(QFrame::HLine);
        upperLine->setFrameShadow(QFrame::Sunken);

        QFrame *bottomLine = new QFrame(this);
        bottomLine->setFrameShape(QFrame::HLine);
        bottomLine->setFrameShadow(QFrame::Sunken);

        generalInfo.setAlignment(Qt::AlignLeft);
        generalInfo.setText("This tool allows you to recast a given source data to [fR32G32B32A32] pixelformat. You can additionally"
                            "\nalter the image gain/bias and its orientation. Resulting visual data will be available as a new image.");

        cancelBtn.setText("Cancel");
        goBtn.setText("Go");


        gLayout.addWidget(new QLabel("Name:"), 0, 0, Qt::AlignRight);
        gLayout.addWidget(&nameEdit, 0, 1);
        nameEdit.setText(("re"));

        gLayout.addWidget(new QLabel("Vertical flip:"), 1, 0, Qt::AlignRight);
        gLayout.addWidget(&vFlipChkBox, 1, 1);
        vFlipChkBox.setChecked(customChcks[0]);

        gLayout.addWidget(new QLabel("Horizontal flip:"), 2, 0, Qt::AlignRight);
        gLayout.addWidget(&hFlipChkBox, 2, 1);
        hFlipChkBox.setChecked(customChcks[1]);

        gLayout.addWidget(new QLabel("Transform mode:"), 3, 0, Qt::AlignRight);
        predefinedTransformsList.addItem(predefinedTransformItems[0]);
        predefinedTransformsList.addItem(predefinedTransformItems[1]);
        predefinedTransformsList.addItem(predefinedTransformItems[2]);
        predefinedTransformsList.addItem(predefinedTransformItems[3]);
        predefinedTransformsList.addItem(predefinedTransformItems[4]);
        predefinedTransformsList.addItem(predefinedTransformItems[5]);
        predefinedTransformsList.setCurrentIndex(0);

        gLayout.addWidget(&predefinedTransformsList, 3, 1);

        QGridLayout *outChannelsLabelPanel = new QGridLayout();
        outChannelsLabelPanel->addWidget(new QLabel("______|"), 0, 0);
        outChannelsLabelPanel->addWidget(new QLabel("R_out="),  1, 0);
        outChannelsLabelPanel->addWidget(new QLabel("G_out="),  2, 0);
        outChannelsLabelPanel->addWidget(new QLabel("B_out="),  3, 0);
        outChannelsLabelPanel->addWidget(new QLabel("A_out="),  4, 0);
        gLayout.addLayout(outChannelsLabelPanel, 4, 0, Qt::AlignRight);

        QGridLayout *transformLayout = new QGridLayout();
        transformLayout->addWidget(new QLabel("R_in"), 0, 0);
        transformLayout->addWidget(new QLabel("G_in"), 0, 1);
        transformLayout->addWidget(new QLabel("B_in"), 0, 2);
        transformLayout->addWidget(new QLabel("A_in"), 0, 3);
        transformLayout->addWidget(new QLabel("bias"), 0, 4);

        transformLayout->addWidget(&gainEdit[0][0], 1, 0);
        transformLayout->addWidget(&gainEdit[0][1], 1, 1);
        transformLayout->addWidget(&gainEdit[0][2], 1, 2);
        transformLayout->addWidget(&gainEdit[0][3], 1, 3);
        transformLayout->addWidget(&biasEdit[0]   , 1, 4);

        transformLayout->addWidget(&gainEdit[1][0], 2, 0);
        transformLayout->addWidget(&gainEdit[1][1], 2, 1);
        transformLayout->addWidget(&gainEdit[1][2], 2, 2);
        transformLayout->addWidget(&gainEdit[1][3], 2, 3);
        transformLayout->addWidget(&biasEdit[1]   , 2, 4);

        transformLayout->addWidget(&gainEdit[2][0], 3, 0);
        transformLayout->addWidget(&gainEdit[2][1], 3, 1);
        transformLayout->addWidget(&gainEdit[2][2], 3, 2);
        transformLayout->addWidget(&gainEdit[2][3], 3, 3);
        transformLayout->addWidget(&biasEdit[2]   , 3, 4);

        transformLayout->addWidget(&gainEdit[3][0], 4, 0);
        transformLayout->addWidget(&gainEdit[3][1], 4, 1);
        transformLayout->addWidget(&gainEdit[3][2], 4, 2);
        transformLayout->addWidget(&gainEdit[3][3], 4, 3);
        transformLayout->addWidget(&biasEdit[3]   , 4, 4);
        gLayout.addLayout(transformLayout, 4, 1);

        gLayout.addWidget(new QLabel("Auto normalize:"), 5, 0, Qt::AlignRight);
        gLayout.addWidget(&autoNormalizeChkBox, 5, 1);
        autoNormalizeChkBox.setChecked(customChcks[2]);

        QPointer<QDoubleValidator> gainVal[4][4];
        for(int r=0; r<4; r++)
            for(int c=0; c<4; c++)
            {gainVal[r][c] = QPointer<QDoubleValidator>(new QDoubleValidator(this)); gainVal[r][c]->setLocale(QLocale::C);
             gainEdit[r][c].setValidator(gainVal[r][c]);
            }

        QPointer<QDoubleValidator> biasVal[4];
        for(int i=0; i<4; i++) {biasVal[i] = QPointer<QDoubleValidator>(new QDoubleValidator(this)); biasVal[i]->setLocale(QLocale::C);}
        biasEdit[0].setValidator(biasVal[0]);
        biasEdit[1].setValidator(biasVal[1]);
        biasEdit[2].setValidator(biasVal[2]);
        biasEdit[3].setValidator(biasVal[3]);

        if(!imgCtx.isNull())
        {
            nameEdit.setText("re_" + imgCtx->getMyName());
            predefinedTransformSelection(predefinedTransformItems[0]);
        }

        nameEdit.setMaxLength(MAX_IMG_NAME_LENGTH);

        QWidget *filler = new QWidget(this);
        filler->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        btnBox.addWidget(filler);
        btnBox.addWidget(&goBtn, 0, Qt::AlignLeft);
        btnBox.addWidget(&cancelBtn, 0, Qt::AlignLeft);

        myLayout.addWidget(&generalInfo);
        myLayout.addWidget(upperLine);
        myLayout.addLayout(&gLayout);
        myLayout.addWidget(bottomLine);
        myLayout.addLayout(&btnBox);
        myLayout.setSizeConstraint(QLayout::SetFixedSize);
        setLayout(&myLayout);

        connect(&goBtn, SIGNAL(clicked()), this, SLOT(goPressed()));
        connect(&cancelBtn, SIGNAL(clicked()), this, SLOT(close()));
        connect(&predefinedTransformsList, SIGNAL(activated(QString)),
                this, SLOT(predefinedTransformSelection(QString)));

        nameEdit.setFocus();
     }


public slots:
    void goPressed()
    {
       QString name  = nameEdit.text();
       QString notes = imgCtx->myNotes;


       float pgain[16] = {gainEdit[0][0].text().toFloat(),
                          gainEdit[0][1].text().toFloat(),
                          gainEdit[0][2].text().toFloat(),
                          gainEdit[0][3].text().toFloat(),
                          gainEdit[1][0].text().toFloat(),
                          gainEdit[1][1].text().toFloat(),
                          gainEdit[1][2].text().toFloat(),
                          gainEdit[1][3].text().toFloat(),
                          gainEdit[2][0].text().toFloat(),
                          gainEdit[2][1].text().toFloat(),
                          gainEdit[2][2].text().toFloat(),
                          gainEdit[2][3].text().toFloat(),
                          gainEdit[3][0].text().toFloat(),
                          gainEdit[3][1].text().toFloat(),
                          gainEdit[3][2].text().toFloat(),
                          gainEdit[3][3].text().toFloat()
                         };

       float pbias[4] = {biasEdit[0].text().toFloat(),
                         biasEdit[1].text().toFloat(),
                         biasEdit[2].text().toFloat(),
                         biasEdit[3].text().toFloat()};

       memcpy(customParameters   , pgain, sizeof(float)*16);
       memcpy(customParameters+16, pbias, sizeof(float)*4);

       customChcks[0] = vFlipChkBox.isChecked();
       customChcks[1] = hFlipChkBox.isChecked();
       customChcks[2] = autoNormalizeChkBox.isChecked();

       if(!Globals::isValidName(name))
       {
           name = "img"+QString::number(Globals::imgCountAbs+1);
           notes += "\nInvalid name has been selected. Generic one is assigned.";
       }

       CWorker_ImageRecaster* newWorker = new CWorker_ImageRecaster(NULL,
                                                                    pgain,
                                                                    pbias,
                                                                    vFlipChkBox.isChecked(),
                                                                    hFlipChkBox.isChecked(),
                                                                    autoNormalizeChkBox.isChecked()?FILTER_FLAG_AUTO_GAIN_BIAS:0,
                                                                    name,
                                                                    imgCtx);

       newWorker->selfStart();

       close();
    }

void predefinedTransformSelection(QString itemStr)
{
    if(itemStr == predefinedTransformItems[1])
    {
        gainEdit[0][0].setText(QString::number(imgCtx->pgain[0]));
        gainEdit[0][1].setText("0.0");
        gainEdit[0][2].setText("0.0");
        gainEdit[0][3].setText("0.0");

        gainEdit[1][0].setText("0.0");
        gainEdit[1][1].setText(QString::number(imgCtx->pgain[1]));
        gainEdit[1][2].setText("0.0");
        gainEdit[1][3].setText("0.0");

        gainEdit[2][0].setText("0.0");
        gainEdit[2][1].setText("0.0");
        gainEdit[2][2].setText(QString::number(imgCtx->pgain[2]));
        gainEdit[2][3].setText("0.0");

        gainEdit[3][0].setText("0.0");
        gainEdit[3][1].setText("0.0");
        gainEdit[3][2].setText("0.0");
        gainEdit[3][3].setText(QString::number(imgCtx->pgain[3]));

        biasEdit[0].setText(QString::number(imgCtx->pbias[0]));
        biasEdit[1].setText(QString::number(imgCtx->pbias[1]));
        biasEdit[2].setText(QString::number(imgCtx->pbias[2]));
        biasEdit[3].setText(QString::number(imgCtx->pbias[3]));
    }
    else if(itemStr == predefinedTransformItems[2])
    {
        gainEdit[0][0].setText("1.0");
        gainEdit[0][1].setText("0.0");
        gainEdit[0][2].setText("0.0");
        gainEdit[0][3].setText("0.0");

        gainEdit[1][0].setText("0.0");
        gainEdit[1][1].setText("1.0");
        gainEdit[1][2].setText("0.0");
        gainEdit[1][3].setText("0.0");

        gainEdit[2][0].setText("0.0");
        gainEdit[2][1].setText("0.0");
        gainEdit[2][2].setText("1.0");
        gainEdit[2][3].setText("0.0");

        gainEdit[3][0].setText("0.0");
        gainEdit[3][1].setText("0.0");
        gainEdit[3][2].setText("0.0");
        gainEdit[3][3].setText("1.0");

        biasEdit[0].setText("0.0");
        biasEdit[1].setText("0.0");
        biasEdit[2].setText("0.0");
        biasEdit[3].setText("0.0");
    }
    else if(itemStr == predefinedTransformItems[3])
    {
        gainEdit[0][0].setText("-1.0");
        gainEdit[0][1].setText("0.0");
        gainEdit[0][2].setText("0.0");
        gainEdit[0][3].setText("0.0");

        gainEdit[1][0].setText("0.0");
        gainEdit[1][1].setText("-1.0");
        gainEdit[1][2].setText("0.0");
        gainEdit[1][3].setText("0.0");

        gainEdit[2][0].setText("0.0");
        gainEdit[2][1].setText("0.0");
        gainEdit[2][2].setText("-1.0");
        gainEdit[2][3].setText("0.0");

        gainEdit[3][0].setText("0.0");
        gainEdit[3][1].setText("0.0");
        gainEdit[3][2].setText("0.0");
        gainEdit[3][3].setText("0.0");

        biasEdit[0].setText("1.0");
        biasEdit[1].setText("1.0");
        biasEdit[2].setText("1.0");
        biasEdit[3].setText("1.0");
    }
    else if(itemStr == predefinedTransformItems[4])
    {
        gainEdit[0][0].setText("1.0");
        gainEdit[0][1].setText("0.9563");
        gainEdit[0][2].setText("0.6210");
        gainEdit[0][3].setText("0.0");

        gainEdit[1][0].setText("1.0");
        gainEdit[1][1].setText("-0.2721");
        gainEdit[1][2].setText("-0.6474");
        gainEdit[1][3].setText("0.0");

        gainEdit[2][0].setText("1.0");
        gainEdit[2][1].setText("-1.1070");
        gainEdit[2][2].setText("1.7046");
        gainEdit[2][3].setText("0.0");

        gainEdit[3][0].setText("0.0");
        gainEdit[3][1].setText("0.0");
        gainEdit[3][2].setText("0.0");
        gainEdit[3][3].setText("0.0");

        biasEdit[0].setText("0.0");
        biasEdit[1].setText("0.0");
        biasEdit[2].setText("0.0");
        biasEdit[3].setText("1.0");
    }
    else if(itemStr == predefinedTransformItems[5])
    {
        gainEdit[0][0].setText("1.0");
        gainEdit[0][1].setText("0.0");
        gainEdit[0][2].setText("1.13983");
        gainEdit[0][3].setText("0.0");

        gainEdit[1][0].setText("1.0");
        gainEdit[1][1].setText("-0.39465");
        gainEdit[1][2].setText("-0.58060");
        gainEdit[1][3].setText("0.0");

        gainEdit[2][0].setText("1.0");
        gainEdit[2][1].setText("2.03211");
        gainEdit[2][2].setText("0.0");
        gainEdit[2][3].setText("0.0");

        gainEdit[3][0].setText("0.0");
        gainEdit[3][1].setText("0.0");
        gainEdit[3][2].setText("0.0");
        gainEdit[3][3].setText("0.0");

        biasEdit[0].setText("0.0");
        biasEdit[1].setText("0.0");
        biasEdit[2].setText("0.0");
        biasEdit[3].setText("1.0");
    }
    else
    {
        gainEdit[0][0].setText(QString::number(customParameters[0]));
        gainEdit[0][1].setText(QString::number(customParameters[1]));
        gainEdit[0][2].setText(QString::number(customParameters[2]));
        gainEdit[0][3].setText(QString::number(customParameters[3]));

        gainEdit[1][0].setText(QString::number(customParameters[4]));
        gainEdit[1][1].setText(QString::number(customParameters[5]));
        gainEdit[1][2].setText(QString::number(customParameters[6]));
        gainEdit[1][3].setText(QString::number(customParameters[7]));

        gainEdit[2][0].setText(QString::number(customParameters[8]));
        gainEdit[2][1].setText(QString::number(customParameters[9]));
        gainEdit[2][2].setText(QString::number(customParameters[10]));
        gainEdit[2][3].setText(QString::number(customParameters[11]));

        gainEdit[3][0].setText(QString::number(customParameters[12]));
        gainEdit[3][1].setText(QString::number(customParameters[13]));
        gainEdit[3][2].setText(QString::number(customParameters[14]));
        gainEdit[3][3].setText(QString::number(customParameters[15]));

        biasEdit[0].setText(QString::number(customParameters[16]));
        biasEdit[1].setText(QString::number(customParameters[17]));
        biasEdit[2].setText(QString::number(customParameters[18]));
        biasEdit[3].setText(QString::number(customParameters[19]));
    }

}

protected:
    void showEvent(QShowEvent *){if(myHandler) myHandler->close(); myHandler = this;}
    void closeEvent(QCloseEvent *){if(myHandler==this) myHandler=0;}


private:

    QLabel      generalInfo;
    QLineEdit   nameEdit;
    //QLineEdit   autoGainAndBiasEdit;
    QCheckBox   vFlipChkBox;
    QCheckBox   hFlipChkBox;
    QComboBox   predefinedTransformsList;
    QCheckBox   autoNormalizeChkBox;
    QLineEdit   gainEdit[4][4];
    QLineEdit   biasEdit[4];
    QVBoxLayout myLayout;
    QHBoxLayout btnBox;
    QGridLayout gLayout;
    QPushButton cancelBtn;
    QPushButton goBtn;
    QSharedPointer<CImgContext> imgCtx;

    static float customParameters[20];
    static bool  customChcks[3];
    static const char* predefinedTransformItems[];

};



///////////////////////////////////////////////////////////////////////////////////////////////////
/*!
 * \brief The qwImageComparatorDialog class.
 *        Reinterpret dialog class.
 *
 */

class qwImageComparatorDialog : public QWidget
{
    Q_OBJECT

public:

    static qwImageComparatorDialog* myHandler;

    qwImageComparatorDialog(QWidget *parent, const QSharedPointer<CImgContext> &imgA, const QSharedPointer<CImgContext> &imgB) :QWidget(parent, Qt::Dialog)
    {

        this->imgA = imgA;
        this->imgB = imgB;

        setWindowFlags(windowFlags()&~Qt::WindowContextHelpButtonHint);

        if((imgA.isNull())||(imgB.isNull()))
            return;

        QFrame *upperLine = new QFrame(this);
        upperLine->setFrameShape(QFrame::HLine);
        upperLine->setFrameShadow(QFrame::Sunken);

        QFrame *bottomLine = new QFrame(this);
        bottomLine->setFrameShape(QFrame::HLine);
        bottomLine->setFrameShadow(QFrame::Sunken);


        setAttribute( Qt::WA_DeleteOnClose, true );
        setWindowIcon(QIcon(":/icos/aid.png"));
        setWindowTitle("Image comparator");

        generalInfo.setAlignment(Qt::AlignLeft);
        generalInfo.setText("This tool allows you to compare SOURCE DATA of two images selected on the [left/top] and [right/bottom] panels."
                            "\nNew visual data will be created as a result of subtraction [left/top] - [right/bottom] gated by the threshold"
                            "\nvalues. The visual data will be normalized: abs([left/top] - [right/bottom]) and scaled to fit within the <0-1>"
                            "\nrange. Additional statistics will be available in the resulting image notes. All modifiers (shift/flip) are applied"
                            "\nto the [left/top] image. Notice that checking 'Swap images' option causes [left/top] with [right/bottom]"
                            "\nimage swap for the above mentioned operations.");

        cancelBtn.setText("Cancel");
        goBtn.setText("Go");

        gLayout.addWidget(new QLabel("x shift:"), 0, 0, Qt::AlignRight);
        gLayout.addWidget(&xShiftEdit, 0, 1);
        xShiftEdit.setText(QString::number(customShift[0]));

        gLayout.addWidget(new QLabel("y shift:"), 1, 0, Qt::AlignRight);
        gLayout.addWidget(&yShiftEdit, 1, 1);
        yShiftEdit.setText(QString::number(customShift[1]));

        gLayout.addWidget(new QLabel("Vertical flip:"), 2, 0, Qt::AlignRight);
        gLayout.addWidget(&vFlipChkBox, 2, 1);
        vFlipChkBox.setChecked(customChkcks[0]);

        gLayout.addWidget(new QLabel("Horizontal flip:"), 3, 0, Qt::AlignRight);
        gLayout.addWidget(&hFlipChkBox, 3, 1);
        hFlipChkBox.setChecked(customChkcks[1]);

        gLayout.addWidget(new QLabel("Swap images:"), 4, 0, Qt::AlignRight);
        gLayout.addWidget(&swapImagesChkBox, 4, 1);
        swapImagesChkBox.setChecked(customChkcks[2]);
        gLayout.addWidget(new QLabel("Threshold leves:"), 5, 0, Qt::AlignRight);

        QGridLayout *thresholdChannelsPanel = new QGridLayout();
        thresholdChannelsPanel->addWidget(new QLabel("Rt=")  , 0, 0);
        thresholdChannelsPanel->addWidget(&thresholdValues[0], 0, 1);
        thresholdChannelsPanel->addWidget(new QLabel("Gt=")  , 0, 2);
        thresholdChannelsPanel->addWidget(&thresholdValues[1], 0, 3);
        thresholdChannelsPanel->addWidget(new QLabel("Bt=")  , 0, 4);
        thresholdChannelsPanel->addWidget(&thresholdValues[2], 0, 5);
        thresholdChannelsPanel->addWidget(new QLabel("At=")  , 0, 6);
        thresholdChannelsPanel->addWidget(&thresholdValues[3], 0, 7);
        gLayout.addLayout(thresholdChannelsPanel, 5, 1, Qt::AlignRight);

        thresholdValues[0].setText(QString::number(customThresholds[0]));
        thresholdValues[1].setText(QString::number(customThresholds[1]));
        thresholdValues[2].setText(QString::number(customThresholds[2]));
        thresholdValues[3].setText(QString::number(customThresholds[3]));

        QPointer<QDoubleValidator> qdv[4];
        for(int i=0; i<4; i++) {qdv[i] = QPointer<QDoubleValidator>(new QDoubleValidator(0.0f, 1.0f, 4, this)); qdv[i]->setLocale(QLocale::C);
            thresholdValues[i].setValidator(qdv[i]);}

        gLayout.addWidget(new QLabel("Name:"), 6, 0,Qt::AlignRight);
        gLayout.addWidget(&nameEdit, 6, 1);
        nameEdit.setText("Cmp_" + imgA->getMyName() + "_" + imgB->getMyName());

        QWidget *filler = new QWidget(this);
        filler->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        btnBox.addWidget(filler);
        btnBox.addWidget(&goBtn, 0, Qt::AlignLeft);
        btnBox.addWidget(&cancelBtn, 0, Qt::AlignLeft);

        myLayout.addWidget(&generalInfo);
        myLayout.addWidget(upperLine);
        myLayout.addLayout(&gLayout);
        myLayout.addWidget(bottomLine);
        myLayout.addLayout(&btnBox);

        myLayout.setSizeConstraint(QLayout::SetFixedSize);
        setLayout(&myLayout);

        xShiftEdit.setValidator(new QIntValidator(this));
        yShiftEdit.setValidator(new QIntValidator(this));

        connect(&goBtn, SIGNAL(clicked()), this, SLOT(goPressed()));
        connect(&cancelBtn, SIGNAL(clicked()), this, SLOT(close()));
    }

protected:
    void showEvent(QShowEvent *){if(myHandler) myHandler->close(); myHandler = this;}
    void closeEvent(QCloseEvent *){if(myHandler==this) myHandler=0;}

public slots:
    void goPressed()
    {
        float thrsholdsv[4] = {thresholdValues[0].text().toFloat(),
                               thresholdValues[1].text().toFloat(),
                               thresholdValues[2].text().toFloat(),
                               thresholdValues[3].text().toFloat()};

        memcpy(customThresholds, thrsholdsv, sizeof(float)*4);
        customChkcks[0] = vFlipChkBox.isChecked();
        customChkcks[1] = hFlipChkBox.isChecked();
        customChkcks[2] = swapImagesChkBox.isChecked();

        customShift[0]  = xShiftEdit.text().toInt();
        customShift[1]  = yShiftEdit.text().toInt();

        QString name  = nameEdit.text();
        QString notes = "Comparison result of: " + imgA->getMyName() + " and " + imgB->getMyName() + ".";
        if(!Globals::isValidName(name))
               {
                   name = "img"+QString::number(Globals::imgCountAbs+1);
                   notes += "\nInvalid name has been selected. Generic one is assigned.";
               }

        CWorker_ImageComparator* newWorker = new CWorker_ImageComparator(NULL,
                                                                    xShiftEdit.text().toInt(),
                                                                    yShiftEdit.text().toInt(),
                                                                    vFlipChkBox.isChecked(),
                                                                    hFlipChkBox.isChecked(),
                                                                    thrsholdsv,
                                                                    name,
                                                                    swapImagesChkBox.isChecked()?imgA:imgB,
                                                                    swapImagesChkBox.isChecked()?imgB:imgA);

        newWorker->selfStart();
        close();
    }

private:

    QLabel      generalInfo;
    QLineEdit   xShiftEdit;
    QLineEdit   yShiftEdit;
    QCheckBox   vFlipChkBox;
    QCheckBox   hFlipChkBox;
    QCheckBox   swapImagesChkBox;
    QLineEdit   thresholdValues[4];
    QLineEdit   nameEdit;
    QVBoxLayout myLayout;
    QHBoxLayout btnBox;
    QGridLayout gLayout;
    QPushButton cancelBtn;
    QPushButton goBtn;

    static float customThresholds[4];
    static float customShift[2];
    static bool  customChkcks[3];

    QSharedPointer<CImgContext> imgA;
    QSharedPointer<CImgContext> imgB;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/*!
 * \brief The qwRawHeaderEditor class.
 *
 *        Dialog for editing header's parameters for RAW files.
 */

class qwRawHeaderEditor : public QWidget
{
    Q_OBJECT

public:

    static qwRawHeaderEditor* myHandler;

    qwRawHeaderEditor(QWidget *parent, QFileInfo rawFileInfo) : QWidget(parent, Qt::Dialog)
    {
        this->rawFileInfo = rawFileInfo;

        QFrame *upperLine = new QFrame(this);
        upperLine->setFrameShape(QFrame::HLine);
        upperLine->setFrameShadow(QFrame::Sunken);

        QFrame *bottomLine = new QFrame(this);
        bottomLine->setFrameShape(QFrame::HLine);
        bottomLine->setFrameShadow(QFrame::Sunken);

        setAttribute( Qt::WA_DeleteOnClose, true );
        setWindowIcon(QIcon(":/icos/aid.png"));
        setWindowTitle("RAW header editor");

        generalInfo.setAlignment(Qt::AlignLeft);
        generalInfo.setText("Edit the loaded data parameters.");

        cancelBtn.setText("Cancel");
        goBtn.setText("Go");

        gLayout.addWidget(new QLabel("Width:"), 0, 0, Qt::AlignRight);
        gLayout.addWidget(&widthEdit, 0, 1);
        widthEdit.setText(QString::number(rawHeader.width));

        gLayout.addWidget(new QLabel("Height:"), 1, 0, Qt::AlignRight);
        gLayout.addWidget(&heightEdit, 1, 1);
        heightEdit.setText(QString::number(rawHeader.height));

        gLayout.addWidget(new QLabel("Name:"), 2, 0, Qt::AlignRight);
        gLayout.addWidget(&nameEdit, 2, 1);
        nameEdit.setText(rawFileInfo.baseName());

        gLayout.addWidget(new QLabel("Pixel format:"), 3, 0, Qt::AlignRight);
        gLayout.addWidget(&pixelFormatEdit, 3, 1);
        pixelFormatEdit.setText(pixelFormatString);

        gLayout.addWidget(new QLabel("Row stride in bits:"), 4, 0, Qt::AlignRight);
        gLayout.addWidget(&rowStrideInBitsEdit, 4, 1);
        rowStrideInBitsEdit.setText(QString::number(rawHeader.rowStrideInBits));

        gLayout.addWidget(new QLabel("Channel gain:"), 5, 0, Qt::AlignRight);
        QGridLayout *gainPanel = new QGridLayout();
        gainPanel->addWidget(new QLabel("R"),  0, 0);
        gainPanel->addWidget(new QLabel("G"),  0, 1);
        gainPanel->addWidget(new QLabel("B"),  0, 2);
        gainPanel->addWidget(new QLabel("A"),  0, 3);
        gainPanel->addWidget(&gainEdit[0],     1, 0);
        gainPanel->addWidget(&gainEdit[1],     1, 1);
        gainPanel->addWidget(&gainEdit[2],     1, 2);
        gainPanel->addWidget(&gainEdit[3],     1, 3);
        gLayout.addLayout(gainPanel, 5, 1, Qt::AlignRight);

        gLayout.addWidget(new QLabel("Channel bias:"), 6, 0, Qt::AlignRight);
        QGridLayout *biasPanel = new QGridLayout();
        biasPanel->addWidget(&biasEdit[0],     0, 0);
        biasPanel->addWidget(&biasEdit[1],     0, 1);
        biasPanel->addWidget(&biasEdit[2],     0, 2);
        biasPanel->addWidget(&biasEdit[3],     0, 3);
        gLayout.addLayout(biasPanel, 6, 1, Qt::AlignRight);

        gLayout.addWidget(new QLabel("Normalize:"), 7, 0, Qt::AlignRight);
        gLayout.addWidget(&autoGainAndBiasChckBox, 7, 1);

        gLayout.addWidget(new QLabel("Data size:"), 8,0, Qt::AlignRight);
        gLayout.addWidget(&dataSizeEdit, 8,1);
        dataSizeEdit.setDisabled(true);
        dataSizeEdit.setText(QString::number(rawFileInfo.size())+ "KB");

        gainEdit[0].setText(QString::number(rawHeader.normGain[0]));
        gainEdit[1].setText(QString::number(rawHeader.normGain[1]));
        gainEdit[2].setText(QString::number(rawHeader.normGain[2]));
        gainEdit[3].setText(QString::number(rawHeader.normGain[3]));

        biasEdit[0].setText(QString::number(rawHeader.normBias[0]));
        biasEdit[1].setText(QString::number(rawHeader.normBias[1]));
        biasEdit[2].setText(QString::number(rawHeader.normBias[2]));
        biasEdit[3].setText(QString::number(rawHeader.normBias[3]));

        widthEdit.setValidator(new QIntValidator(1, MAX_IMAGE_SIZE, this));
        heightEdit.setValidator(new QIntValidator(1, MAX_IMAGE_SIZE, this));
        rowStrideInBitsEdit.setValidator(new QIntValidator(0, 255, this));
        nameEdit.setMaxLength(MAX_IMG_NAME_LENGTH);

        QPointer<QDoubleValidator> qdv[8];
        for(int i=0; i<8; i++) {qdv[i] = QPointer<QDoubleValidator>(new QDoubleValidator(this)); qdv[i]->setLocale(QLocale::C);}

        gainEdit[0].setValidator(qdv[0]);
        gainEdit[1].setValidator(qdv[1]);
        gainEdit[2].setValidator(qdv[2]);
        gainEdit[3].setValidator(qdv[3]);
        biasEdit[0].setValidator(qdv[4]);
        biasEdit[1].setValidator(qdv[5]);
        biasEdit[2].setValidator(qdv[6]);
        biasEdit[3].setValidator(qdv[7]);

        QWidget *filler = new QWidget(this);
        filler->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        btnBox.addWidget(filler);
        btnBox.addWidget(&goBtn, 0, Qt::AlignLeft);
        btnBox.addWidget(&cancelBtn, 0, Qt::AlignLeft);

        myLayout.addWidget(&generalInfo);
        myLayout.addWidget(upperLine);
        myLayout.addLayout(&gLayout);
        myLayout.addWidget(bottomLine);
        myLayout.addLayout(&btnBox);
        myLayout.setSizeConstraint(QLayout::SetFixedSize);
        setLayout(&myLayout);

        connect(&goBtn, SIGNAL(clicked()), this, SLOT(goPressed()));
        connect(&cancelBtn, SIGNAL(clicked()), this, SLOT(close()));

        pixelFormatEdit.setFocus();
    }


public slots:
    void goPressed()
    {
       QByteArray hrawBuff;

       if(!Globals::isValidName(nameEdit.text()))
       {
           nameEdit.text() = "img"+QString::number(Globals::imgCountAbs+1);
           notesEdit.setText(nameEdit.text() + "\nInvalid name has been selected. Generic one is assigned.");
       }

       rawHeader.width  = max(widthEdit.text().toInt(),1);
       rawHeader.height = max(heightEdit.text().toInt(),1);
       rawHeader.formatStrLength = pixelFormatEdit.text().size();
       rawHeader.nameLength = nameEdit.text().size();
       rawHeader.notesLength = notesEdit.text().size();
       rawHeader.rowStrideInBits = rowStrideInBitsEdit.text().toInt();

       rawHeader.normGain[0] = gainEdit[0].text().toFloat();
       rawHeader.normGain[1] = gainEdit[1].text().toFloat();
       rawHeader.normGain[2] = gainEdit[2].text().toFloat();
       rawHeader.normGain[3] = gainEdit[3].text().toFloat();

       rawHeader.normBias[0] = biasEdit[0].text().toFloat();
       rawHeader.normBias[1] = biasEdit[1].text().toFloat();
       rawHeader.normBias[2] = biasEdit[2].text().toFloat();
       rawHeader.normBias[3] = biasEdit[3].text().toFloat();

       rawHeader.auxFiltering = autoGainAndBiasChckBox.isChecked()?FILTER_FLAG_AUTO_GAIN_BIAS:0;

      /*
       rawHeader.sizeInBytes =  MAGIC_CHARS_SIZE
                                + sizeof(dHeader)
                                + rawHeader.formatStrLength
                                + rawHeader.nameLength
                                + rawHeader.notesLength
                                + rawFileInfo.size();
                                */

       rawHeader.sizeInBytes = rawFileInfo.size();

       hrawBuff.append(magichars);
       hrawBuff.append((char*)&rawHeader, sizeof(dHeader));
       hrawBuff.append(pixelFormatEdit.text());
       hrawBuff.append(nameEdit.text());
       hrawBuff.append(notesEdit.text());

       QFile rawFile(rawFileInfo.absoluteFilePath());

       if(!rawFile.open(QIODevice::ReadOnly))
       {
           showStatusMessage("Error opening the file.", UI_STATUS_ERROR, true);
           close();
       }

       QByteArray qbuff = rawFile.readAll();

       if(qbuff.size() == 0)
       {
           showStatusMessage("No data has been loaded.", UI_STATUS_ERROR, true);
           rawFile.close();
           close();
       }

       hrawBuff.append(qbuff);

       char* rawBitsPtr = (char*)malloc(hrawBuff.size());
       memcpy(rawBitsPtr, hrawBuff.data(), hrawBuff.size());

       CWorker_loadFromNativeData* newWorker = new CWorker_loadFromNativeData(NULL, rawBitsPtr, hrawBuff.size());
       newWorker->selfStart();
       rawFile.close();
       close();
    }

private:

    QFileInfo       rawFileInfo;
    QLabel          generalInfo;
    QLineEdit       widthEdit;
    QLineEdit       heightEdit;
    QLineEdit       pixelFormatEdit;
    QLineEdit       sizeInBytesEdit;
    QLineEdit       nameEdit;
    QLineEdit       notesEdit;
    QLineEdit       dataSizeEdit;
    QLineEdit       rowStrideInBitsEdit;
    QLineEdit       gainEdit[4];
    QLineEdit       biasEdit[4];
    QCheckBox       autoGainAndBiasChckBox;

    QVBoxLayout     myLayout;
    QHBoxLayout     btnBox;
    QGridLayout     gLayout;
    QPushButton     cancelBtn;
    QPushButton     goBtn;

    static  dHeader rawHeader;
    static  QString pixelFormatString;
};


#endif // QWAUXDIALOGS_H
