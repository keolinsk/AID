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

#include "./inc/CTcpServer.h"
#include "./inc/defines.h"
#include "./inc/globals.h"
#include "./inc/aidMainWindow.h"

#include <QString>
#include <iostream>
#include <QScopedPointer>

using namespace std;


const int CLIENT_STATE_READING    =1;
const int CLIENT_STATE_KILL_ME    =0;
const int CLIENT_STATE_PROCESSING =-1;


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
CSocketService::CSocketService(QTcpSocket* socketPtr, QObject* parentPtr):QObject(parentPtr)
{
    mySocketPtr = socketPtr;

    //inBuff.reserve(sizeof(magichars) + sizeof(dHeader) + 16512);
    wcounter = Globals::idleSocketTimeoutInSecs*1000/COM_TIMER_INTERVAL_MS;

    connect(mySocketPtr, SIGNAL(readChannelFinished()), this,
            SLOT(finishRead()));

    connect(mySocketPtr, SIGNAL(readyRead()), this,
            SLOT(dataReceived()));

    mySocketPtr->write(COM_ACK_CHAR);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CSocketService::~CSocketService()
{
   //inBuff.clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CSocketService::finishRead()
{
    CWorker_loadFromNativeData* newWorker;

    if(wcounter <0)
        goto __EXIT_POINT;

    wcounter = -1;

    dataReceived(false);

    newWorker = new CWorker_loadFromNativeData(this, inBuff.getDataPtr(), inBuff.getCursor());
    newWorker->selfStart();

    goto __EXIT_POINT;


__EXIT_POINT:
    if(mySocketPtr->isOpen()){
        connect(mySocketPtr, SIGNAL(disconnected()),
            mySocketPtr, SLOT(deleteLater()));
        mySocketPtr->disconnectFromHost();
    }
    mySocketPtr=NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CSocketService::dataReceived(bool dataCache)
{
    int preSize;
    char* buffPtr;

    if(wcounter>0) wcounter = Globals::idleSocketTimeoutInSecs*1000/COM_TIMER_INTERVAL_MS;

    if(inBuff.getCursor() + mySocketPtr->bytesAvailable() > COM_MAX_DATA_SIZE)
    {
        goto __EXIT_WITH_OVERFLOW;
    }

    if((dataCache)&&(mySocketPtr->bytesAvailable()< COM_CACHE))
        return;

    if(inBuff.getAllocatedSpace()==0)
    {
        tmpqa = mySocketPtr->read(MAGIC_CHARS_SIZE + sizeof(dHeader));

        preSize = MAGIC_CHARS_SIZE + sizeof(dHeader);
        preSize += ((dHeader*)(tmpqa.data()+MAGIC_CHARS_SIZE))->formatStrLength;
        preSize += ((dHeader*)(tmpqa.data()+MAGIC_CHARS_SIZE))->nameLength;
        preSize += ((dHeader*)(tmpqa.data()+MAGIC_CHARS_SIZE))->notesLength;
        preSize += ((dHeader*)(tmpqa.data()+MAGIC_CHARS_SIZE))->sizeInBytes;

        if(preSize > COM_MAX_DATA_SIZE)
        {
            goto __EXIT_WITH_OVERFLOW;
        }

        if(inBuff.init(preSize) == RES_ERROR)
             goto __EXIT_WITH_OVERFLOW;

        inBuff.addData(tmpqa.data(), tmpqa.size());
    }

    buffPtr = inBuff.resizeAndGetBufferPointer(mySocketPtr->bytesAvailable());
    if(!buffPtr)
      goto __EXIT_WITH_OVERFLOW;

    mySocketPtr->read(buffPtr, mySocketPtr->bytesAvailable());

    goto __EXIT_POINT;

__EXIT_WITH_OVERFLOW:
   // Q_ASSERT_X(0, "CSocketService::dataReceived", "Out of memory!");
    mySocketPtr->disconnectFromHost();
    showStatusMessage("Image receiving aborted - improper received data size.", UI_STATUS_ERROR, true);

__EXIT_POINT:
    return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int  CSocketService::poolClientState(bool decrementCounter)
{
    if(mySocketPtr)
    {
        if(mySocketPtr->state()!=QAbstractSocket::ConnectedState)
            finishRead();
    }


    if(decrementCounter)
        if(wcounter>0)
        {    wcounter--;
             if(inBuff.getAllocatedSpace()==0) mySocketPtr->write(COM_ACK_CHAR);
        }

    if(wcounter==0)     return CLIENT_STATE_KILL_ME;
    if(wcounter>0)      return CLIENT_STATE_READING;
    if(wcounter==-1)    return CLIENT_STATE_PROCESSING;
    return CLIENT_STATE_KILL_ME;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CSocketService::iAmDone()
{
    wcounter = -2;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CSocketService::deleteSocket()
{
    if(mySocketPtr)
        mySocketPtr->deleteLater();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
CTcpServer::CTcpServer(QObject* parent): QObject(parent)
{
  server.setMaxPendingConnections(COM_MAX_PENDING_CONNECTIONS);
  startTimer(COM_TIMER_INTERVAL_MS);
}


void CTcpServer::delayedInit()
{
    connect(this, SIGNAL(setActiveStateForStartStopButton(bool)),
            static_cast<aidMainWindow*>(Globals::mainWindowPtr),
            SLOT(setActiveStateForStartStopButton(bool)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CTcpServer::~CTcpServer()
{
  server.close();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CTcpServer::acceptConnection()
{

  QTcpSocket *client = server.nextPendingConnection();
  client->setReadBufferSize(COM_MAX_DATA_SIZE);

  if(!Globals::imageRecEnabled)
  {
      showStatusMessage("Ignoring new data - image receiving is DISABLED.", UI_STATUS_NETWORK, true);
      client->close();
      client->deleteLater();
      return;
  };

  QString msg = "Receiving new data...";

  CSocketService *newSocket = new CSocketService(client, this);
  clientsList.append(newSocket);

  showStatusMessage(msg, UI_STATUS_NETWORK, true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CTcpServer::restart()
{
    if(server.isListening())
        server.close();

    server.listen(QHostAddress::Any, Globals::serverPort);
    if(server.isListening())
        emit setActiveStateForStartStopButton(true);
    else
        emit setActiveStateForStartStopButton(false);

    showStatusMessage(UI_SPEC_STATUS_NETWORK, 0, true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CTcpServer::timerEvent(QTimerEvent*)
{
    processClientQueue(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CTcpServer::processClientQueue(bool triggeredByTimer)
{
    QList<CSocketService*>::iterator i;

    Globals::imgContextListLock.lock();
    i = clientsList.begin();

    while (i!= clientsList.end())
    {
        if( (*i)->poolClientState(triggeredByTimer) == CLIENT_STATE_KILL_ME)
        {
            (*i)->deleteSocket();
            delete (*i);
            i = clientsList.erase(i);
        }
        else
            i++;
    }

    if(server.hasPendingConnections())
    {
        if(clientsList.size() < COM_MAX_PROCESSING_THREADS)
            acceptConnection();
    }

    Globals::imgContextListLock.unlock();
}

