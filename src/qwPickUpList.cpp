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

#include "./inc/qwPickUpList.h"
#include "./inc/CImgContext.h"
#include "./inc/Threads.h"
#include "./inc/globals.h"
#include "./inc/qwDecoratedCanvas.h"

#ifdef QT4_HEADERS
    #include <QListWidget>
    #include <QMessageBox>
    #include <QInputDialog>
    #include <QLineEdit>
    #include <QMenu>
#elif QT5_HEADERS
    #include <QtWidgets/QListWidget>
    #include <QtWidgets/QMessageBox>
    #include <QtWidgets/QInputDialog>
    #include <QtWidgets/QLineEdit>
    #include <QtWidgets/QMenu>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
/*!
 * \brief qwPickUpList::qwPickUpList
 * \param parent
 */

qwPickUpList::qwPickUpList(QWidget* parent):QListWidget(parent)
{
    setMaximumHeight(1.8*UI_THUMBNAIL_SIZE);
    setIconSize(QSize(UI_THUMBNAIL_SIZE, UI_THUMBNAIL_SIZE));
    setViewMode(QListView::IconMode);
    setMovement(QListView::Static);
    setFlow(QListView::LeftToRight);
    setUniformItemSizes(true);
    setWrapping(false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);


    connect(this, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
            parent, SLOT(setCurrentImageSlot(QListWidgetItem*)));

    connect(this, SIGNAL(itemChanged(QListWidgetItem*)),
            this, SLOT(itemNameChanged(QListWidgetItem*)));

    connect(this, SIGNAL(saveImageAs()),
            parent, SLOT(saveImageAs()));

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(showContextMenu(const QPoint&)));

    myPopUpMenuPtr = QPointer<qwPopUpMenuThumbnailList>(new qwPopUpMenuThumbnailList(this));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
qwPickUpList::~qwPickUpList()
{
    hidePopUpMenu();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void qwPickUpList::showContextMenu(const QPoint &p)
{
    myPopUpMenuPtr->popup(viewport()->mapToGlobal(p));
}

void qwPickUpList::popupRename()
{
    QMutexLocker lock(&Globals::imgContextListLock);

    QSharedPointer<CImgContext> whichImagePtr = Globals::findImgContextByWidget(currentItem());
    if(whichImagePtr == NULL)
        return;

    if(whichImagePtr->getMyState() == STATE_BUSY)
        return;

    if(currentItem() != NULL)
        editItem(currentItem());
}

void qwPickUpList::popupSaveAs()
{
    Globals::imgContextListLock.lock();
    QSharedPointer<CImgContext> whichImagePtr = Globals::findImgContextByWidget(currentItem());
    Globals::imgContextListLock.unlock();

    if(whichImagePtr == NULL)
        return;

    if(whichImagePtr->getMyState() == STATE_BUSY)
        return;

    if(currentItem() != NULL)
       emit saveImageAs();
}

void qwPickUpList::popupDelete()
{
    QMutexLocker lock(&Globals::imgContextListLock);

    QSharedPointer<CImgContext> whichImagePtr = Globals::findImgContextByWidget(currentItem());
    if(whichImagePtr == NULL)
        return;

    if(whichImagePtr->getMyState() == STATE_BUSY)
        return;

    if(currentItem() != NULL)
      Globals::removeImage(whichImagePtr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void qwPickUpList::itemNameChanged(QListWidgetItem* itemEdited)
{
    QSharedPointer<CImgContext> imgCtx = Globals::findImgContextByWidget(itemEdited);

    if(imgCtx.isNull())
        return;

    if(imgCtx->getMyState() != STATE_READY)
    {
        itemEdited->setText(imgCtx->getMyName());
        return;
    }

    if(imgCtx->myThumbnail.dont_process_an_update_event)
        return;

    if(Globals::isValidName(itemEdited->text()))
    {
        imgCtx = Globals::findImgContextByWidget(itemEdited);
        if(imgCtx != NULL)
        {
            imgCtx->setMyName(itemEdited->text());

            if(itemEdited->flags()&Qt::ItemIsSelectable)
            {       
                Globals::addCmdToLocalQueue(CMD_REFRESH_THUMBNAILS_LIST);
            }
        }
    }
    else
    {
       showStatusMessage("The given name is not valid.", UI_STATUS_ERROR, true);
       //restore previous name
       imgCtx = Globals::findImgContextByWidget(itemEdited);
       if(imgCtx != NULL)
       {
         itemEdited->setText(imgCtx->getMyName());
       }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void qwPickUpList::focusInEvent(QFocusEvent*)
{
    (static_cast<qwDecoratedCanvas*>(parent()))->childSetFocus();
}

