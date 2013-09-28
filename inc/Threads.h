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

#ifndef THREADS_H
#define THREADS_H

#include "CImgContext.h"

#include <QThread>
#include <QObject>

////////////////////////////////////////////////////////////////////////////////////////////////////
//Virtual class for a worker.
class CWorker : public QObject
{
    Q_OBJECT
public:
    CWorker(const QObject* parent){informMeWhenFinished = const_cast<QObject*>(parent);}
    void         selfStart();

public slots:
    virtual void process()=0;


signals:
    void         finished();
    void         iAmDone();

private:
    QObject     *informMeWhenFinished;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
class CWorker_loadFromNativeData : public CWorker
{
 public:
    CWorker_loadFromNativeData(const QObject *parent, const QByteArray &qba);
    CWorker_loadFromNativeData(const QObject *parent, const char *inBuffPtr, int inBuffLength);
    CWorker_loadFromNativeData(const QObject *parent, const QSharedPointer<CImgContext> &imgCtxPtr, uint iwidth, uint iheight, QString pixelFormatStr, uint rowStrideInBits, QString name, QString notes, const float gain[16], const float bias[16], quint32 auxFilteringFlags);
    virtual void               process();

 private:
   bool                        reinterpretProcess;
   QSharedPointer<CImgContext> imgCtxPtr;
   char*                       inBuffPtr;
   QByteArray                  qba;
   int                         inBuffLength;
   QString                     name;
   QString                     notes;
   quint32                     iwidth;
   quint32                     iheight;
   QString                     pixelFormatStr;
   uint                        rowStrideInBits;
   float                       gain[4];
   float                       bias[4];
   quint32                     auxFilteringFlags;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
class CWorker_loadFromRICFile : public CWorker
{
 public:
    CWorker_loadFromRICFile(const QString &fileName);

    virtual void process();

 private:
   QString       fileName;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
class CWorker_loadFromGraphicsFile : public CWorker
{
 public:

   CWorker_loadFromGraphicsFile(QSharedPointer<CImgContext> _imgContextPtr,
                                QString _fileName);

   virtual void                 process();

 private:
   QSharedPointer<CImgContext> imgContextPtr;
   QString                     fileName;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
class CWorker_saveToGraphicsFile : public CWorker
{
 Q_OBJECT
 public:

   CWorker_saveToGraphicsFile(QSharedPointer<CImgContext> _imgContextPtr,
                             QString _fileName);

   virtual void                process();

 private:
   QSharedPointer<CImgContext> imgContextPtr;
   QString                     fileName;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
class CWorker_saveToRICFile : public CWorker
{
 Q_OBJECT
 public:

   CWorker_saveToRICFile(QSharedPointer<CImgContext> _imgContextPtr,
                         QString _fileName);

   virtual void                process();

 private:
   QSharedPointer<CImgContext> imgContextPtr;
   QString                     fileName;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
class CWorker_ImageComparator : public CWorker
{
    Q_OBJECT
    public:

    CWorker_ImageComparator(const QObject *parent, qint32 shiftAX, qint32 shiftAY, bool vFlip, bool hFlip, float thresholdsv[4], QString name, QSharedPointer<CImgContext> imgA, QSharedPointer<CImgContext> imgB);
    virtual void process();

private:
    QSharedPointer<CImgContext> imgA;
    QSharedPointer<CImgContext> imgB;
    qint32                      shiftAX, shiftAY;
    QString                     name;
    float                       thresholds[4];
    bool                        vFlip;
    bool                        hFlip;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
class CWorker_ImageRecaster : public CWorker
{
    Q_OBJECT
    public:

    CWorker_ImageRecaster(const QObject *parent, float gain[16], float bias[4], bool vFlip, bool hFlip, quint32 auxFlags, QString name, const QSharedPointer<CImgContext> &imgCtx);
    virtual void                process();

private:
    QSharedPointer<CImgContext> imgCtx;
    float                       gain[16];
    float                       bias[4];
    QString                     name;
    bool                        vFlip;
    bool                        hFlip;
    quint32                     auxFlags;
};
#endif // THREADS_H
