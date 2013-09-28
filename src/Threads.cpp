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

#include "./inc/Threads.h"
#include "./inc/globals.h"
#include "./inc/aidMainWindow.h"

#ifdef QT4_HEADERS
    #include <QInputDialog>
    #include <QLineEdit>
    #include <QMessageBox>
    #include <QInputDialog>
#elif QT5_HEADERS
    #include <QtWidgets/QInputDialog>
    #include <QtWidgets/QLineEdit>
    #include <QtWidgets/QMessageBox>
    #include <QtWidgets/QInputDialog>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
void CWorker::selfStart()
{
    QThread* thread = new QThread;
    moveToThread(thread);
    connect(thread, SIGNAL(started()),  this,   SLOT(process()));
    connect(this,   SIGNAL(finished()), thread, SLOT(quit()));
    connect(this,   SIGNAL(finished()), this,   SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    if(informMeWhenFinished!=0)
        connect(this, SIGNAL(iAmDone()), informMeWhenFinished, SLOT(iAmDone()));
    thread->start();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

CWorker_loadFromNativeData::CWorker_loadFromNativeData(const QObject *parent, const QByteArray &qba) : CWorker(parent)
{
  this->qba = qba;
  reinterpretProcess = false;
  this->inBuffPtr = const_cast<char*>(qba.data());
  this->inBuffLength = qba.size();
}

CWorker_loadFromNativeData::CWorker_loadFromNativeData(const QObject *parent, const char* inBuffPtr, int inBuffLength) : CWorker(parent)
{
  reinterpretProcess = false;
  this->inBuffPtr = const_cast<char*>(inBuffPtr);
  this->inBuffLength = inBuffLength;
}

CWorker_loadFromNativeData::CWorker_loadFromNativeData(const QObject *parent, const QSharedPointer<CImgContext> &imgCtxPtr, uint iwidth, uint iheight, QString pixelFormatStr, uint rowStrideInBits, QString name, QString notes, const float gain[16], const float bias[4], quint32 auxFilteringFlags) : CWorker(parent)
{
  reinterpretProcess = true;
  this->imgCtxPtr = imgCtxPtr;
  this->name = name;
  this->notes = notes;
  this->pixelFormatStr = pixelFormatStr;
  this->iwidth = iwidth;
  this->iheight = iheight;
  this->rowStrideInBits = rowStrideInBits;
  memcpy(this->gain, gain, sizeof(float)*4);
  memcpy(this->bias, bias, sizeof(float)*4);
  this->auxFilteringFlags = auxFilteringFlags;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void CWorker_loadFromNativeData::process()
{
    QSharedPointer<CImgContext>  newImgContextPtr;
    dHeader                     *headerPtr = NULL;
    QSharedPointer<CNativeData>  nativeDataPtr;

    if(!reinterpretProcess)
    {
        if(inBuffLength < int(MAGIC_CHARS_SIZE + sizeof(dHeader)))
              goto __EXIT_WITH_ERROR;

        //check magic chars
        if((inBuffPtr[0]!=magichars[0])||
        (inBuffPtr[1]!=magichars[1])||
        (inBuffPtr[2]!=magichars[2])||
        (inBuffPtr[3]!=magichars[3]))
              goto __EXIT_WITH_ERROR;

        //check the limits
        headerPtr = (dHeader*)(inBuffPtr + MAGIC_CHARS_SIZE);

        if((uint)inBuffLength <= (MAGIC_CHARS_SIZE + sizeof(dHeader)\
                                   + headerPtr->formatStrLength \
                                   + headerPtr->nameLength \
                                   + headerPtr->notesLength ))
            goto __EXIT_WITH_ERROR;

        if(headerPtr->width > MAX_IMAGE_SIZE)
            goto __EXIT_WITH_ERROR;

        if(headerPtr->height > MAX_IMAGE_SIZE)
            goto __EXIT_WITH_ERROR;

        if(headerPtr->formatStrLength > MAX_FORMAT_STRING_LENGTH)
            goto __EXIT_WITH_ERROR;

        if(headerPtr->sizeInBytes > MAX_IMAGE_BLOCK_SIZE)
            goto __EXIT_WITH_ERROR;

        if(headerPtr->nameLength > MAX_IMG_NAME_LENGTH)
            goto __EXIT_WITH_ERROR;

        if(headerPtr->notesLength > MAX_IMG_NOTES_LENGTH)
            goto __EXIT_WITH_ERROR;

        if(headerPtr->nameLength  ==0)
        {
            name = "img";
            name += QString::number(Globals::imgCountAbs);
        }
        else
        {
            name = QString::fromLatin1((inBuffPtr + MAGIC_CHARS_SIZE + sizeof(dHeader) + headerPtr->formatStrLength),
                                            headerPtr->nameLength);
        }

        if(headerPtr->formatStrLength  ==0)
        {
            pixelFormatStr = "";
        }
        else
        {
            pixelFormatStr = QString::fromLatin1(inBuffPtr + MAGIC_CHARS_SIZE + sizeof(dHeader),
                                              headerPtr->formatStrLength);
        }

        if(headerPtr->notesLength  ==0)
        {
            notes = "";
        }
        else
        {
            notes = QString::fromLatin1((char*)(inBuffPtr + MAGIC_CHARS_SIZE + sizeof(dHeader) +
                                                      headerPtr->formatStrLength +
                                                      headerPtr->nameLength),
                                                      headerPtr->notesLength);
        }
    }

    //Create a new CNativeData object.
    if(!reinterpretProcess)
    {
        nativeDataPtr = QSharedPointer<CNativeData>(new CNativeData(inBuffPtr));
    }
    else if(imgCtxPtr->imgSource == SOURCE_FILE)
    {
        nativeDataPtr = QSharedPointer<CNativeData>(new CNativeData(imgCtxPtr->getVisualData()));
        nativeDataPtr->getData().append(COM_ALIGN_CHARS); // long long alignment
    }
    else
    {
        nativeDataPtr = imgCtxPtr->nativeDataPtr;
    }

    //Create a new CImgContext object.
    newImgContextPtr = QSharedPointer<CImgContext>(new CImgContext());
    newImgContextPtr->pendingFlag(PENDING_FLAG_LOCKED); // must pass

    if(Globals::autoScaleOnLoad)
       newImgContextPtr->setZoomFactor(0.0f);

    newImgContextPtr->need_thumbnail_refresh = true;
    Globals::addCmdToLocalQueue(CMD_CREATE_RENDERABLE_DATA, newImgContextPtr);

    newImgContextPtr->attachNativeData(nativeDataPtr);
    Globals::addImage(newImgContextPtr);

    //Load data.
    newImgContextPtr->setMyState(STATE_BUSY);

    if(!reinterpretProcess)
    {
        if(newImgContextPtr->loadFromNativeData(*headerPtr,
                                                pixelFormatStr,
                                                name,
                                                notes
                                                ) == RES_ERROR)
        {
            newImgContextPtr->setMyState(STATE_BAD);
            showStatusMessage("New image has been loaded - data corrupted or invalid format.", UI_STATUS_ERROR, true);
        }
        else
        {
            newImgContextPtr->setMyState(STATE_READY);
            showStatusMessage("New image has been loaded.", UI_STATUS_INFO, true);
        }
    }
    else
    {
        if(newImgContextPtr->loadFromNativeData(iwidth,
                                                iheight,
                                                rowStrideInBits,
                                                pixelFormatStr,
                                                name,
                                                notes,
                                                gain,
                                                bias,
                                                auxFilteringFlags
                                                ) == RES_ERROR)
        {
            newImgContextPtr->setMyState(STATE_BAD);
            showStatusMessage("Data reinterpretation - data corrupted or invalid format.", UI_STATUS_ERROR, true);
        }
        else
        {
            newImgContextPtr->setMyState(STATE_READY);
            showStatusMessage("Data reinterpretation finished.", UI_STATUS_INFO, true);
        }
    }

    newImgContextPtr->need_thumbnail_refresh = true;
    Globals::addCmdToLocalQueue(CMD_CREATE_RENDERABLE_DATA, newImgContextPtr);
    emit iAmDone();
    emit finished();
    return;

    __EXIT_WITH_ERROR:
    showStatusMessage("Image loading error.", UI_STATUS_ERROR, true);
    emit iAmDone();
    emit finished();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
CWorker_loadFromRICFile::CWorker_loadFromRICFile(const QString &fileName):CWorker(0)
{
    this->fileName = fileName;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CWorker_loadFromRICFile::process()
{
    QFile   ifile(fileName);
    int     fileSize;

    if(!ifile.open(QIODevice::ReadOnly))
    {
        showStatusMessage("Error opening the file.", UI_STATUS_ERROR, true);
        emit finished();
        return;
    }

    fileSize = ifile.bytesAvailable();
    char* inBuff = (char*)malloc(fileSize);
    if(!inBuff)
        return;
    ifile.read(inBuff, fileSize);

    CWorker_loadFromNativeData lnd(0, inBuff, fileSize);

    lnd.blockSignals(true);
    lnd.process();
    emit iAmDone();
    emit finished();
    ifile.close();
    return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
CWorker_loadFromGraphicsFile::CWorker_loadFromGraphicsFile(QSharedPointer<CImgContext> _imgContextPtr,
                                                           QString _fileName): CWorker(0)
{
    imgContextPtr = _imgContextPtr;
    fileName = _fileName;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CWorker_loadFromGraphicsFile::process()
{
    //Load data.
    imgContextPtr->pendingFlag(PENDING_FLAG_LOCKED); // must pass
    imgContextPtr->setMyState(STATE_BUSY);

    if(Globals::autoScaleOnLoad)
      imgContextPtr->setZoomFactor(0.0f);

    if(imgContextPtr->loadFromFile(fileName) == RES_ERROR)
    {
         imgContextPtr->setMyState(STATE_BAD);
         showStatusMessage("Error loading an image.", UI_STATUS_ERROR, true);
    }
    else
    {
         imgContextPtr->setMyState(STATE_READY);
         showStatusMessage("New image has been loaded.", UI_STATUS_INFO, true);
    }
    imgContextPtr->need_thumbnail_refresh = true;

    Globals::addCmdToLocalQueue(CMD_CREATE_RENDERABLE_DATA, imgContextPtr);
    emit iAmDone();
    emit finished();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
CWorker_saveToGraphicsFile::CWorker_saveToGraphicsFile(QSharedPointer<CImgContext> _imgContextPtr,
                                                       QString _fileName): CWorker(0)
{
    imgContextPtr = _imgContextPtr;
    fileName = _fileName;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CWorker_saveToGraphicsFile::process()
{
    if(!Globals::toolThreadSlot.tryLock())
    {
      Globals::addCmdToLocalQueue(CMD_SHOW_MSGBOX_TOOL_SLOT_BUSY);
      return;
    }

    Globals::imgContextListLock.lock();

    if(!imgContextPtr.isNull())
    {
        if(imgContextPtr->getMyState() == STATE_READY)
        {
            imgContextPtr->setMyState(STATE_BUSY);
            Globals::imgContextListLock.unlock();
            Globals::addCmdToLocalQueue(CMD_CREATE_THUMBNAIL, imgContextPtr);
            imgContextPtr->auxInfo = "Saving to a graphics file...";
            if(imgContextPtr->saveToGraphicsFile(fileName) == RES_ERROR)
            {
                Globals::addCmdToLocalQueue(CMD_SHOW_MSGBOX_SAVE_GFILE_FAILED);
                showStatusMessage("Error saving the image.", UI_STATUS_ERROR, true);
            }
            else
                showStatusMessage("The image has been saved.", UI_STATUS_INFO, true);

            imgContextPtr->auxInfo = "";
            imgContextPtr->setMyState(STATE_READY);
        }
        else
            Globals::imgContextListLock.unlock();

        Globals::addCmdToLocalQueue(CMD_CREATE_THUMBNAIL, imgContextPtr);
    }
    else
    {
        showStatusMessage("Nothing to save.", UI_STATUS_ERROR, true);
        Globals::imgContextListLock.unlock();
    }

    emit iAmDone();
    emit finished();
    Globals::addCmdToLocalQueue(CMD_REFRESH_VIEW_PANLES);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
CWorker_saveToRICFile::CWorker_saveToRICFile(QSharedPointer<CImgContext> _imgContextPtr,
                                             QString _fileName): CWorker(0)
{
    imgContextPtr = _imgContextPtr;
    fileName = _fileName;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CWorker_saveToRICFile::process()
{
    Globals::imgContextListLock.lock();
    if(!imgContextPtr.isNull())
    {
        if((imgContextPtr->getMyState() == STATE_READY)||(imgContextPtr->getMyState() == STATE_BAD))
        {
            uint lastState = imgContextPtr->getMyState();
            imgContextPtr->setMyState(STATE_BUSY);
            Globals::addCmdToLocalQueue(CMD_CREATE_THUMBNAIL, imgContextPtr);
            imgContextPtr->auxInfo = "Saving to a RIC file...";
            Globals::imgContextListLock.unlock();
            if(imgContextPtr->saveToRICFile(fileName) == RES_ERROR)
            {
                Globals::addCmdToLocalQueue(CMD_SHOW_MSGBOX_SAVE_GFILE_FAILED);
                showStatusMessage("Error saving the image.", UI_STATUS_ERROR, true);
            }
            else
                showStatusMessage("The image has been saved.", UI_STATUS_INFO, true);

            imgContextPtr->auxInfo = "";
            imgContextPtr->setMyState(lastState);
        }
        else
            Globals::imgContextListLock.unlock();

        Globals::addCmdToLocalQueue(CMD_CREATE_THUMBNAIL, imgContextPtr);
    }
    else
    {
        showStatusMessage("Nothing to save.", UI_STATUS_ERROR, true);
        Globals::imgContextListLock.unlock();
    }
    emit iAmDone();
    emit finished();
    Globals::addCmdToLocalQueue(CMD_REFRESH_VIEW_PANLES);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
CWorker_ImageComparator::CWorker_ImageComparator(const QObject *parent, qint32 shiftAX, qint32 shiftAY, bool vFlip, bool hFlip, float thresholdsv[4], QString name, QSharedPointer<CImgContext> imgA, QSharedPointer<CImgContext> imgB) : CWorker(parent)
{
    this->imgA = imgA;
    this->imgB = imgB;
    this->shiftAX = shiftAX;
    this->shiftAY = shiftAY;
    this->name = name;
    this->hFlip = hFlip;
    this->vFlip = vFlip;
    memcpy(thresholds, thresholdsv, sizeof(float)*4);
}

void CWorker_ImageComparator::process()
{
    if(!Globals::toolThreadSlot.tryLock())
    {
      Globals::addCmdToLocalQueue(CMD_SHOW_MSGBOX_TOOL_SLOT_BUSY);
      return;
    }

    QSharedPointer<CImgContext> compResult   = QSharedPointer<CImgContext>(new CImgContext());
    QSharedPointer<CNativeData> nativeResult = QSharedPointer<CNativeData>(new CNativeData());

    QByteArray dataA, dataB;
    qint32 start_x, start_y, stop_x, stop_y, auX;
    BitIndexAndCount bic;
    quint32 bitCursorA, bitCursorB, offsAX, offsAY, offsX, offsY, shiftBX, shiftBY;
    quint32 hFlipMod, vFlipMod;
    float* resBuffPtr;
    float pA[4], pB[4];

    float dummyTab[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    if(imgA.isNull())
         goto __EXIT_IMMEDIATE;

    if(imgB.isNull())
         goto __EXIT_IMMEDIATE;

    if((imgA->getMyState() != STATE_READY)||(imgB->getMyState() != STATE_READY))
        goto __EXIT_IMMEDIATE;

    imgA->setMyState(STATE_BUSY);
    imgB->setMyState(STATE_BUSY);
    Globals::addCmdToLocalQueue(CMD_REFRESH_THUMBNAILS_LIST);

    compResult->pendingFlag(PENDING_FLAG_LOCKED); // must pass

    imgA->auxInfo = imgB->auxInfo = compResult->auxInfo = "Comparing...";

    if(imgA->imgSource == SOURCE_FILE)
    {
        dataA = imgA->getVisualData();
        imgA->myNormalizator.setNativeDataPtr(dataA.data());
        imgA->myNormalizator.resetMasks();
        bic.bitIndex=0; bic.bitsCount=8;
        imgA->myNormalizator.addBLUEBitsMask(BitIndexAndCount(bic));
        imgA->myNormalizator.setBLUEType(NORM_IUNSIGNED);
        bic.bitIndex=8; bic.bitsCount=8;
        imgA->myNormalizator.addGREENBitsMask(BitIndexAndCount(bic));
        imgA->myNormalizator.setGREENType(NORM_IUNSIGNED);
        bic.bitIndex=16; bic.bitsCount=8;
        imgA->myNormalizator.addREDBitsMask(BitIndexAndCount(bic));
        imgA->myNormalizator.setREDType(NORM_IUNSIGNED);
        bic.bitIndex=24; bic.bitsCount=8;
        imgA->myNormalizator.addALPHABitsMask(BitIndexAndCount(bic));
        imgA->myNormalizator.setALPHAType(NORM_IUNSIGNED);
        imgA->myNormalizator.adjustCapacity();
    }

    if(imgB->imgSource == SOURCE_FILE)
    {
        dataB = imgB->getVisualData();
        imgB->myNormalizator.setNativeDataPtr(dataB.data());
        imgB->myNormalizator.resetMasks();
        bic.bitIndex=0; bic.bitsCount=8;
        imgB->myNormalizator.addBLUEBitsMask(BitIndexAndCount(bic));
        imgB->myNormalizator.setBLUEType(NORM_IUNSIGNED);
        bic.bitIndex=8; bic.bitsCount=8;
        imgB->myNormalizator.addGREENBitsMask(BitIndexAndCount(bic));
        imgB->myNormalizator.setGREENType(NORM_IUNSIGNED);
        bic.bitIndex=16; bic.bitsCount=8;
        imgB->myNormalizator.addREDBitsMask(BitIndexAndCount(bic));
        imgB->myNormalizator.setREDType(NORM_IUNSIGNED);
        bic.bitIndex=24; bic.bitsCount=8;
        imgB->myNormalizator.addALPHABitsMask(BitIndexAndCount(bic));
        imgB->myNormalizator.setALPHAType(NORM_IUNSIGNED);
        imgB->myNormalizator.adjustCapacity();
    }

    start_x = max(shiftAX, 0);
    start_y = max(shiftAY, 0);
    stop_x  = min(imgA->getIWidth()+shiftAX, imgB->getIWidth());
    stop_y  = min(imgA->getIHeight()+shiftAY, imgB->getIHeight());

    if((stop_x <= start_x)||(stop_y <= start_y))
    {
        imgA->setMyState(STATE_READY);
        imgB->setMyState(STATE_READY);
        imgA->auxInfo = imgB->auxInfo = "";
        Globals::addCmdToLocalQueue(CMD_REFRESH_THUMBNAILS_LIST);
        goto __EXIT_POINT;
    }

    compResult->setIWidth(stop_x - start_x);
    compResult->setIHeight(stop_y - start_y);
    compResult->setMyState(STATE_BUSY);
    compResult->myPixelFormat = "fa R32 G32 B32 A32";

    nativeResult->getData().fill(0, compResult->getIWidth()*compResult->getIHeight()*16);
    resBuffPtr = (float*)nativeResult->getData().data();

    shiftBX = 0;
    if(shiftAX<0)
    {
        shiftBX = 0;
        shiftAX =-shiftAX;
    }
    else
    {
        shiftBX = shiftAX;
        shiftAX = 0;
    }

    if(shiftAY<0)
    {
        shiftBY = 0;
        shiftAY =-shiftAY;
    }
    else
    {
        shiftBY = shiftAY;
        shiftAY = 0;
    }

    offsAX = offsAY = 0;
    offsX = offsY = 0;
    vFlipMod = hFlipMod = 1;

    if(hFlip)
        hFlipMod = -1;

    if(vFlip)
    {
        vFlipMod = -1;
        offsAY = compResult->getIHeight()-1;
    }

    {
        for(; start_y < stop_y; start_y++)
        {
            bitCursorA = (((shiftAY + offsY)*imgA->getIWidth() + shiftAX)*imgA->myNormalizator.getEffectiveBitCount());
            bitCursorB = (((shiftBY + offsY)*imgB->getIWidth() + shiftBX)*imgB->myNormalizator.getEffectiveBitCount());

            offsAX = 0;
            offsX = 0;
            if(hFlip)
                offsAX = compResult->getIWidth()-1;

            for(auX = start_x; auX < stop_x; auX++)
            {
                imgA->myNormalizator.getPixelValue(bitCursorA, pA);
                imgB->myNormalizator.getPixelValue(bitCursorB, pB);

                resBuffPtr[4*(offsAY*compResult->getIWidth() + offsAX)    ] += pA[0];
                resBuffPtr[4*(offsAY*compResult->getIWidth() + offsAX) + 1] += pA[1];
                resBuffPtr[4*(offsAY*compResult->getIWidth() + offsAX) + 2] += pA[2];
                resBuffPtr[4*(offsAY*compResult->getIWidth() + offsAX) + 3] += pA[3];

                resBuffPtr[4*(offsY*compResult->getIWidth() + offsX)    ]   -= pB[0];
                resBuffPtr[4*(offsY*compResult->getIWidth() + offsX) + 1]   -= pB[1];
                resBuffPtr[4*(offsY*compResult->getIWidth() + offsX) + 2]   -= pB[2];
                resBuffPtr[4*(offsY*compResult->getIWidth() + offsX) + 3]   -= pB[3];

                if(abs(resBuffPtr[4*(offsY*compResult->getIWidth() + offsX)])     < thresholds[0])
                   resBuffPtr[4*(offsY*compResult->getIWidth() + offsX)    ] = 0.0f;
                if(abs(resBuffPtr[4*(offsY*compResult->getIWidth() + offsX) + 1]) < thresholds[1])
                   resBuffPtr[4*(offsY*compResult->getIWidth() + offsX) + 1] = 0.0f;
                if(abs(resBuffPtr[4*(offsY*compResult->getIWidth() + offsX) + 2]) < thresholds[2])
                   resBuffPtr[4*(offsY*compResult->getIWidth() + offsX) + 2] = 0.0f;
                if(abs(resBuffPtr[4*(offsY*compResult->getIWidth() + offsX) + 3]) < thresholds[3])
                   resBuffPtr[4*(offsY*compResult->getIWidth() + offsX) + 3] = 0.0f;

                offsAX+=hFlipMod;
                offsX++;
                bitCursorA += imgA->myNormalizator.getColumnStride();
                bitCursorB += imgB->myNormalizator.getColumnStride();
            }
            offsAY+=vFlipMod;
            offsY++;
            bitCursorA += imgA->myNormalizator.getRowStride();
            bitCursorB += imgB->myNormalizator.getRowStride();
            imgA->auxInfo = imgB->auxInfo = compResult->auxInfo = "Comparing: " + QString::number((ulong)start_y*100/stop_y) + "%";
        }

        imgA->setMyState(STATE_READY);
        imgB->setMyState(STATE_READY);
        imgA->auxInfo = imgB->auxInfo = "";
        Globals::addCmdToLocalQueue(CMD_REFRESH_THUMBNAILS_LIST);

        //statistics
        float meanSNR[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        float minValue[4] = {MAX_FLOAT, MAX_FLOAT, MAX_FLOAT, MAX_FLOAT};
        float maxValue[4] = {-MAX_FLOAT, -MAX_FLOAT, -MAX_FLOAT, -MAX_FLOAT};

        for(start_y = 0; start_y < (int)compResult->getIHeight(); start_y++)
        {
            for(start_x = 0; start_x < (int)compResult->getIWidth(); start_x++)
            {
                meanSNR[0] += resBuffPtr[4*(start_y*compResult->getIWidth() + start_x)   ]*
                              resBuffPtr[4*(start_y*compResult->getIWidth() + start_x)   ];
                meanSNR[1] += resBuffPtr[4*(start_y*compResult->getIWidth() + start_x) +1]*
                              resBuffPtr[4*(start_y*compResult->getIWidth() + start_x) +1];
                meanSNR[2] += resBuffPtr[4*(start_y*compResult->getIWidth() + start_x) +2]*
                              resBuffPtr[4*(start_y*compResult->getIWidth() + start_x) +2];
                meanSNR[3] += resBuffPtr[4*(start_y*compResult->getIWidth() + start_x) +3]*
                              resBuffPtr[4*(start_y*compResult->getIWidth() + start_x) +3];

                if(resBuffPtr[4*(start_y*compResult->getIWidth() + start_x)   ] < minValue[0])
                    minValue[0] = resBuffPtr[4*(start_y*compResult->getIWidth() + start_x)   ];
                if(resBuffPtr[4*(start_y*compResult->getIWidth() + start_x)   ] > maxValue[0])
                    maxValue[0] = resBuffPtr[4*(start_y*compResult->getIWidth() + start_x)   ];

                if(resBuffPtr[4*(start_y*compResult->getIWidth() + start_x) +1] < minValue[1])
                    minValue[1] = resBuffPtr[4*(start_y*compResult->getIWidth() + start_x) +1];
                if(resBuffPtr[4*(start_y*compResult->getIWidth() + start_x) +1] > maxValue[1])
                    maxValue[1] = resBuffPtr[4*(start_y*compResult->getIWidth() + start_x) +1];

                if(resBuffPtr[4*(start_y*compResult->getIWidth() + start_x) +2] < minValue[2])
                    minValue[2] = resBuffPtr[4*(start_y*compResult->getIWidth() + start_x) +2];
                if(resBuffPtr[4*(start_y*compResult->getIWidth() + start_x) +2] > maxValue[2])
                    maxValue[2] = resBuffPtr[4*(start_y*compResult->getIWidth() + start_x) +2];

                if(resBuffPtr[4*(start_y*compResult->getIWidth() + start_x) +3] < minValue[3])
                    minValue[3] = resBuffPtr[4*(start_y*compResult->getIWidth() + start_x) +3];
                if(resBuffPtr[4*(start_y*compResult->getIWidth() + start_x) +3] > maxValue[3])
                    maxValue[3] = resBuffPtr[4*(start_y*compResult->getIWidth() + start_x) +3];
            }
            compResult->auxInfo = "Statistics: " + QString::number((ulong)start_y*100/compResult->getIHeight()) + "%";
        }
        meanSNR[0]/= compResult->getIWidth()*compResult->getIHeight();
        meanSNR[1]/= compResult->getIWidth()*compResult->getIHeight();
        meanSNR[2]/= compResult->getIWidth()*compResult->getIHeight();
        meanSNR[3]/= compResult->getIWidth()*compResult->getIHeight();

        compResult->myNotes += "\n--------------------------------------------------\n";
        compResult->myNotes += "###Comparison statistics###\n";
        compResult->myNotes += "@ Max/min values:";
        compResult->myNotes += "\nR: " + QString::number(maxValue[0]) + " / " + QString::number(minValue[0]);
        compResult->myNotes += "\nG: " + QString::number(maxValue[1]) + " / " + QString::number(minValue[1]);
        compResult->myNotes += "\nB: " + QString::number(maxValue[2]) + " / " + QString::number(minValue[2]);
        compResult->myNotes += "\nA: " + QString::number(maxValue[3]) + " / " + QString::number(minValue[3]);
        compResult->myNotes += "\n@ Mean SNR:";
        compResult->myNotes += "\nR: " + QString::number(meanSNR[0]);
        compResult->myNotes += "\nG: " + QString::number(meanSNR[1]);
        compResult->myNotes += "\nB: " + QString::number(meanSNR[2]);
        compResult->myNotes += "\nA: " + QString::number(meanSNR[3]);
        compResult->myNotes += "\n--------------------------------------------------\n";
    }
    compResult->attachNativeData(nativeResult);

    compResult->need_thumbnail_refresh = true;
    Globals::addCmdToLocalQueue(CMD_CREATE_RENDERABLE_DATA, compResult);
    Globals::addImage(compResult);

    //Load data.
    compResult->setMyState(STATE_BUSY);

    compResult->loadFromNativeData(compResult->getIWidth(),
                                   compResult->getIHeight(),
                                   0,
                                   compResult->myPixelFormat,
                                   name,
                                   compResult->myNotes,
                                   dummyTab,
                                   dummyTab,
                                   AUX_FILTERIN_AUTO_LINEAR
                                   );

    compResult->setZoomFactor(0.0f);
    compResult->setMyState(STATE_READY);
    compResult->auxInfo = "";
    showStatusMessage("Comparation finished.", UI_STATUS_INFO, true);

    compResult->need_thumbnail_refresh = true;
    Globals::addCmdToLocalQueue(CMD_CREATE_RENDERABLE_DATA, compResult);

    emit iAmDone();
    emit finished();

__EXIT_POINT:
    if(imgA->imgSource == SOURCE_FILE)
    {
        imgA->myNormalizator.setNativeDataPtr(0);
    }

    if(imgB->imgSource == SOURCE_FILE)
    {
        imgB->myNormalizator.setNativeDataPtr(0);
    }

__EXIT_IMMEDIATE:
    Globals::toolThreadSlot.unlock();
    Globals::addCmdToLocalQueue(CMD_REFRESH_VIEW_PANLES);
 return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
CWorker_ImageRecaster::CWorker_ImageRecaster(const QObject *parent, float gain[16], float bias[4], bool vFlip, bool hFlip, quint32 auxFlags, QString name, const QSharedPointer<CImgContext> &imgCtx) : CWorker(parent)
{
    this->imgCtx = imgCtx;
    this->name = name;
    memcpy(this->gain, gain, sizeof(float)*16);
    memcpy(this->bias, bias, sizeof(float)*4);

    this->hFlip = hFlip;
    this->vFlip = vFlip;
    this->auxFlags = auxFlags;
}

void CWorker_ImageRecaster::process()
{
    if(!Globals::toolThreadSlot.tryLock())
    {
      Globals::addCmdToLocalQueue(CMD_SHOW_MSGBOX_TOOL_SLOT_BUSY);
      return;
    }

    QSharedPointer<CImgContext> recastResult = QSharedPointer<CImgContext>(new CImgContext());
    QSharedPointer<CNativeData> nativeResult = QSharedPointer<CNativeData>(new CNativeData());

    QByteArray data;
    quint32 start_x, start_y, stop_x, stop_y;
    BitIndexAndCount bic;
    quint32 bitCursor, offsX, offsY, offsYD, auX;
    quint32 hFlipMod, vFlipMod;
    float*  resBuffPtr;
    float   p[4];

    float unitGain[4]  = {1.0f, 1.0f, 1.0f, 1.0f};

    float zeroBias[4]  = {0.0f, 0.0f, 0.0f, 0.0f};

    if(imgCtx.isNull())
         goto __EXIT_IMMEDIATE;

    if((imgCtx->getMyState() != STATE_READY))
        goto __EXIT_IMMEDIATE;

    imgCtx->setMyState(STATE_BUSY);
    Globals::addCmdToLocalQueue(CMD_REFRESH_THUMBNAILS_LIST);

    recastResult->pendingFlag(PENDING_FLAG_LOCKED); // must pass

    imgCtx->auxInfo = recastResult->auxInfo = "Recasting...";

    if(imgCtx->imgSource == SOURCE_FILE)
    {
        data = imgCtx->getVisualData();
        imgCtx->myNormalizator.setNativeDataPtr(data.data());
        imgCtx->myNormalizator.resetMasks();
        bic.bitIndex=0; bic.bitsCount=8;
        imgCtx->myNormalizator.addBLUEBitsMask(BitIndexAndCount(bic));
        imgCtx->myNormalizator.setBLUEType(NORM_IUNSIGNED);
        bic.bitIndex=8; bic.bitsCount=8;
        imgCtx->myNormalizator.addGREENBitsMask(BitIndexAndCount(bic));
        imgCtx->myNormalizator.setGREENType(NORM_IUNSIGNED);
        bic.bitIndex=16; bic.bitsCount=8;
        imgCtx->myNormalizator.addREDBitsMask(BitIndexAndCount(bic));
        imgCtx->myNormalizator.setREDType(NORM_IUNSIGNED);
        bic.bitIndex=24; bic.bitsCount=8;
        imgCtx->myNormalizator.addALPHABitsMask(BitIndexAndCount(bic));
        imgCtx->myNormalizator.setALPHAType(NORM_IUNSIGNED);
        imgCtx->myNormalizator.adjustCapacity();
    }

    start_x = 0;
    start_y = 0;
    stop_x  = imgCtx->getIWidth();
    stop_y  = imgCtx->getIHeight();

    if((stop_x <= start_x)||(stop_y <= start_y))
        goto __EXIT_POINT;

    recastResult->setIWidth(stop_x - start_x);
    recastResult->setIHeight(stop_y - start_y);
    recastResult->setMyState(STATE_BUSY);
    recastResult->myPixelFormat = "f R32 G32 B32 A32";
    recastResult->myNotes += "Recast from: " + imgCtx->getMyName();

    nativeResult->getData().fill(0, recastResult->getIWidth()*recastResult->getIHeight()*16);
    resBuffPtr = (float*)nativeResult->getData().data();

    offsX = offsY = offsYD = 0;
    vFlipMod = hFlipMod = 1;

    if(hFlip)
        hFlipMod = -1;

    if(vFlip)
    {
        vFlipMod = -1;
        offsY = recastResult->getIHeight()-1;
    }

    {
        for(; start_y < stop_y; start_y++)
        {
            bitCursor = ((offsYD*imgCtx->getIWidth())*imgCtx->myNormalizator.getEffectiveBitCount());
            offsX = 0;
            if(hFlip)
                offsX = recastResult->getIWidth()-1;

            for(auX = start_x; auX < stop_x; auX++)
            {
                imgCtx->myNormalizator.getPixelValue(bitCursor, p);

                resBuffPtr[4*(offsY*recastResult->getIWidth() + offsX)    ] = gain[0]*p[0] + gain[1]*p[1] + gain[2]*p[2] + gain[3]*p[3] + bias[0];
                resBuffPtr[4*(offsY*recastResult->getIWidth() + offsX) + 1] = gain[4]*p[0] + gain[5]*p[1] + gain[2]*p[2] + gain[7]*p[3] + bias[1];
                resBuffPtr[4*(offsY*recastResult->getIWidth() + offsX) + 2] = gain[8]*p[0] + gain[9]*p[1] + gain[10]*p[2] + gain[11]*p[3] + bias[2];
                resBuffPtr[4*(offsY*recastResult->getIWidth() + offsX) + 3] = gain[12]*p[0] + gain[13]*p[1] + gain[14]*p[2] + gain[15]*p[3] + bias[3];

                offsX+=hFlipMod;

                bitCursor += imgCtx->myNormalizator.getColumnStride();
            }

            offsY+=vFlipMod;
            offsYD++;
            bitCursor += imgCtx->myNormalizator.getRowStride();
            imgCtx->auxInfo = recastResult->auxInfo = "Recasting: " + QString::number((ulong)start_y*100/stop_y) + "%";
        }

        imgCtx->setMyState(STATE_READY);
        imgCtx->auxInfo = recastResult->auxInfo = "";
        Globals::addCmdToLocalQueue(CMD_REFRESH_THUMBNAILS_LIST);
    }

    recastResult->attachNativeData(nativeResult);

    recastResult->need_thumbnail_refresh = true;
    Globals::addCmdToLocalQueue(CMD_CREATE_RENDERABLE_DATA, recastResult);
    Globals::addImage(recastResult);

    //Load data.
    recastResult->setMyState(STATE_BUSY);

    recastResult->loadFromNativeData(recastResult->getIWidth(),
                                     recastResult->getIHeight(),
                                     0,
                                     recastResult->myPixelFormat,
                                     name,
                                     recastResult->myNotes,
                                     unitGain,
                                     zeroBias,
                                     auxFlags
                                     );

    recastResult->setZoomFactor(0.0f);
    recastResult->setMyState(STATE_READY);
    recastResult->auxInfo = "";
    showStatusMessage("Recast finished.", UI_STATUS_INFO, true);

    recastResult->need_thumbnail_refresh = true;
    Globals::addCmdToLocalQueue(CMD_CREATE_RENDERABLE_DATA, recastResult);

    emit iAmDone();
    emit finished();


__EXIT_POINT:
    if(imgCtx->imgSource == SOURCE_FILE)
    {
        imgCtx->myNormalizator.setNativeDataPtr(0);
    }

__EXIT_IMMEDIATE:
    Globals::toolThreadSlot.unlock();
    Globals::addCmdToLocalQueue(CMD_REFRESH_VIEW_PANLES);
    return;
}
