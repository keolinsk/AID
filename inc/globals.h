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

#ifndef GLOBAL_H
#define GLOBAL_H

#include "CImgContext.h"
#include <QMutex>
#include <QSemaphore>

#ifdef QT4_HEADERS
    #include <QApplication>
    #include <QMainWindow>
    #include <QLabel>
#elif QT5_HEADERS
    #include <QtWidgets/QApplication>
    #include <QtWidgets/QMainWindow>
    #include <QtWidgets/QLabel>
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////
/*!
 * Macro for adding a status message to the queue.
 */

#define showStatusMessage(msg, icon, instantShow)\
{\
    statusBarMsg m1;\
    m1.msgStr = msg;\
    m1.iconID = icon;\
    Globals::statusMsgQueueLock.lock();\
    if(Globals::statusMsgQueue.size()>=MSG_STATUS_QUEUE_MAX)\
        Globals::statusMsgQueue.erase(Globals::statusMsgQueue.begin());\
    if(instantShow)\
        Globals::statusMsgQueue.prepend(m1);\
    else\
        Globals::statusMsgQueue.append(m1);\
    Globals::statusMsgQueueLock.unlock();\
    if(instantShow)\
        Globals::addCmdToLocalQueue(CMD_ADD_STATUS_MESSAGE);\
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/*!
 * \brief The cmdS struct.
 *        An internal message-queue element structure.
 */

struct cmdS
{
    int      commandID;
    quintptr auxParam;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/*!
 * \brief The statusBarMsg struct.
 *        Structure coding message content displayed on the status bar.
 */

struct statusBarMsg
{
    int     iconID;
    QString msgStr;

};

///////////////////////////////////////////////////////////////////////////////////////////////////
class Globals
{
public:
    /*! The mutex for the local command queue. */
    static QMutex                                    commandQueueLock;
    /*! The local queue implementation. */
    static QVector<cmdS>                             commandQueue;

    /*! Thread count trimmer */
    static QSemaphore                                processingThreadTrimmer;

    /*! The status message queue. */
    static QMutex                                    statusMsgQueueLock;
    static QVector<statusBarMsg>                     statusMsgQueue;

    /*! Pointers for the main windows and the application object. */
    static QMainWindow                              *mainWindowPtr;
    static QApplication                             *myApp;

    /*! The mutex, the head pointer and aux counters for the loaded images list. */
    static QMutex                                    imgContextListLock;
    static QSharedPointer<CImgContext>               imgListHeadPtr;
    static quint32                                   imgCount;
    static quint32                                   imgCountAbs;
    static quint32                                   imgCountLimit;

    /*! The base for a color representation. */
    static qint32                                    option_colorBase;

    /*! TCP server port. */
    static quint16                                   serverPort;
    /*! Idle socket timeout */
    static quint32                                   idleSocketTimeoutInSecs;

    /*! Active panel. */
    static panelID                                   activePanel;

    /*! Status bar reinterpreted pointer. */
    static QLabel                                   *statusBarPtr;

    /*! Image receivement flag. */
    static bool                                      imageRecEnabled;
    static bool                                      autoScaleOnLoad;

    /*! Shared view options. */
    static bool                                      sharedViewFlagsEnabled;
    static bool                                      sharedPositionEnabled;
    static bool                                      sharedZoomEnabled;

    static int                                       sharedPosition[2];
    static float                                     sharedZoom;
    static quint32                                   sharedViewFlags;
    static quint8                                    sharedMul[3];
    static quint8                                    sharedDiv[3];
    static quint8                                    sharedBias[3];

    /*! Font size multiplier */
    static int                                       fontSizeMul;

    /*! Tool thread slot */
    static QMutex                                    toolThreadSlot;

    /*! Aux image context hooks. */
    static QMap<int, QSharedPointer<CImgContext> >   sharedPointerHooks;

    /*! Adds a new image to the loaded images list. */
    static void addImage(const QSharedPointer<CImgContext> &newImage);

    /*! Removes the image from the loaded images list. */
    static void removeImage(QSharedPointer<CImgContext> &anImage);

    /*! Removes the last image in the queue. */
    static void removeLastImage();

    /*! Removes all images from the loaded images list. */
    static void removeAll(QSharedPointer<CImgContext> &whereToStart);

    /*! Validates if <fileName> is a legal system filename. */
    static bool isValidName(QString fileName);

    /*! Finds a CImgContext object by its widget item. */
    static QSharedPointer<CImgContext> findImgContextByWidget(QListWidgetItem* itemEdited);

    /*! Adds a new entry to the local command queue. */
    static void addCmdToLocalQueue(int cmdID, int auxParam=0);
    static void addCmdToLocalQueue(int cmdID, const QSharedPointer<CImgContext> &auxParam);
};

#endif // GLOBAL_H
