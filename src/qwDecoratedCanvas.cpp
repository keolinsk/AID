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

#include "./inc/qwDecoratedCanvas.h"
#include "./inc/qwGridCanvas.h"
#include "./inc/globals.h"
#include "./inc/commons.h"
#include "./inc/defines.h"
#include "./inc/CImgContext.h"
#include "./inc/aidMainWindow.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
qwDecoratedCanvas::qwDecoratedCanvas(int defSelBvr, panelID myID, QWidget *parent): QWidget(parent), defaultSelectionBehavior(defSelBvr)
{

   this->myID = myID;

   myGridCanvasPtr  = new qwGridCanvas(this);
   myBottomPanelPtr = new qwBottomPanel(this);
   myThumbnailListPtr = new qwPickUpList(this);

   setMinimumWidth(UI_MIN_PANEL_WIDTH);
   setMinimumHeight(UI_MIN_PANEL_HEIGHT);

   myLayout.setSpacing(0);
   myLayout.setContentsMargins(0, 0, 0, 0);

   myGridCanvasPtr->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
   myBottomPanelPtr->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
   myThumbnailListPtr->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
   myThumbnailListPtr->clear();
   myThumbnailListPtr->setContentsMargins(0, 0, 0, 0);

   myBottomPanelPtr->thumbnailsBarButtonPtr->setChecked(true);

   myLayout.addWidget(myGridCanvasPtr);
   myLayout.addWidget(myBottomPanelPtr);
   myLayout.addWidget(myThumbnailListPtr, 0, Qt::AlignBottom);
   setLayout(&myLayout);

   setFocusPolicy(Qt::ClickFocus);

   if(myID == panelLeftTop)
       focusInEvent(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void qwDecoratedCanvas::enableButtons()
{
    myBottomPanelPtr->rgbaSwitchButtonPtr[0]->setEnabled(true);
    myBottomPanelPtr->rgbaSwitchButtonPtr[1]->setEnabled(true);
    myBottomPanelPtr->rgbaSwitchButtonPtr[2]->setEnabled(true);
    myBottomPanelPtr->rgbaSwitchButtonPtr[3]->setEnabled(true);

    myBottomPanelPtr->redBlueButtonPtr->setEnabled(true);
    myBottomPanelPtr->redGreenButtonPtr->setEnabled(true);
    myBottomPanelPtr->linearTransformButtonPtr->setEnabled(true);
    myBottomPanelPtr->verticalFlipButtonPtr->setEnabled(true);
    myBottomPanelPtr->horizontalFlipButtonPtr->setEnabled(true);
    myBottomPanelPtr->snapToGridButtonPtr->setEnabled(true);
    myBottomPanelPtr->backgroundButtonPtr->setEnabled(true);
    myBottomPanelPtr->zoomAutoButtonPtr->setEnabled(true);
    myBottomPanelPtr->zoomInButtonPtr->setEnabled(true);
    myBottomPanelPtr->zoomOutButtonPtr->setEnabled(true);
    myBottomPanelPtr->notesDataButtonPtr->setEnabled(true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void qwDecoratedCanvas::disableButtons()
{
    myBottomPanelPtr->rgbaSwitchButtonPtr[0]->setDisabled(true);
    myBottomPanelPtr->rgbaSwitchButtonPtr[1]->setDisabled(true);
    myBottomPanelPtr->rgbaSwitchButtonPtr[2]->setDisabled(true);
    myBottomPanelPtr->rgbaSwitchButtonPtr[3]->setDisabled(true);

    myBottomPanelPtr->redBlueButtonPtr->setDisabled(true);
    myBottomPanelPtr->redGreenButtonPtr->setDisabled(true);
    myBottomPanelPtr->linearTransformButtonPtr->setDisabled(true);
    myBottomPanelPtr->verticalFlipButtonPtr->setDisabled(true);
    myBottomPanelPtr->horizontalFlipButtonPtr->setDisabled(true);
    myBottomPanelPtr->snapToGridButtonPtr->setDisabled(true);
    myBottomPanelPtr->backgroundButtonPtr->setDisabled(true);
    myBottomPanelPtr->zoomAutoButtonPtr->setDisabled(true);
    myBottomPanelPtr->zoomInButtonPtr->setDisabled(true);
    myBottomPanelPtr->zoomOutButtonPtr->setDisabled(true);
    myBottomPanelPtr->notesDataButtonPtr->setDisabled(true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void qwDecoratedCanvas::restoreSWButtonsState()
{
    if(currentImgPtr.isNull())
    {
        disableButtons();
        return;
    }

    if(currentImgPtr->getMyState() != STATE_READY)
    {
        disableButtons();
        if(currentImgPtr->getMyState())
        {
            if(currentImgPtr->myNotes.size()>0)
                myBottomPanelPtr->notesDataButtonPtr->setDisabled(false);
        }
        return;
    }

    myBottomPanelPtr->rgbaSwitchButtonPtr[0]->setChecked(currentImgPtr->getFlag(IMGCX_RED_check));
    myBottomPanelPtr->rgbaSwitchButtonPtr[1]->setChecked(currentImgPtr->getFlag(IMGCX_GREEN_check));
    myBottomPanelPtr->rgbaSwitchButtonPtr[2]->setChecked(currentImgPtr->getFlag(IMGCX_BLUE_check));
    myBottomPanelPtr->rgbaSwitchButtonPtr[3]->setChecked(currentImgPtr->getFlag(IMGCX_ALPHA_check));
    myBottomPanelPtr->redBlueButtonPtr->setChecked(currentImgPtr->getFlag(IMGCX_SWAPPED_REDBLUE_check));
    myBottomPanelPtr->redGreenButtonPtr->setChecked(currentImgPtr->getFlag(IMGCX_SWAPPED_REDGREEN_check));
    myBottomPanelPtr->linearTransformButtonPtr->setChecked(currentImgPtr->getFlag(IMGCX_LINEAR_TRANSFORM_check));
    myBottomPanelPtr->verticalFlipButtonPtr->setChecked(currentImgPtr->getFlag(IMGCX_VERTICAL_FLIP_check));
    myBottomPanelPtr->horizontalFlipButtonPtr->setChecked(currentImgPtr->getFlag(IMGCX_HORIZONTAL_FLIP_check));
    myBottomPanelPtr->snapToGridButtonPtr->setChecked(currentImgPtr->getFlag(IMGCX_SNAP_TO_GRID_check));
    myBottomPanelPtr->backgroundButtonPtr->setChecked(currentImgPtr->getFlag(IMGCX_TRANSPARENT_BKG_check));

    enableButtons();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void qwDecoratedCanvas::setCurrentImage(const QSharedPointer<CImgContext> &_ptr)
{

   if((_ptr == currentImgPtr)&&(!_ptr.isNull()))
       if((currentImgPtr->getActivePanel(myID)))
          return;

   myGridCanvasPtr->refreshView("Fetching...");

   myBottomPanelPtr->linearTransformDialogPtr->closeMe();

   if(!_ptr.isNull())
       _ptr->SetFocus(myID);

   if(_ptr != currentImgPtr)
      if(!currentImgPtr.isNull())
          currentImgPtr->LostFocus(myID);


   currentImgPtr = _ptr;


   if(_ptr.isNull())
   {
       myGridCanvasPtr->refreshView("Empty view");
       goto __EXIT_POINT;
   }



  // if(myID == panelLeftTop)
   {
        if(Globals::sharedViewFlagsEnabled)
        {
            bool needRefresh = false;

            if(currentImgPtr->getMul(R) != Globals::sharedMul[0]){ currentImgPtr->setMul(R,  Globals::sharedMul[0]); needRefresh = true;}
            if(currentImgPtr->getDiv(R) !=  Globals::sharedDiv[0]){ currentImgPtr->setDiv(R,  Globals::sharedDiv[0]); needRefresh = true;}
            if(currentImgPtr->getBias(R) !=  Globals::sharedBias[0]){ currentImgPtr->setBias(R,  Globals::sharedBias[0]); needRefresh = true;}

            if(currentImgPtr->getMul(G) !=  Globals::sharedMul[1]){ currentImgPtr->setMul(G,  Globals::sharedMul[1]); needRefresh = true;}
            if(currentImgPtr->getDiv(G) !=  Globals::sharedDiv[1]){ currentImgPtr->setDiv(G,  Globals::sharedDiv[1]); needRefresh = true;}
            if(currentImgPtr->getBias(G) !=  Globals::sharedBias[1]){currentImgPtr->setBias(G,  Globals::sharedBias[1]); needRefresh = true;}

            if(currentImgPtr->getMul(B) !=  Globals::sharedMul[2]){ currentImgPtr->setMul(B,  Globals::sharedMul[2]); needRefresh = true;}
            if(currentImgPtr->getDiv(B) !=  Globals::sharedDiv[2]){ currentImgPtr->setDiv(B,  Globals::sharedDiv[2]); needRefresh = true;}
            if(currentImgPtr->getBias(B) !=  Globals::sharedBias[2]){ currentImgPtr->setBias(B,  Globals::sharedBias[2]); needRefresh = true;}

            if(currentImgPtr->getFlags() !=  Globals::sharedViewFlags){ currentImgPtr->setFlags( Globals::sharedViewFlags); needRefresh = true;}
            if(needRefresh){currentImgPtr->applyOnTheFlyFilters(); currentImgPtr->applyPreFilters();}
        };


       if(Globals::sharedZoomEnabled)
       {
          currentImgPtr->setZoomFactor(Globals::sharedZoom);
       }

       if(Globals::sharedPositionEnabled)
       {
          currentImgPtr->setImgOffset(axX,  Globals::sharedPosition[0]);
          currentImgPtr->setImgOffset(axY,  Globals::sharedPosition[1]);
       }

   }

   myBottomPanelPtr->linearTransformDialogPtr->setRParams(currentImgPtr->getMul(R),
                                                          currentImgPtr->getDiv(R),
                                                          currentImgPtr->getBias(R));
   myBottomPanelPtr->linearTransformDialogPtr->setGParams(currentImgPtr->getMul(G),
                                                          currentImgPtr->getDiv(G),
                                                          currentImgPtr->getBias(G));
   myBottomPanelPtr->linearTransformDialogPtr->setBParams(currentImgPtr->getMul(B),
                                                          currentImgPtr->getDiv(B),
                                                          currentImgPtr->getBias(B));


__EXIT_POINT:
   myBottomPanelPtr->myNoteBox.setCurrentImage(currentImgPtr);
   myGridCanvasPtr->setImgContext(currentImgPtr);
   myGridCanvasPtr->refreshView();
   restoreSWButtonsState();
   refreshInfoBar();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void qwDecoratedCanvas::refreshThumbnailsList(QSharedPointer<CImgContext> header)
{
    QSharedPointer<CImgContext> next;
    CThumbnail  *tmpPtr = NULL, *currentPtr = NULL;
    int         currentRow = 0;

    currentRow = max(0, myThumbnailListPtr->currentRow());
    myThumbnailListPtr->hidePopUpMenu();
    myThumbnailListPtr->blockSignals(true);
    myThumbnailListPtr->clearSelection();
    myThumbnailListPtr->clear();
    myThumbnailListPtr->blockSignals(false);


    if(header.isNull())
    {
       setCurrentImageSlot(NULL);
       return;
    }

    if(header->need_thumbnail_refresh)
        header->makeThumbnail();

    tmpPtr = header->myThumbnail.clone();
    if(header == currentImgPtr)
        currentPtr = tmpPtr;

    myThumbnailListPtr->addItem(reinterpret_cast<QListWidgetItem*>(tmpPtr));

    next = header->getNextPtr();
    while((next != header) && (!next.isNull()))
    {
        if(next->need_thumbnail_refresh)
            next->makeThumbnail();

        tmpPtr = next->myThumbnail.clone();
        if(next == currentImgPtr)
            currentPtr = tmpPtr;

        myThumbnailListPtr->addItem(reinterpret_cast<QListWidgetItem*>(tmpPtr));
        next = next->getNextPtr();
    }

    if((currentRow>0)&&(currentPtr))
        myThumbnailListPtr->setCurrentItem(currentPtr);
    else
        myThumbnailListPtr->setCurrentRow(0);

    setCurrentImageSlot(myThumbnailListPtr->currentItem());

}

////////////////////////////////////////////////////////////////////////////////////////////////////
qwDecoratedCanvas::~qwDecoratedCanvas()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////
void qwDecoratedCanvas::setCurrentImageSlot(QListWidgetItem* imc)
{
     setCurrentImage(Globals::findImgContextByWidget(imc));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void qwDecoratedCanvas::reciveSWButtonEvent(quint32 btnID, qint32 state)
{
    quint8 pmul, pdiv, pbias;

    if(btnID == UI_SWBUTTON_THUMBNAILSBAR)
    {
        myThumbnailListPtr->setVisible((state)?true:false);
        return;
    }

    if(currentImgPtr.isNull())
        return;

    if(currentImgPtr->getMyState() != STATE_READY)
        return;

    switch(btnID)
    {
        case UI_SWBUTTON_R_CHANNEL:           currentImgPtr->setFlag(IMGCX_RED_check,(state)?true:false);                setSharedViewFlag(IMGCX_RED_check,(state)?true:false);               currentImgPtr->applyOnTheFlyFilters();          break;
        case UI_SWBUTTON_G_CHANNEL:           currentImgPtr->setFlag(IMGCX_GREEN_check,(state)?true:false);              setSharedViewFlag(IMGCX_GREEN_check,(state)?true:false);             currentImgPtr->applyOnTheFlyFilters();          break;
        case UI_SWBUTTON_B_CHANNEL:           currentImgPtr->setFlag(IMGCX_BLUE_check,(state)?true:false);               setSharedViewFlag(IMGCX_BLUE_check,(state)?true:false);              currentImgPtr->applyOnTheFlyFilters();          break;
        case UI_SWBUTTON_A_CHANNEL:           currentImgPtr->setFlag(IMGCX_ALPHA_check,(state)?true:false);              setSharedViewFlag(IMGCX_ALPHA_check,(state)?true:false);             if(currentImgPtr->hasAlpha())
                                                                                                                                                                                                  currentImgPtr->applyPreFilters();           break;                                                                                                                                     break;
        case UI_SWBUTTON_RB_SWAP:             currentImgPtr->setFlag(IMGCX_SWAPPED_REDBLUE_check,(state)?true:false);    setSharedViewFlag(IMGCX_SWAPPED_REDBLUE_check,(state)?true:false);   currentImgPtr->applyPreFilters();               break;
        case UI_SWBUTTON_RG_SWAP:             currentImgPtr->setFlag(IMGCX_SWAPPED_REDGREEN_check,(state)?true:false);   setSharedViewFlag(IMGCX_SWAPPED_REDGREEN_check,(state)?true:false);  currentImgPtr->applyPreFilters();               break;
        case UI_MENUBUTTON_LINEAR_TRANSFORM:  currentImgPtr->setFlag(IMGCX_LINEAR_TRANSFORM_check,(state)?true:false);   setSharedViewFlag(IMGCX_LINEAR_TRANSFORM_check,(state)?true:false);
                                              myBottomPanelPtr->linearTransformDialogPtr->getRParams(pmul, pdiv, pbias);
                                              currentImgPtr->setMul(R, pmul);
                                              currentImgPtr->setDiv(R, pdiv);
                                              currentImgPtr->setBias(R, pbias);
                                              myBottomPanelPtr->linearTransformDialogPtr->getGParams(pmul, pdiv, pbias);
                                              currentImgPtr->setMul(G, pmul);
                                              currentImgPtr->setDiv(G, pdiv);
                                              currentImgPtr->setBias(G, pbias);
                                              myBottomPanelPtr->linearTransformDialogPtr->getBParams(pmul, pdiv, pbias);
                                              currentImgPtr->setMul(B, pmul);
                                              currentImgPtr->setDiv(B, pdiv);
                                              currentImgPtr->setBias(B, pbias);
                                              currentImgPtr->applyPreFilters();                                                                                                                                                               break;

                                                                                                                                                                                                                                              break;
        case UI_SWBUTTON_VERTICAL_FLIP:       currentImgPtr->setFlag(IMGCX_VERTICAL_FLIP_check,(state)?true:false);     setSharedViewFlag(IMGCX_VERTICAL_FLIP_check,(state)?true:false);      currentImgPtr->applyPreFilters();               break;
        case UI_SWBUTTON_HORIZONTAL_FLIP:     currentImgPtr->setFlag(IMGCX_HORIZONTAL_FLIP_check,(state)?true:false);   setSharedViewFlag(IMGCX_HORIZONTAL_FLIP_check,(state)?true:false);    currentImgPtr->applyPreFilters();               break;
        case UI_SWBUTTON_SNAP_TO_GRID:        currentImgPtr->setFlag(IMGCX_SNAP_TO_GRID_check,(state)?true:false);      setSharedViewFlag(IMGCX_SNAP_TO_GRID_check,(state)?true:false);                                                       break;
        case UI_SWBUTTON_BACKGROUND:          currentImgPtr->setFlag(IMGCX_TRANSPARENT_BKG_check,(state)?true:false);   setSharedViewFlag(IMGCX_TRANSPARENT_BKG_check,(state)?true:false);                                                    break;
        case UI_BUTTON_ZOOMIN:                myGridCanvasPtr->zoom(-1,-1,-1);                                                                                                                                                                break;
        case UI_BUTTON_ZOOMOUT:               myGridCanvasPtr->zoom(1,-1,-1);                                                                                                                                                                 break;
        case UI_BUTTON_ZOOMAUTO:              myGridCanvasPtr->zoom(0,-1,-1);                                                                                                                                                                 break;
    }


    myGridCanvasPtr->refreshView();
    refreshInfoBar();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void qwDecoratedCanvas::refreshInfoBar()
{
    QString tmpStr = "";

    if(currentImgPtr.isNull())
    {
        tmpStr += "[Empty]";
    }
    else
    {
        tmpStr += QString::number(currentImgPtr->getIWidth()) + "x" + QString::number(currentImgPtr->getIHeight()) + " ";
        tmpStr += "[" + currentImgPtr->myPixelFormat + "]";
        if(currentImgPtr->getSelectedCoords(axX)>-1)
        {
            tmpStr += " @";
            tmpStr += "(";
            tmpStr += QString::number(currentImgPtr->getSelectedCoords(axX));
            tmpStr += ", ";
            tmpStr += QString::number(currentImgPtr->getSelectedCoords(axY));
            tmpStr += ")";
            tmpStr += "   ZOOM: ";
            tmpStr += QString::number(currentImgPtr->getZoomFactor(), 'f', 4);
            tmpStr += "\n"+currentImgPtr->getPixelValueString(currentImgPtr->getSelectedCoords(axX), currentImgPtr->getSelectedCoords(axY), Globals::option_colorBase);
        }
        else
        {
            tmpStr += "   ZOOM: ";
            tmpStr += QString::number(currentImgPtr->getZoomFactor(), 'f', 4);
        }
    }

    myBottomPanelPtr->infoBarString.setText(tmpStr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void qwDecoratedCanvas::deleteCurrentImage()
{
    Globals::removeImage(currentImgPtr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void qwDecoratedCanvas::saveImageAs()
{
    reinterpret_cast<aidMainWindow*>(Globals::mainWindowPtr)->menuApp_SaveAs();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void qwDecoratedCanvas::setSharedViewFlag(quint32 bitToSet, bool value)
{
    if(value)
        Globals::sharedViewFlags |= bitToSet;
    else
        Globals::sharedViewFlags &= ~bitToSet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void qwDecoratedCanvas::hideEvent(QHideEvent *e)
{
    setCurrentImage(QSharedPointer<CImgContext>());
    QWidget::hideEvent(e);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void qwDecoratedCanvas::showEvent(QShowEvent *e)
{
    refreshThumbnailsList(Globals::imgListHeadPtr);
    refreshInfoBar();
    QWidget::showEvent(e);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void qwDecoratedCanvas::enterEvent(QEvent *e)
{
  myGridCanvasPtr->refreshView();
  restoreSWButtonsState();
  refreshInfoBar();
  QWidget::enterEvent(e);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void qwDecoratedCanvas::focusInEvent(QFocusEvent *e)
{
    myBottomPanelPtr->activePanelIndicator.setVisible(true);
    Globals::activePanel = myID;
    emit iHaveFocus();
    QWidget::focusInEvent(e);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void qwDecoratedCanvas::childLostFocus()
{
    myBottomPanelPtr->activePanelIndicator.setVisible(false);
}
