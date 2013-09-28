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

#ifndef CIMGCONTEXT_H
#define CIMGCONTEXT_H

#ifndef IMGDATABUFFER_H
#define IMGDATABUFFER_H

#include "commons.h"
#include "defines.h"
#include "CNativeData.h"
#include "CNormalizator.h"
#include "CBitParser.h"

#include <QPixmap>
#include <QtDebug>
#include <QtGlobal>
#include <QFileInfo>

#include <QPointer>
#include <QPainter>
#include <QMutex>


#ifdef QT4_HEADERS
    #include <QListWidgetItem>
#elif QT5_HEADERS
    #include <QtWidgets/QListWidgetItem>
#endif

/*!
 * Image context specific parameters/flags.
 */

//Context image state flags
const uint STATE_READY             =0x00;    //Ready for render.
const uint STATE_BUSY              =0x01;    //Busy - currently processed.
const uint STATE_BAD               =0x02;    //Bad - data corrupted or invalid bit parser string.
const uint STATE_DELETE_MARK       =0x03;    //Marked for deletion.

//Image source flags
const int SOURCE_RAW               =0x01;    //Loaded from uploaded pixel array.
const int SOURCE_FILE              =0x02;    //Loaded from a graphics file.

#define THREAD_SAFE QMutexLocker lock(&internalLock);

///////////////////////////////////////////////////////////////////////////////////////////////////
//Basic per-pixel value operations
///////////////////////////////////////////////////////////////////////////////////////////////////
/*!
 *  \brief   Color addition with saturation.
 *  \param   X    base color
 *  \param   Y    modifier
 *  \return  X+Y in [0; 0xFF] range
 */
inline quint8 sadd8(quint8 X, qint8 Y)
{
    qint16 tmp = X; tmp += Y;

    if(tmp<0)return 0;
    else if(tmp> 0xFF) return 0xFF;
    return (quint8)tmp;
}

/*!
 *  \brief   Color multiplication with saturation.
 *  \param   X    base color
 *  \param   a    scale modifier
 *  \param   b    offset modifier
 *  \return  a*X/b+c in [0; 0xFF] range
 */
inline quint8 smuladd8(qint32 X, quint8 a, quint8 b, quint8 c)
{
    X = a*X/b+c;
    if(X> 0xFF) return 0xFF;
    else if (X<0) return 0x00;
    return (quint8)X;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//Thumbnail wrapper class
///////////////////////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief The CThumbnail class.
 *        This is a wrapper for the QListWidgetItem class.
 *        For each of the loaded image, a thumbnail, visible
 *        on the bottom panel list, is created.
 */
class CThumbnail : public QListWidgetItem
{
public:
    /*!
     * When <dont_process_an_update_event> is set, the <itemNameChanged>
     * event is not processed.
     */
    bool    dont_process_an_update_event;

    CThumbnail():dont_process_an_update_event(false), myParentPtr(NULL){}

    CThumbnail(QListWidgetItem* _ptr, void* _myParentPtr):QListWidgetItem((QListWidgetItem)*_ptr), myParentPtr(_myParentPtr)
    {
       dont_process_an_update_event = false;
    }

    CThumbnail* clone() const
    {
        return new CThumbnail((QListWidgetItem*)this, myParentPtr);
    }

    // Pointer to an image context instance.
private:
    void*   myParentPtr;
public:
    void*   getMyParent()
            {return myParentPtr;}
    void    setMyParent(void* _myParentPtr)
            {myParentPtr = _myParentPtr;}

};

///////////////////////////////////////////////////////////////////////////////////////////////////
//Image context class declaration
///////////////////////////////////////////////////////////////////////////////////////////////////
/*!
 *  /brief Image context class.
 *  This class encapsulates properties and attributes associated with an image view.
 */

const int IMGCX_RED_check               =0x002;
const int IMGCX_GREEN_check             =0x004;
const int IMGCX_BLUE_check              =0x008;
const int IMGCX_ALPHA_check             =0x010;
const int IMGCX_LINEAR_TRANSFORM_check  =0x020;
const int IMGCX_SWAPPED_REDBLUE_check   =0x040;
const int IMGCX_SWAPPED_REDGREEN_check  =0x080;
const int IMGCX_VERTICAL_FLIP_check     =0x100;
const int IMGCX_HORIZONTAL_FLIP_check   =0x200;
const int IMGCX_SNAP_TO_GRID_check      =0x400;
const int IMGCX_TRANSPARENT_BKG_check   =0x800;

class CImgContext : public QObject
{
    Q_OBJECT

//------early deletion prevent mechanism
    private:
        int                _pendingFlag;
        QMutex             internalLock;
    public:
        int                pendingFlag(int value, int timeout = 0)
                           { if(internalLock.tryLock(timeout))
                             {
                                 int resValue = _pendingFlag;
                                 if(value == PENDING_FLAG_MARKED_FOR_DELETION)
                                 {
                                     if(_pendingFlag != PENDING_FLAG_LOCKED)
                                     {
                                         _pendingFlag = PENDING_FLAG_MARKED_FOR_DELETION;
                                         resValue = _pendingFlag;
                                     }
                                 }
                                 if(value != PENDING_FLAG_UNDEFINED)
                                    _pendingFlag = value;
                                 internalLock.unlock();
                                 return resValue;
                             }
                             return PENDING_FLAG_UNDEFINED;
                           }

    public:
        QMutex             notesLock;

//------state flag
    private:
        uint               myState;
    public:
        void               setMyState(uint state){THREAD_SAFE {myState = state;}}
        uint               getMyState(){return myState;}

//------display filter settings
    private:
        qint32             imgOffset[2];
        qint32             selectedCoords[2];
        qint32             selectedPixel[2];
        qint32             cursorPos[2];
    public:
        void               setImgOffset(axis index, qint32 value){THREAD_SAFE {imgOffset[index] = value;}}
        qint32             getImgOffset(axis index){return imgOffset[index];}
        void               setSelectedCoords(axis index, qint32 value){THREAD_SAFE {selectedCoords[index] = value;}}
        qint32             getSelectedCoords(axis index){return selectedCoords[index];}
        void               setSelectedPixel(axis index, qint32 value){THREAD_SAFE {selectedPixel[index] = value;}}
        qint32             getSelectedPixel(axis index){return selectedPixel[index];}
        void               setCursorPos(axis index, qint32 value){THREAD_SAFE {cursorPos[index] = value;}}
        qint32             getCursorPos(axis index){return cursorPos[index];}

//------zoom factor
    private:
        float              zoomFactor;
    public:
        void               setZoomFactor(float value){THREAD_SAFE {zoomFactor = value;}}
        float              getZoomFactor(){return zoomFactor;}

//------linear on-the-fly filter
    private:
        quint8             vbias[3];
        quint8             vmul[3];
        quint8             vdiv[3];

    public:
        void               setBias(channel index, quint8 value){THREAD_SAFE {vbias[index] = value;}}
        quint8             getBias(channel index){return vbias[index];}
        void               setMul(channel index, quint8 value){THREAD_SAFE {vmul[index] = value;}}
        quint8             getMul(channel index){return vmul[index];}
        void               setDiv(channel index, quint8 value){THREAD_SAFE {vdiv[index] = value;}}
        quint8             getDiv(channel index){return vdiv[index];}


//------linear filter
public:
        float              pgain[4];
        float              pbias[4];

//------synchronized refresh flags
        bool               need_thumbnail_refresh;
        bool               need_renderData_refresh;
        bool               need_progress_refresh;

//------RGBA mask
    private:
        QColor             displayMask;
    public:
        QColor             getDisplayMask(){return displayMask;}

//------image parameters
    private:
        qint32             iwidth;
        qint32             iheight;
    public:
        void               setIWidth(quint32 value){THREAD_SAFE {iwidth = value;}}
        quint32            getIWidth(){return iwidth;}
        void               setIHeight(quint32 value){THREAD_SAFE {iheight = value;}}
        quint32            getIHeight(){return iheight;}

//------native data
    public:
        QSharedPointer<CNativeData>   nativeDataPtr;

//------normalizator
    public:
        CNormalizator      myNormalizator;

//------visual data
     private:
        QImage             visualData;
     public:
        QByteArray         getVisualData()
        {
            QByteArray ret;
            for(int i = 0; i < visualData.height(); i++)
                ret.append((char*)visualData.scanLine(i), visualData.bytesPerLine());
            return ret;
        }

        bool hasAlpha(){return visualData.hasAlphaChannel();}

//------render data
     private:
        QPixmap           *renderDataPtr;
     public:
        const QPixmap     *getRenderDataPtr(){return renderDataPtr;}

//------name
    private:
        QString            myName;
    public:
        void setMyName(QString name)
        {
            myName = name;
            myThumbnail.dont_process_an_update_event = true;
            myThumbnail.setText(name);
            myThumbnail.setToolTip(name);
            myThumbnail.dont_process_an_update_event = false;
        }
        QString            getMyName(){return myName;}

//------pixel format
    public:
        QString            myPixelFormat;

//------row stride in bits
        unsigned int       rowStrideInBits;

//------source indicator
        quint8             imgSource;

//------bit fields
    private:
        quint32            flag_bitfield;
    public:
        void               setFlag(quint32 bitToSet, bool value)
                           {THREAD_SAFE {if(value)
                                             flag_bitfield |= bitToSet;
                                         else
                                             flag_bitfield &= ~bitToSet;}}
        void               setFlags(quint32 flag_bitfield)
                           {THREAD_SAFE {this->flag_bitfield = flag_bitfield;}}

        bool               getFlag(quint32 bitToSet){return (flag_bitfield & bitToSet);}
        quint32            getFlags(){return flag_bitfield;}

    public:
//------image aux notes
        QString            myNotes;

//------thumbnail
        CThumbnail         myThumbnail;

//------image context list aux. members
    private:
         QSharedPointer<CImgContext>      prev;
         QSharedPointer<CImgContext>      next;
    public:
         QSharedPointer<CImgContext>      getPrevPtr(){return prev;}
         QSharedPointer<CImgContext>      getNextPtr(){return next;}
         void                             setPrevPtr(const QSharedPointer<CImgContext> &_prev) {prev = _prev;}
         void                             setNextPtr(const QSharedPointer<CImgContext> &_next) {next = _next;}

//------active focus tracking
    private:
         quint8                           activeInPanel[2];
    public:
         void SetFocus(panelID pID)
         {
             if(myState == STATE_READY)
             {
                activeInPanel[pID] = 1;
                if(renderDataPtr == NULL)
                    produceRenderableData();
             }
         }

         void LostFocus(panelID pID)
         {
             activeInPanel[pID] = 0;

             if((activeInPanel[panelLeftTop] == 0)&&
                (activeInPanel[panelRightBottom] == 0))
             {
                 if(renderDataPtr)
                 {
                    delete renderDataPtr;
                    renderDataPtr = NULL;
                 }
             }
         }

         bool getActivePanel(panelID pID)
         {
             return (activeInPanel[pID])?true:false;
         }

//------aux info
    public:
         QString auxInfo;


//------
//------
    public:

       /*!
        *  \brief Class constructor.
        */
        CImgContext()
        {
            imgOffset[0] = imgOffset[1] = 0;

            selectedPixel[0] = selectedPixel[1] = -1;
            selectedCoords[0] = selectedCoords[1] = -1;
            cursorPos[0] = cursorPos[1] = -1;

            //visual data modifiers
            vmul[0] = vmul[1] = vmul[2] = 1;
            vdiv[0] = vdiv[1] = vdiv[2] = 1;
            vbias[0] = vbias[1] = vbias[2] = 0;

            //linear filter params

            pgain[0] = 1.0f;
            pgain[1] = 1.0f;
            pgain[2] = 1.0f;
            pgain[3] = 1.0f;

            pbias[0] = 0.0f;
            pbias[1] = 0.0f;
            pbias[2] = 0.0f;
            pbias[3] = 0.0f;


            myState = STATE_READY;

            iwidth = iheight = 0;
            myName = "(Empty)";
            myPixelFormat = "";
            zoomFactor = 1.0f;

            myThumbnail.setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom | Qt::AlignAbsolute);
            myThumbnail.setFlags(myThumbnail.flags()|Qt::ItemIsEditable);
            myThumbnail.setMyParent((void*)this);

            myNormalizator.setMyParent(reinterpret_cast<void*>(this));

            flag_bitfield = 0x81E;

            myState = STATE_BUSY;
            setMyName("LOADING");
            activeInPanel[0] = activeInPanel[1] = 0;

            renderDataPtr = NULL;
            rowStrideInBits = 0;
        }

       /*!
        *  \breif Class destructor.
        */
        ~CImgContext()
        {
        }

       /*!
        * \brief Image offset update function.
        * @param x x-delta offset
        * @param y y-delta offset
        */
        void moveViewTo(qint32 x, qint32 y)
        {
            imgOffset[0] += x;
            imgOffset[1] += y;
        }

        /*!
         * \brief A thumbnail generator.
         */

        void makeThumbnail()
        {
            THREAD_SAFE
            QPixmap thumbnail(UI_THUMBNAIL_SIZE, UI_THUMBNAIL_SIZE);
            thumbnail.fill(Qt::white);

            QPainter thumbPainter(&thumbnail);

            if(myState == STATE_READY)
            {
                thumbPainter.drawImage(0,0, visualData.scaled(UI_THUMBNAIL_SIZE, UI_THUMBNAIL_SIZE, Qt::KeepAspectRatio));
            }
            else if(myState == STATE_BUSY)
            {
                QPixmap tmpPM = QPixmap(":/icos/wait.png");
                thumbPainter.drawPixmap(0,0, tmpPM.scaled(UI_THUMBNAIL_SIZE, UI_THUMBNAIL_SIZE, Qt::KeepAspectRatio));
            }
            else if(myState == STATE_BAD)
            {
                QPixmap tmpPM = QPixmap(":/icos/corrupted.png");
                thumbPainter.drawPixmap(0,0, tmpPM.scaled(UI_THUMBNAIL_SIZE, UI_THUMBNAIL_SIZE, Qt::KeepAspectRatio));
            }

            myThumbnail.dont_process_an_update_event = true;
            myThumbnail.setFlags(myThumbnail.flags()&~Qt::ItemIsSelectable);
            myThumbnail.setIcon(QIcon(thumbnail));
            need_thumbnail_refresh = false;

            if(myState == STATE_READY)
                myThumbnail.setFlags(myThumbnail.flags() | Qt::ItemIsSelectable);
            myThumbnail.dont_process_an_update_event = false;
        }

        /*!
         * \brief Native data attacher.
         * \param nativeDataPtr - a pointer for a native data object to attach.
         */
        void attachNativeData(const QSharedPointer<CNativeData> _nativeDataPtr)
        {
             nativeDataPtr = _nativeDataPtr;
        }


       /*!
        * \brief   File image loader.
        * @param   filename QFileInfo object
        * @return  success flag (RES_OK/RES_ERROR)
        */

        int loadFromFile(QFileInfo filename)
        {
            setMyName(filename.fileName());

            if(!visualData.load(filename.absoluteFilePath()))
                return RES_ERROR;

            iwidth  = visualData.width();
            iheight = visualData.height();

            visualData = visualData.convertToFormat(QImage::Format_ARGB32_Premultiplied);

            myNotes = "Loaded from: " + filename.absolutePath();
            myPixelFormat = "B8G8R8A8";
            myState = STATE_READY;
            imgSource = SOURCE_FILE;
            return RES_OK;
        }

        /*!
         * \brief   File image writer (graphics format).
         * \param   filename QFileInfo object
         * \return  success flag (RES_OK/RES_ERROR)
         */

         int saveToGraphicsFile(const QString &filename)
         {
             THREAD_SAFE
             return (visualData.save(filename))?RES_OK:RES_ERROR;
         }

         /*!
          * \brief  File image writer (raw format).
          * \param  filename QFileInfo object
          * \return  success flag (RES_OK/RES_ERROR)
          */

         #define WRITE_AND_VERIFY(what, how_big)\
             if(how_big != ofile.write((const char*)what, how_big))\
                goto __EXIT_WITH_ERROR;

         int saveToRICFile(const QString &filename)
         {
             THREAD_SAFE
             QFile   ofile(filename);
             dHeader header;
             if(!ofile.open(QIODevice::WriteOnly))
                 return RES_ERROR;

             WRITE_AND_VERIFY(magichars, MAGIC_CHARS_SIZE);

             header.width = this->iwidth;
             header.height = this->iheight;
             header.formatStrLength = this->myPixelFormat.size();
             if(imgSource == SOURCE_RAW)
             {
                if(!nativeDataPtr.isNull())
                    header.sizeInBytes = this->nativeDataPtr->getData().size();
                else
                    goto __EXIT_WITH_ERROR;
             }
             else
                header.sizeInBytes = this->visualData.height()*this->visualData.bytesPerLine();

             header.nameLength = this->myName.size();
             header.notesLength = this->myNotes.size();
             header.rowStrideInBits = rowStrideInBits;
             header.auxFiltering = 0;

             header.normGain[0] = pgain[0];
             header.normGain[1] = pgain[1];
             header.normGain[2] = pgain[2];
             header.normGain[3] = pgain[3];

             header.normBias[0] = pbias[0];
             header.normBias[1] = pbias[1];
             header.normBias[2] = pbias[2];
             header.normBias[3] = pbias[3];

             WRITE_AND_VERIFY((char*)&header, sizeof(dHeader));
             WRITE_AND_VERIFY(myPixelFormat.toLatin1().data(), myPixelFormat.size());
             WRITE_AND_VERIFY(myName.toLatin1().data(), myName.size());
             WRITE_AND_VERIFY(myNotes.toLatin1().data(),  myNotes.size());

             if(imgSource == SOURCE_RAW)
             {
                WRITE_AND_VERIFY(nativeDataPtr->getData().data(), nativeDataPtr->getData().size());
             }
             else
             {
                 for(int i = 0; i < visualData.height(); i++)
                    WRITE_AND_VERIFY(visualData.scanLine(i), visualData.bytesPerLine());
             }

             ofile.close();
             return RES_OK;
         __EXIT_WITH_ERROR:
             ofile.close();
             return RES_ERROR;
         }

        /*!
         * \brief Loads an image from a byte array.
         * GUI Thread SAFE.
         * \param headerRef header reference
         * \param formaStr  string describing the pixelformat
         * \param name      image name string
         * \param noteStr   image notes string
         *
         * @return  success flag (RES_OK/RES_ERROR)
         */

        int loadFromNativeData(const dHeader &headerRef,
                               QString formatStr,
                               QString nameStr,
                               QString notesStr)
        {
            return loadFromNativeData(headerRef.width,
                                      headerRef.height,
                                      headerRef.rowStrideInBits,
                                      formatStr,
                                      nameStr,
                                      notesStr,
                                      headerRef.normGain,
                                      headerRef.normBias,
                                      headerRef.auxFiltering);
        }

        int loadFromNativeData(quint32       width,
                               quint32       height,
                               quint32       rowStrideInBits,
                               QString       formatStr,
                               QString       nameStr,
                               QString       notesStr,
                               const float   gain[4],
                               const float   bias[4],
                               quint32       auxFilteringFlags)
        {
            iwidth = width;
            iheight = height;
            myNotes = notesStr;
            myPixelFormat = formatStr;
            this->rowStrideInBits = rowStrideInBits;

            setMyName(nameStr);
            imgSource = SOURCE_RAW;

            if(nativeDataPtr.isNull())
                return RES_ERROR;

            //Initialize a normalizator.

            CBitParser myFormatParser;
            if(myFormatParser.parse(&myNormalizator, formatStr) != RES_OK)
            {
                myNotes = "Format string parsing error: \n";
                myNotes += myFormatParser.lastLog;
                return RES_ERROR;
            }
            else
            {
                //check buffer size
                if((iwidth*iheight*myNormalizator.getEffectiveBitCount()+rowStrideInBits*iheight)/8 > quint32(nativeDataPtr->getData().size()))
                {
                    myNotes = "Error: Invalid native data block size. Declared: " + QString::number((iwidth*iheight*myNormalizator.getEffectiveBitCount()+rowStrideInBits*iheight)/8)\
                            + "B, received: " + QString::number(nativeDataPtr->getData().size()) + "B.";
                    return RES_ERROR;
                }

                myNormalizator.setImageWidth(iwidth);
                myNormalizator.setImageHeight(iheight);
                myNormalizator.setNativeDataPtr(nativeDataPtr->getData().data());
                myNormalizator.setRowStride(rowStrideInBits);
                myNormalizator.setREDGain(gain[0]);
                myNormalizator.setREDBias(bias[0]);
                myNormalizator.setGREENGain(gain[1]);
                myNormalizator.setGREENBias(bias[1]);
                myNormalizator.setBLUEGain(gain[2]);
                myNormalizator.setBLUEBias(bias[2]);
                myNormalizator.setALPHAGain(gain[3]);
                myNormalizator.setALPHABias(bias[3]);

                if(auxFilteringFlags & FILTER_FLAG_AUTO_GAIN_BIAS)
                {
                    myNormalizator.calibrate();
                }

                pgain[0] = myNormalizator.getChannelGain(R);
                pgain[1] = myNormalizator.getChannelGain(G);
                pgain[2] = myNormalizator.getChannelGain(B);
                pgain[3] = myNormalizator.getChannelGain(A);

                pbias[0] = myNormalizator.getChannelBias(R);
                pbias[1] = myNormalizator.getChannelBias(G);
                pbias[2] = myNormalizator.getChannelBias(B);
                pbias[3] = myNormalizator.getChannelBias(A);

                //Check if a linear filtering is rquired.  
                if((myNormalizator.getChannelBias(R)!=0)||
                   (myNormalizator.getChannelBias(G)!=0)||
                   (myNormalizator.getChannelBias(B)!=0)||
                   (myNormalizator.getChannelBias(A)!=0)||
                   (myNormalizator.getChannelGain(R)!=1)||
                   (myNormalizator.getChannelGain(G)!=1)||
                   (myNormalizator.getChannelGain(B)!=1)||
                   (myNormalizator.getChannelGain(A)!=1))
                {
                        visualData = myNormalizator.getImageWithFiltering();
                        myNotes += "\n--------------------------------------------------\n";
                        myNotes += "###Pre-filters: Gain/Bias values###\n";
                        myNotes += "R: " + QString::number(pgain[0], 'g') + " / " + QString::number(pbias[0], 'g') +"\n";
                        myNotes += "G: " + QString::number(pgain[1], 'g') + " / " + QString::number(pbias[1], 'g') +"\n";
                        myNotes += "B: " + QString::number(pgain[2], 'g') + " / " + QString::number(pbias[2], 'g') +"\n";
                        myNotes += "A: " + QString::number(pgain[3], 'g') + " / " + QString::number(pbias[3], 'g') +"\n";
                        myNotes += "--------------------------------------------------\n";
                }
                   else
                        visualData = myNormalizator.getImage();

               need_renderData_refresh = true;
            }

            //need_thumbnail_refresh = true;
            need_progress_refresh = true;

            setMyName(myName);

            return RES_OK;
        }

        /*!
         * \brief Renderable data creator.
         */

        void produceRenderableData()
        {
           THREAD_SAFE
           if(renderDataPtr)
               delete renderDataPtr;
           renderDataPtr = new QPixmap();
           renderDataPtr->convertFromImage(visualData);
           need_renderData_refresh = false;
           applyPreFilters();
           applyOnTheFlyFilters();
        }

        /*!
         * \brief Visual data creator.
         */

        int produceVisualData()
        {
            int      iw, ih;
            QRgb*    line;

            THREAD_SAFE

            visualData = QImage(iwidth, iheight, QImage::Format_ARGB32);
            visualData.fill(Qt::white);

            for(ih = 0; ih < iheight; ih++)
            {
                line = (QRgb*)visualData.scanLine(ih);
                for(iw = 0; iw < iwidth; iw++)
                {
                    *(line + iw) = qRgba(nativeDataPtr->getData()[4*(ih*iheight + iw)],
                                         nativeDataPtr->getData()[4*(ih*iheight + iw)+1],
                                         nativeDataPtr->getData()[4*(ih*iheight + iw)+2],
                                         nativeDataPtr->getData()[4*(ih*iheight + iw)+3]);
                }
            }
            return RES_OK;
        }

       /*!
        * \brief Pixel value to string converter.
        */

       QString getPixelValueString(qint32 x, qint32 y, qint32 base, quint32* numValue = NULL)
       {
           QString colorStr;
           QString prefixStr;
           quint32 color;



           color = visualData.pixel(x, y);

           if(imgSource == SOURCE_FILE)
           {
             switch(base)
             {
                 case 10: prefixStr = "";       break;
                 case 16: prefixStr = "0x";     break;
                 default:
                     prefixStr = "";
             }

             // resource loaded from file
             colorStr = "SRC= |R:" + prefixStr + "u" + QString::number(qRed(color), base);
             colorStr += " |G:" + prefixStr + "u" + QString::number(qGreen(color), base);
             colorStr += " |B:" + prefixStr + "u" + QString::number(qBlue(color), base);
             colorStr += " |A:" + prefixStr + "u" + QString::number(qAlpha(color), base);
           }
           else if(imgSource == SOURCE_RAW)
           {
               colorStr = "SRC=";
               colorStr += myNormalizator.getPixelValueStr(x, y, base);
           }

           color = applyAlllFiltersForOnePixel(color);

           if((pgain[0]!=1.0)||
              (pgain[1]!=1.0)||
              (pgain[2]!=1.0)||
              (pgain[3]!=1.0)||
              (pbias[0]!=0.0)||
              (pbias[1]!=0.0)||
              (pbias[2]!=0.0)||
              (pbias[3]!=0.0))
               colorStr += "\nOUT*=";
           else
               colorStr += "\nOUT=";

           if(numValue)
              *numValue = color;
           if(flag_bitfield & IMGCX_RED_check)
               colorStr += " |R:" + QString::number(qRed(color)/255.0f,'f', 3);
           else
               colorStr += " |R:-.---";

           if(flag_bitfield & IMGCX_GREEN_check)
               colorStr += " |G:" +  QString::number(qGreen(color)/255.0f,'f', 3);
           else
               colorStr += " |G:-.---";

           if(flag_bitfield & IMGCX_BLUE_check)
               colorStr += " |B:" +  QString::number(qBlue(color)/255.0f,'f', 3);
           else
               colorStr += " |B:-.---";

           if(flag_bitfield & IMGCX_ALPHA_check)
               colorStr += " |A:" + QString::number(qAlpha(color)/255.0f,'f', 3);
           else
               colorStr += " |A:-.---";

           return colorStr;
       }

       /*!
        * \brief Filtering for the context info.
        */
       quint32 applyAlllFiltersForOnePixel(quint32 src_pixel)
       {
           if(!(flag_bitfield & IMGCX_ALPHA_check))
               prefilter_alphaFiller_core(&src_pixel, src_pixel);
           if(flag_bitfield & IMGCX_SWAPPED_REDGREEN_check)
               prefilter_swapRED_GREEN_core(&src_pixel, src_pixel);
           if(flag_bitfield & IMGCX_SWAPPED_REDBLUE_check)
                prefilter_swapRED_BLUE_core(&src_pixel, src_pixel);
           if(flag_bitfield & IMGCX_LINEAR_TRANSFORM_check)
               prefilter_YaXpB_core(&src_pixel, src_pixel);

           if(!(flag_bitfield & IMGCX_RED_check))
               src_pixel &= 0xFF00FFFF;
           if(!(flag_bitfield & IMGCX_GREEN_check))
               src_pixel &= 0xFFFF00FF;
           if(!(flag_bitfield & IMGCX_BLUE_check))
               src_pixel &= 0xFFFFFF00;

           return src_pixel;
       }

       /*!
        * \brief Pre-filter trigger.
        */

        void applyPreFilters()
        {
            if(visualData.isNull())
                return;

            myState = STATE_BUSY;

            QImage workData(visualData);

            int     _rows, _cols;
            uchar* pixelPtr         = workData.bits();
            int    nBytesPerLine    = workData.bytesPerLine();
            uchar * scanLine;

            for(_rows=0; _rows<iheight; _rows++)
            {
                scanLine = pixelPtr+_rows*nBytesPerLine;
                for(_cols=0; _cols<iwidth; _cols++)
                {
                    if(!(flag_bitfield & IMGCX_ALPHA_check))
                       prefilter_alphaFiller_core(&((quint32*)scanLine)[_cols], ((quint32*)scanLine)[_cols]);
                    if(flag_bitfield & IMGCX_SWAPPED_REDBLUE_check)
                       prefilter_swapRED_BLUE_core(&((quint32*)scanLine)[_cols], ((quint32*)scanLine)[_cols]);
                    if(flag_bitfield & IMGCX_SWAPPED_REDGREEN_check)
                       prefilter_swapRED_GREEN_core(&((quint32*)scanLine)[_cols], ((quint32*)scanLine)[_cols]);
                    if(flag_bitfield & IMGCX_LINEAR_TRANSFORM_check)
                       prefilter_YaXpB_core(&((quint32*)scanLine)[_cols], ((quint32*)scanLine)[_cols]);
                }
            }
            renderDataPtr->convertFromImage(workData.mirrored(flag_bitfield & IMGCX_HORIZONTAL_FLIP_check,
                                                          flag_bitfield & IMGCX_VERTICAL_FLIP_check));

            myState = STATE_READY;
        }

        /*!
         * \brief Pre-filter trigger.
         */

         void applyOnTheFlyFilters()
         {
             displayMask.setRgb(
                          (flag_bitfield&IMGCX_RED_check)?0xFF:0x00,
                          (flag_bitfield&IMGCX_GREEN_check)?0xFF:0x00,
                          (flag_bitfield&IMGCX_BLUE_check)?0xFF:0x00,
                          0xFF);
         }

       /*!
        * \brief Render data handle getter.
        */

        QPixmap* getPixMap()
        {
            return renderDataPtr;
        }

        //Image pre-filters.

       /*!
        * \brief Alpha filler
        */

        inline void prefilter_alphaFiller_core(quint32* dst_pixel, quint32 src_pixel)
        {
            *dst_pixel = (src_pixel|0xFF000000);
        }

       /*!
        * \brief RED/BLUE channel swapper.
        */

        inline void prefilter_swapRED_BLUE_core(quint32* dst_pixel, quint32 src_pixel)
        {
            *dst_pixel = ((src_pixel&0xFF000000)|
                         ((src_pixel&0x00000000FF)<<16)|
                         ((src_pixel&0x0000FF00)|
                         (src_pixel&0x00FF0000)>>16));
        }


       /*!
        * \brief RED/GREEN channel swapper.
        */

        inline void prefilter_swapRED_GREEN_core(quint32* dst_pixel, quint32 src_pixel)
        {
            *dst_pixel = ((src_pixel&0xFF000000)|
                         ((src_pixel&0x0000FF00)<<8)|
                         ((src_pixel&0x00FF0000)>>8)|
                         (src_pixel&0x000000FF));
        }


        /*!
         * \brief Linear transform Cy = a*Cx/b + c.
         */

        inline void prefilter_YaXpB_core(quint32* dst_pixel, quint32 src_pixel)
        {
            *dst_pixel =  (  (src_pixel&0xFF000000)|
                             (smuladd8((src_pixel&0x00FF0000)>>16, vmul[0], vdiv[0], vbias[0])<<16)|
                             (smuladd8((src_pixel&0x0000FF00)>>8, vmul[1], vdiv[1], vbias[1])<<8) |
                             (smuladd8((src_pixel&0x000000FF),vmul[2], vdiv[2], vbias[2])));
        }
};
#endif // IMGDATABUFFER_H
#endif // CIMGCONTEXT_H
