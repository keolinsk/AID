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

#include "./inc/qwBottomPanel.h"
#include "./inc/globals.h"
#include "./inc/defines.h"

#include <QFont>

#ifdef QT4_HEADERS
    #include <QPushButton>
    #include <QSlider>
    #include <QVBoxLayout>
    #include <QHBoxLayout>
    #include <QSizePolicy>
#elif QT5_HEADERS
    #include <QtWidgets/QPushButton>
    #include <QtWidgets/QSlider>
    #include <QtWidgets/QVBoxLayout>
    #include <QtWidgets/QHBoxLayout>
    #include <QtWidgets/QSizePolicy>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
qwBottomPanel::qwBottomPanel(QWidget* parent): QWidget(parent)
{

    myLayoutPtr   = new QVBoxLayout(this);
    bottomRowPtr  = new QHBoxLayout();

    topRowPtr     = new QVBoxLayout();

    infoBarString.setText("[Empty view]");


    rgbaSwitchButtonPtr[0]     = new qwSWButton(QIcon(":/icos/R.png"), parent, UI_SWBUTTON_R_CHANNEL);
    rgbaSwitchButtonPtr[1]     = new qwSWButton(QIcon(":/icos/G.png"), parent, UI_SWBUTTON_G_CHANNEL);
    rgbaSwitchButtonPtr[2]     = new qwSWButton(QIcon(":/icos/B.png"), parent, UI_SWBUTTON_B_CHANNEL);
    rgbaSwitchButtonPtr[3]     = new qwSWButton(QIcon(":/icos/A.png"), parent, UI_SWBUTTON_A_CHANNEL);
    redBlueButtonPtr           = new qwSWButton(QIcon(":/icos/RedBlueSwap.png"), parent, UI_SWBUTTON_RB_SWAP);
    redGreenButtonPtr          = new qwSWButton(QIcon(":/icos/RedGreenSwap.png"), parent, UI_SWBUTTON_RG_SWAP);
    linearTransformButtonPtr   = new qwSWButton(QIcon(":/icos/linearTransform.png"), parent, UI_MENUBUTTON_LINEAR_TRANSFORM);
    verticalFlipButtonPtr      = new qwSWButton(QIcon(":/icos/verticalFlip.png"), parent, UI_SWBUTTON_VERTICAL_FLIP);
    horizontalFlipButtonPtr    = new qwSWButton(QIcon(":/icos/horizontalFlip.png"), parent, UI_SWBUTTON_HORIZONTAL_FLIP);
    snapToGridButtonPtr        = new qwSWButton(QIcon(":/icos/snapToGrid.png"), parent, UI_SWBUTTON_SNAP_TO_GRID);
    zoomInButtonPtr            = new qwSWButton(QIcon(":/icos/zoomIn.png"), parent, UI_BUTTON_ZOOMIN);
    zoomOutButtonPtr           = new qwSWButton(QIcon(":/icos/zoomOut.png"), parent, UI_BUTTON_ZOOMOUT);
    zoomAutoButtonPtr          = new qwSWButton(QIcon(":/icos/zoomAuto.png"), parent, UI_BUTTON_ZOOMAUTO);
    backgroundButtonPtr        = new qwSWButton(QIcon(":/icos/bckga.png"), parent, UI_SWBUTTON_BACKGROUND);
    thumbnailsBarButtonPtr     = new qwSWButton(QIcon(":/icos/thumbnailBar.png"), parent, UI_SWBUTTON_THUMBNAILSBAR);

    notesDataButtonPtr         = new QPushButton(QIcon(":/icos/notesData.png"),"");

    deleteImageButtonPtr       = new QPushButton(QIcon(":/icos/trash.png"),"");


    rgbaSwitchButtonPtr[0]->setToolTip(TIP_MAIN_CHANNEL_SW);
    rgbaSwitchButtonPtr[1]->setToolTip(TIP_MAIN_CHANNEL_SW);
    rgbaSwitchButtonPtr[2]->setToolTip(TIP_MAIN_CHANNEL_SW);
    rgbaSwitchButtonPtr[3]->setToolTip(TIP_MAIN_CHANNEL_SW);
    redBlueButtonPtr->setToolTip(TIP_MAIN_RBCHANNEL_SWAP);
    redGreenButtonPtr->setToolTip(TIP_MAIN_RGCHANNEL_SWAP);
    linearTransformButtonPtr->setToolTip(TIP_MAIN_LINEAR_FILTER);
    verticalFlipButtonPtr->setToolTip(TIP_MAIN_FLIP_V);
    horizontalFlipButtonPtr->setToolTip(TIP_MAIN_FLIP_H);
    snapToGridButtonPtr->setToolTip(TIP_MAIN_SNAP_TO_GRID);
    zoomInButtonPtr->setToolTip(TIP_MAIN_ZOOM_IN);
    zoomOutButtonPtr->setToolTip(TIP_MAIN_ZOOM_OUT);
    zoomAutoButtonPtr->setToolTip(TIP_MAIN_ZOOM_AUTO);
    backgroundButtonPtr->setToolTip(TIP_MAIN_BCKGROUND_MODE);
    thumbnailsBarButtonPtr->setToolTip(TIP_MAIN_SHOW_IPPBAR);
    notesDataButtonPtr->setToolTip(TIP_MAIN_IMAGE_NOTES);
    deleteImageButtonPtr->setToolTip(TIP_MAIN_DELETE);

    activePanelIndicator.setFrameShape(QFrame::HLine);
    activePanelIndicator.setFrameShadow(QFrame::Raised);
    activePanelIndicator.setLineWidth(UI_ACTIVE_PANEL_INDICATOR_SIZE);
    topRowPtr->addWidget(&activePanelIndicator);
    activePanelIndicator.setVisible(false);

    myLayoutPtr->setSpacing(0);
    myLayoutPtr->setContentsMargins(0, 1, 0, 0);
    bottomRowPtr->setSpacing(0);
    bottomRowPtr->setContentsMargins(0, 0, 0, 0);

    myFont.setFamily("Courier New");
    myFont.setPointSize(UI_INFO_FONT_SIZE*Globals::fontSizeMul/2);
    infoBarString.setFont(myFont);
    topRowPtr->addWidget(&infoBarString);

    linearTransformDialogPtr = new qwDialogLT(linearTransformButtonPtr, &linearTransformDialogMenu);
    linearTransformDialogMenuActionPtr = new QWidgetAction(this);
    linearTransformDialogMenuActionPtr->setDefaultWidget(linearTransformDialogPtr);
    linearTransformDialogMenu.addAction(linearTransformDialogMenuActionPtr);
    linearTransformDialogMenu.setAttribute(Qt::WA_TransparentForMouseEvents);

    rgbaSwitchButtonPtr[0]->setFixedSize(UI_SWITCH_BUTTON_SIZE, UI_SWITCH_BUTTON_SIZE);
    rgbaSwitchButtonPtr[1]->setFixedSize(UI_SWITCH_BUTTON_SIZE, UI_SWITCH_BUTTON_SIZE);
    rgbaSwitchButtonPtr[2]->setFixedSize(UI_SWITCH_BUTTON_SIZE, UI_SWITCH_BUTTON_SIZE);
    rgbaSwitchButtonPtr[3]->setFixedSize(UI_SWITCH_BUTTON_SIZE, UI_SWITCH_BUTTON_SIZE);
    redBlueButtonPtr->setFixedSize(UI_SWITCH_BUTTON_SIZE, UI_SWITCH_BUTTON_SIZE);
    redGreenButtonPtr->setFixedSize(UI_SWITCH_BUTTON_SIZE, UI_SWITCH_BUTTON_SIZE);
    linearTransformButtonPtr->setFixedSize(UI_SWITCH_BUTTON_SIZE, UI_SWITCH_BUTTON_SIZE);
    linearTransformButtonPtr->setMenu(&linearTransformDialogMenu);
    verticalFlipButtonPtr->setFixedSize(UI_SWITCH_BUTTON_SIZE, UI_SWITCH_BUTTON_SIZE);
    horizontalFlipButtonPtr->setFixedSize(UI_SWITCH_BUTTON_SIZE, UI_SWITCH_BUTTON_SIZE);
    snapToGridButtonPtr->setFixedSize(UI_SWITCH_BUTTON_SIZE, UI_SWITCH_BUTTON_SIZE);
    notesDataButtonPtr->setFixedSize(UI_SWITCH_BUTTON_SIZE, UI_SWITCH_BUTTON_SIZE);
    zoomInButtonPtr->setFixedSize(UI_SWITCH_BUTTON_SIZE, UI_SWITCH_BUTTON_SIZE);
    zoomOutButtonPtr->setFixedSize(UI_SWITCH_BUTTON_SIZE, UI_SWITCH_BUTTON_SIZE);
    zoomAutoButtonPtr->setFixedSize(UI_SWITCH_BUTTON_SIZE, UI_SWITCH_BUTTON_SIZE);
    backgroundButtonPtr->setFixedSize(UI_SWITCH_BUTTON_SIZE, UI_SWITCH_BUTTON_SIZE);
    deleteImageButtonPtr->setFixedSize(UI_SWITCH_BUTTON_SIZE, UI_SWITCH_BUTTON_SIZE);
    thumbnailsBarButtonPtr->setFixedSize(UI_SWITCH_BUTTON_SIZE, UI_SWITCH_BUTTON_SIZE);


    bottomRowPtr->addWidget(rgbaSwitchButtonPtr[0], 0, Qt::AlignLeft);
    bottomRowPtr->addWidget(rgbaSwitchButtonPtr[1], 0, Qt::AlignLeft);
    bottomRowPtr->addWidget(rgbaSwitchButtonPtr[2], 0, Qt::AlignLeft);
    bottomRowPtr->addWidget(rgbaSwitchButtonPtr[3], 0, Qt::AlignLeft);
    bottomRowPtr->addSpacing(UI_BUTTON_SPACING_SMALL);
    bottomRowPtr->addWidget(redBlueButtonPtr, 0, Qt::AlignLeft);
    bottomRowPtr->addWidget(redGreenButtonPtr, 0, Qt::AlignLeft);
    bottomRowPtr->addWidget(linearTransformButtonPtr, 0, Qt::AlignLeft);
    bottomRowPtr->addSpacing(UI_BUTTON_SPACING_LARGE);
    bottomRowPtr->addWidget(zoomInButtonPtr, 0, Qt::AlignLeft);
    bottomRowPtr->addWidget(zoomOutButtonPtr, 0, Qt::AlignLeft);
    bottomRowPtr->addWidget(zoomAutoButtonPtr, 0, Qt::AlignLeft);
    bottomRowPtr->addWidget(verticalFlipButtonPtr, 0, Qt::AlignLeft);
    bottomRowPtr->addWidget(horizontalFlipButtonPtr, 0, Qt::AlignLeft);
    bottomRowPtr->addSpacing(UI_BUTTON_SPACING_LARGE);
    bottomRowPtr->addWidget(snapToGridButtonPtr, 0, Qt::AlignLeft);
    bottomRowPtr->addWidget(backgroundButtonPtr, 0, Qt::AlignLeft);
    bottomRowPtr->addSpacing(UI_BUTTON_SPACING_LARGE);
    bottomRowPtr->addWidget(notesDataButtonPtr, 0, Qt::AlignLeft);
    bottomRowPtr->insertStretch(-1);
    bottomRowPtr->addWidget(deleteImageButtonPtr, 0, Qt::AlignRight);
    bottomRowPtr->addSpacing(UI_BUTTON_SPACING_SMALL);
    bottomRowPtr->addWidget(thumbnailsBarButtonPtr, 0, Qt::AlignRight);
    bottomRowPtr->addSpacing(UI_BUTTON_SPACING_SMALL);

    myLayoutPtr->addLayout(topRowPtr);
    myLayoutPtr->addLayout(bottomRowPtr);

    connect(notesDataButtonPtr, SIGNAL(clicked()), this, SLOT(showNoteBox()));
    connect(deleteImageButtonPtr, SIGNAL(clicked()), parent, SLOT(deleteCurrentImage()));

    setLayout(myLayoutPtr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
qwBottomPanel::~qwBottomPanel()
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////
void qwBottomPanel::showNoteBox()
{
    myNoteBox.showIfNecessary();
}

