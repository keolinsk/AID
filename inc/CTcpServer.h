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

#ifndef CTCPSERVER_H
#define CTCPSERVER_H

#include "Threads.h"
#include "globals.h"
#include "commons.h"

#include <QObject>
#include <QMutex>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QtNetwork>

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/*!
 * \brief The CSimpleDataContainer class
 * \section DESCRIPTION
 *          This class implements a basic communication mechanism based on the TCP/IP soceket connection.
 */

class CSimpleDataContainer
{
public:

    CSimpleDataContainer()
    {
        currentSize = 0;
        cursor = 0;
    }

    /*!
     * \brief Initialization routine.
     * \param initSize Initial size of the cache buffer.
     */
    int init(ulong initSize)
    {
        ptrData = (char*)malloc(initSize);
        if(!ptrData)
        {
           // Q_ASSERT(ptrData);
            return RES_ERROR;
        }
        currentSize = initSize;
        cursor = 0;
        return RES_OK;
    }

    /*!
     * \brief addData Method for adding cosecutive data chunks received from a client.
     * \param buffPtr Data chunk buffer pointer.
     * \param size    Size of the data chunk.
     */
    int addData(const char* buffPtr, ulong size)
    {
        if(cursor+size > currentSize)
        {
            ptrData = (char*)realloc(ptrData, cursor+size);
            if(!ptrData)
                return RES_ERROR;
           // Q_ASSERT(ptrData);
            currentSize = cursor+size;
        }

        memcpy(ptrData+cursor, buffPtr, size);
        cursor += size;
        return RES_OK;
    }

    char* resizeAndGetBufferPointer(ulong appendSize)
    {
        if(cursor+appendSize > currentSize)
        {
            ptrData = (char*)realloc(ptrData, cursor+appendSize);
            if(!ptrData)
            {
               // Q_ASSERT(ptrData);
                return NULL;
            }
            currentSize = cursor+appendSize;
        }

        char* resPtr = ptrData+cursor;
        cursor += appendSize;
        return resPtr;
    }

    ~CSimpleDataContainer()
    {
        //CNativeData takes ownership od the data buffer.
    }

    char* getDataPtr(){return ptrData;}
    ulong getCursor(){return cursor;}
    ulong getAllocatedSpace(){return currentSize;}

private:
    char* ptrData;
    ulong cursor;
    ulong currentSize;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
/*!
 * \brief The CSocketService class.
 * \section DESCRIPTION
 *          This class implements a TCP/IP server object used to client connection management.
 */

class CSocketService : public QObject
{
    Q_OBJECT

public:

    explicit CSocketService(QTcpSocket *, QObject *parentPtr);
    ~CSocketService();

    /*!
     * \brief poolClientState Connection timeout monitoring routine.
     * \param decrementCounter If yes an internal timeout counter is decremented.
     * \return Returns a timeout counter value.
     */
    int     poolClientState(bool decrementCounter);

public slots:
    void finishRead();
    void dataReceived(bool dataCache = true);
    void iAmDone();
    void deleteSocket();

private:

    QTcpSocket *mySocketPtr;

    CSimpleDataContainer inBuff;

    quint8      partID(const QByteArray &qba);
    void        finalize();
    int         wcounter;
    QByteArray  tmpqa;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
class CTcpServer : public QObject
{
    Q_OBJECT

    /* The port number is defined by [Globals.serverPort]. */

public:

    CTcpServer(QObject * parent = 0);
    void delayedInit();
   ~CTcpServer();
    void restart();
    void closeMe(){server.close();}
    bool isWorking(){return server.isListening();}

signals:
    void setActiveStateForStartStopButton(bool);

public slots:
    void acceptConnection();
    void processClientQueue(bool triggeredByTimer=false);

protected:
    void timerEvent(QTimerEvent*);

private:
    QTcpServer  server;
    QMutex      myMutex;

    QList<CSocketService*> clientsList;
};


#endif // CTCPSERVER_H
