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

#include "./inc/globals.h"
#include "./inc/defines.h"
#include "./inc/CImgContext.h"

#include <QMutex>

///////////////////////////////////////////////////////////////////////////////////////////////////
QMutex                                    Globals::commandQueueLock(QMutex::NonRecursive);
QVector<cmdS>                             Globals::commandQueue;
QMutex                                    Globals::statusMsgQueueLock(QMutex::NonRecursive);
QVector<statusBarMsg>                     Globals::statusMsgQueue;
QMainWindow*                              Globals::mainWindowPtr;
QApplication*                             Globals::myApp;
int                                       Globals::fontSizeMul                                            = 1;
QMutex                                    Globals::imgContextListLock(QMutex::NonRecursive);
QSharedPointer<CImgContext>               Globals::imgListHeadPtr;
QSemaphore                                Globals::processingThreadTrimmer(COM_MAX_PROCESSING_THREADS);
quint32                                   Globals::imgCount                                               = 0;
quint32                                   Globals::imgCountAbs                                            = 0;
quint32                                   Globals::imgCountLimit                                          = 10;
qint32                                    Globals::option_colorBase                                       = 10;
quint16                                   Globals::serverPort                                             = COM_DEFAULT_PORT;
quint32                                   Globals::idleSocketTimeoutInSecs                                = COM_TIMEOUT_SEC;
panelID                                   Globals::activePanel                                            = panelLeftTop;
QLabel*                                   Globals::statusBarPtr                                           = NULL;
bool                                      Globals::imageRecEnabled                                        = true;
bool                                      Globals::autoScaleOnLoad                                        = true;
bool                                      Globals::sharedViewFlagsEnabled                                 = false;
bool                                      Globals::sharedPositionEnabled                                  = false;
bool                                      Globals::sharedZoomEnabled                                      = false;
int                                       Globals::sharedPosition[2]                                      = {0, 0};
float                                     Globals::sharedZoom                                             = 1.0f;
quint32                                   Globals::sharedViewFlags                                        = 0x81E;
quint8                                    Globals::sharedMul[3]                                           = {1, 1, 1};
quint8                                    Globals::sharedDiv[3]                                           = {1, 1, 1};
quint8                                    Globals::sharedBias[3]                                          = {1, 1, 1};
QMap<int, QSharedPointer<CImgContext> >   Globals::sharedPointerHooks;
QMutex                                    Globals::toolThreadSlot;


///////////////////////////////////////////////////////////////////////////////////////////////////
void Globals::addImage(const QSharedPointer<CImgContext> &newImage)
{
    Q_ASSERT(newImage);

    imgCount++;
    imgCountAbs++;

    if(imgCount > imgCountLimit)
        removeLastImage();

    if(imgListHeadPtr.isNull())
    {
        imgListHeadPtr = newImage;
    }
    else
    {
        imgListHeadPtr->setPrevPtr(newImage);
        newImage->setNextPtr(imgListHeadPtr);
        imgListHeadPtr = newImage;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Globals::removeImage(QSharedPointer<CImgContext> &anImage)
{

    if(imgCount==0)
           return;

    if(anImage->pendingFlag(PENDING_FLAG_UNDEFINED) == PENDING_FLAG_MARKED_FOR_DELETION)
        return;

    anImage->pendingFlag(PENDING_FLAG_MARKED_FOR_DELETION, -1);
    imgCount--;
    if(anImage == imgListHeadPtr)
        imgListHeadPtr = anImage->getNextPtr();
    if(!anImage->getPrevPtr().isNull())
        anImage->getPrevPtr()->setNextPtr(anImage->getNextPtr());
    if(!anImage->getNextPtr().isNull())
        anImage->getNextPtr()->setPrevPtr(anImage->getPrevPtr());

    anImage->setPrevPtr(QSharedPointer<CImgContext>());
    anImage->setNextPtr(QSharedPointer<CImgContext>());

    Globals::addCmdToLocalQueue(CMD_REFRESH_THUMBNAILS_LIST); //Cannot be invoked from local queue executor!
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Globals::removeAll(QSharedPointer<CImgContext> &whereToStart)
{
    QSharedPointer<CImgContext> nextToDie, current;

    current = whereToStart;
    if(current.isNull()) return;

    nextToDie = current->getNextPtr();

    while(current)
    {
        nextToDie = current->getNextPtr();
        current->setNextPtr(QSharedPointer<CImgContext>());
        current->setPrevPtr(QSharedPointer<CImgContext>());
        current->pendingFlag(PENDING_FLAG_MARKED_FOR_DELETION, -1);
        current = nextToDie;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool Globals::isValidName(QString fileName)
{
    QRegExp res_pattern = QRegExp(" *[\\~#%&*{}/:<>?|\"-]+ *");

    if(fileName.size()==0) return false;
    return (res_pattern.indexIn(fileName)>=0)?false:true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QSharedPointer<CImgContext> Globals::findImgContextByWidget(QListWidgetItem* widgetItem)
{
    QSharedPointer<CImgContext> next = Globals::imgListHeadPtr;

    if(widgetItem == NULL)
        return QSharedPointer<CImgContext>();

    while(!next.isNull())
    {
        if(next == (CImgContext*)(((CThumbnail*)widgetItem)->getMyParent()))
          break;
        next = next->getNextPtr();
    }

    if(next != NULL)
        if(next == Globals::imgListHeadPtr)
            if(next != (CImgContext*)(((CThumbnail*)widgetItem)->getMyParent()))
                next = QSharedPointer<CImgContext>();
    return next;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void Globals::addCmdToLocalQueue(int cmdID, int auxParam)
{
    cmdS c1;
    c1.commandID = cmdID;
    c1.auxParam = auxParam;
    Globals::commandQueueLock.lock();
    Globals::commandQueue.append(c1);
    Globals::commandQueueLock.unlock();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void Globals::addCmdToLocalQueue(int cmdID, const QSharedPointer<CImgContext> &auxParam)
{
    Globals::sharedPointerHooks.insertMulti((quintptr)auxParam.data(), QSharedPointer<CImgContext>(auxParam));
    cmdS c1;
    c1.commandID = cmdID;
    c1.auxParam = (quintptr)auxParam.data();
    Globals::commandQueueLock.lock();
    Globals::commandQueue.append(c1);
    Globals::commandQueueLock.unlock();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void Globals::removeLastImage()
{
    QSharedPointer<CImgContext> next, current;

    current = Globals::imgListHeadPtr;
    if(current.isNull()) return;

    next = current->getNextPtr();

    while(!next.isNull())
    {
        current = next;
        next = next->getNextPtr();
    }
    removeImage(current);
}
