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

#include "./inc/qwGridCanvas.h"
#include "./inc/aidMainWindow.h"
#include "./inc/commons.h"
#include "./inc/globals.h"
#include "./inc/defines.h"

#include <math.h>

#include <QPainter>
#include <QWheelEvent>
#include <QtDebug>
#include <QBitmap>

#ifdef QT4_HEADERS
    #include <QWidgetAction>
    #include <QLabel>
    #include <QVBoxLayout>
#elif QT5_HEADERS
    #include <QtWidgets/QWidgetAction>
    #include <QtWidgets/QLabel>
    #include <QtWidgets/QVBoxLayout>
#endif

///////////////////////////////////////////////////////////////////////////////
// Predefined data
///////////////////////////////////////////////////////////////////////////////
#define BKG_SIZE 16
const unsigned char bkg_pattern[] = {0xFF, 0xFF, 0xFE, 0x7F, 0xFC, 0x3F, 0xF8,
                                     0x1F, 0xF0, 0x0F, 0xE0, 0x07, 0xC0, 0x03,
                                     0x80, 0x01, 0x80, 0x01, 0xC0, 0x03, 0xE0,
                                     0x07, 0xF0, 0x0F, 0xF8, 0x1F, 0xFC, 0x3F,
                                     0xFE, 0x7F, 0xFF, 0xFF};

///////////////////////////////////////////////////////////////////////////////
//Static members
///////////////////////////////////////////////////////////////////////////////
QImage   qwGridCanvas::s_rollerW;
QImage   qwGridCanvas::s_rollerH;
quint32  qwGridCanvas::s_references;
QPen     qwGridCanvas::s_gridPen;
QPen     qwGridCanvas::s_gridPenT;
QPen     qwGridCanvas::s_pixelSelector;

///////////////////////////////////////////////////////////////////////////////
//Class constructor
///////////////////////////////////////////////////////////////////////////////
qwGridCanvas::qwGridCanvas(QWidget *parent):QWidget(parent)
{
    quint32 i;

    //m_ImgContextPtrLock = QSharedPointer<QMutex>(new QMutex(QMutex::Recursive));

    //default sizing policy
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    setAttribute(Qt::WA_NoSystemBackground);

    myFont = QFont("Courier New");
    myFont.setPointSize(MESSAGE_FONT_SIZE*Globals::fontSizeMul/2);

    m_zoomFactor = 1.0f;

    textMessage = "";

    if(s_references<1)
    {
        //initialize rollers
        s_rollerW = QImage(MAX_IMG_SIZE, 1, QImage::Format_RGB32);
        s_rollerH = QImage(1, MAX_IMG_SIZE, QImage::Format_RGB32);

        //initialize grid pens
        s_gridPen  = QPen(QColor((quint32)GRID_COL), GRID_WIDTH,   Qt::DashLine);
        s_gridPenT = QPen(QColor((quint32)GRID_BOUND_COL), GRID_BOUND_WIDTH, Qt::DotLine);
        s_pixelSelector = QPen(QColor((quint32)PIXEL_SELECTOR_COL), GRID_WIDTH*4, Qt::DotLine);

        for(i=0; i< MAX_IMG_SIZE; i++)
        {
            s_rollerW.setPixel(i,0,i);
            s_rollerH.setPixel(0,i,i);
        };
    }

    //wallpaper
    QImage tmpImg(BKG_SIZE, BKG_SIZE, QImage::Format_RGB32);
    for(int _r=0;_r<BKG_SIZE;_r++)
        for(int _c=0;_c<BKG_SIZE;_c++)
            if ((*(qint16*)&bkg_pattern[_r*2])&(0x1<<_c))
                tmpImg.setPixel(_c,_r,BKG_COL1);
            else
                tmpImg.setPixel(_c,_r,BKG_COL2);
    m_BkgTile = QPixmap::fromImage(tmpImg);

    s_references++;

    m_lastSourceX = m_lastSourceY = 0;

    setContextMenuPolicy(Qt::CustomContextMenu);

    contextMenuActionPtr = new QWidgetAction(this);
    pixelIcon.convertFromImage(QImage(PIXEL_ICON_SIZE, PIXEL_ICON_SIZE, QImage::Format_RGB888));

    iconContainer.setPixmap(pixelIcon);
    iconContainer.setStyleSheet("border: 1px solid black");
    iconContainer.setFixedSize(PIXEL_ICON_SIZE, PIXEL_ICON_SIZE);
    contextMenuItem.addWidget(&iconContainer);
    contextMenuItem.addWidget(&captionContainer);

    contextItemConatinerPtr = new QWidget();
    contextItemConatinerPtr->setLayout(&contextMenuItem);

    contextMenuActionPtr->setDefaultWidget(contextItemConatinerPtr);

    m_myContextMenu.addAction(contextMenuActionPtr);

    connect(this, SIGNAL(clickedIHaveBeen()),
    parent, SLOT(refreshInfoBar()));

    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
    this, SLOT(showContextMenu(const QPoint&)));

    connect(this, SIGNAL(sendMouseMoveEvent(qwGridCanvas*, QPoint)),
    reinterpret_cast<aidMainWindow*>(Globals::mainWindowPtr), SLOT(mimicMouseMoveOnTheOtherPanel(qwGridCanvas*, QPoint)));

    connect(this, SIGNAL(sendWheelEvent(qwGridCanvas*,QWheelEvent*)),
    reinterpret_cast<aidMainWindow*>(Globals::mainWindowPtr), SLOT(mimicWheelOnTheOtherPanel(qwGridCanvas*,QWheelEvent*)));

    connect(&refreshTimer,SIGNAL(timeout()), this, SLOT(timerShot()));
    refreshTimer.start(700);
}

///////////////////////////////////////////////////////////////////////////////
//Class destructor
///////////////////////////////////////////////////////////////////////////////
qwGridCanvas::~qwGridCanvas()
{
    s_references--;
}

///////////////////////////////////////////////////////////////////////////////
//Calculations of display image boundaries
///////////////////////////////////////////////////////////////////////////////
void qwGridCanvas::calcSizeParams()
{
    qint32 wtmp;

    clientW = width()  + m_zoomFactor + OFFSCREEN_RENDER_MARGIN;
    clientH = height() + m_zoomFactor + OFFSCREEN_RENDER_MARGIN;

    if(m_ImgContextPtr == NULL)
        goto __zero_return;
    else if(m_ImgContextPtr->getRenderDataPtr() == NULL)
        goto __zero_return;

    if(m_ImgContextPtr->getImgOffset(axX)<=0)
        sourceW = (min(clientW/m_zoomFactor, m_ImgContextPtr->getRenderDataPtr()->width() + m_ImgContextPtr->getImgOffset(axX)/m_zoomFactor));
    else if(m_ImgContextPtr->getImgOffset(axX)/m_zoomFactor + m_ImgContextPtr->getRenderDataPtr()->width() -clientW/m_zoomFactor >0)
        sourceW = ((clientW- m_ImgContextPtr->getImgOffset(axX))/m_zoomFactor);
    else
        sourceW = m_ImgContextPtr->getRenderDataPtr()->width()-1;

    if(m_ImgContextPtr->getImgOffset(axY)<=0)
        sourceH = (min(clientH/m_zoomFactor, m_ImgContextPtr->getRenderDataPtr()->height() + m_ImgContextPtr->getImgOffset(axY)/m_zoomFactor));
    else if(m_ImgContextPtr->getImgOffset(axY)/m_zoomFactor + m_ImgContextPtr->getRenderDataPtr()->height() -clientH/m_zoomFactor >0)
        sourceH = ((clientH - m_ImgContextPtr->getImgOffset(axY))/m_zoomFactor);
    else
        sourceH = m_ImgContextPtr->getRenderDataPtr()->height()-1;

    wtmp =  (m_ImgContextPtr->getImgOffset(axX)>0)?0:-roundTo(m_ImgContextPtr->getImgOffset(axX), m_zoomFactor);
    wtmp /= m_zoomFactor;
    if(abs((float)m_lastSourceX - wtmp)>0.9f)
        m_lastSourceX = wtmp;

    wtmp =  (m_ImgContextPtr->getImgOffset(axY)>0)?0:-roundTo(m_ImgContextPtr->getImgOffset(axY), m_zoomFactor);
    wtmp /= m_zoomFactor;
    if(abs((float)m_lastSourceY - wtmp)>0.9f)
         m_lastSourceY = wtmp;

    m_imgBoundaryX = m_ImgContextPtr->getImgOffset(axX);

    if((m_ImgContextPtr->getImgOffset(axX)<0)&&(m_ImgContextPtr->getImgOffset(axX)>-m_zoomFactor*m_ImgContextPtr->getIWidth()))
        m_imgBoundaryX = rem(m_ImgContextPtr->getImgOffset(axX),m_zoomFactor);
    else if ((m_ImgContextPtr->getImgOffset(axX)<-m_zoomFactor*m_ImgContextPtr->getIWidth())||(m_ImgContextPtr->getImgOffset(axX)>clientW))
    {
        sourceW = -1;
        goto __exit_point;
    }

    m_imgBoundaryY =  m_ImgContextPtr->getImgOffset(axY);

    if((m_ImgContextPtr->getImgOffset(axY)<0)&&(m_ImgContextPtr->getImgOffset(axY)>=-m_zoomFactor*m_ImgContextPtr->getIHeight()))
        m_imgBoundaryY = rem(m_ImgContextPtr->getImgOffset(axY), m_zoomFactor);
    else  if ((m_ImgContextPtr->getImgOffset(axY)<-m_zoomFactor*m_ImgContextPtr->getIHeight())||(m_ImgContextPtr->getImgOffset(axY)>clientH))
    {
        sourceH = -1;
        goto __exit_point;
    }

    if(m_imgBoundaryX==0){sourceW--;}
    if(m_imgBoundaryY==0){sourceH--;}

    if((m_ImgContextPtr->getFlag(IMGCX_HORIZONTAL_FLIP_check))&&(m_ImgContextPtr->getSelectedCoords(axX)>-1))
        m_ImgContextPtr->setSelectedPixel(axX, m_ImgContextPtr->getIWidth() - m_ImgContextPtr->getSelectedCoords(axX)-1);
    else
        m_ImgContextPtr->setSelectedPixel(axX, m_ImgContextPtr->getSelectedCoords(axX));

    if((m_ImgContextPtr->getFlag(IMGCX_VERTICAL_FLIP_check))&&(m_ImgContextPtr->getSelectedCoords(axY)>-1))
        m_ImgContextPtr->setSelectedPixel(axY, m_ImgContextPtr->getIHeight() - m_ImgContextPtr->getSelectedCoords(axY)-1);
    else
        m_ImgContextPtr->setSelectedPixel(axY, m_ImgContextPtr->getSelectedCoords(axY));

    return;

__zero_return:
    m_imgBoundaryX = 0;
    m_imgBoundaryY = 0;
    sourceW = 0;
    sourceH = 0;
    m_lastSourceX = 0;
    m_lastSourceY = 0;

__exit_point:
    return;
}
///////////////////////////////////////////////////////////////////////////////
//Calculations of selected pixel coordinates
///////////////////////////////////////////////////////////////////////////////
QPoint qwGridCanvas::whichPixel(QPoint userCoordinates)
{
    if(m_ImgContextPtr.isNull())
        return QPoint(-1,-1);

    if( (userCoordinates.x()>=m_imgBoundaryX) && (userCoordinates.x() <= (m_imgBoundaryX + m_rollerW.width())) &&
        ((userCoordinates.y()>=m_imgBoundaryY) && (userCoordinates.y() <= (m_imgBoundaryY + m_rollerH.height()))))
        {
            userCoordinates.setX((qint32)(0x00FFFFFF&m_rollerW.pixel(userCoordinates.x()- m_imgBoundaryX,0)));
            userCoordinates.setY((qint32)(0x00FFFFFF&m_rollerH.pixel(0,userCoordinates.y()- m_imgBoundaryY)));

            if(m_ImgContextPtr->getFlag(IMGCX_HORIZONTAL_FLIP_check))
                userCoordinates.setX(m_ImgContextPtr->getIWidth() - userCoordinates.x()-1);

            if(m_ImgContextPtr->getFlag(IMGCX_VERTICAL_FLIP_check))
                userCoordinates.setY(m_ImgContextPtr->getIHeight() - userCoordinates.y()-1);
        }
    else
        {
            userCoordinates.setX(-1);
            userCoordinates.setY(-1);
        }

    return userCoordinates;
}
///////////////////////////////////////////////////////////////////////////////
//Event handlers
///////////////////////////////////////////////////////////////////////////////

//----user interaction

void qwGridCanvas::wheelEvent(QWheelEvent *event)
{
    if(m_ImgContextPtr == NULL)
    {
        if(event->buttons() != 0x1F)
            QWidget::wheelEvent(event);
        return;
    }

    if(m_ImgContextPtr->getMyState()!= STATE_READY)
    {
        if(event->buttons() != 0x1F)
            QWidget::wheelEvent(event);
        return;
    }

    zoom(event->delta() , event->pos().x(), event->pos().y());
    emit(clickedIHaveBeen());

    if(event->buttons() != 0x1F)
    {
        if(event->modifiers()&Qt::ControlModifier)
            emit sendWheelEvent(this, event);
        QWidget::wheelEvent(event);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void qwGridCanvas::mouseMoveEvent(QMouseEvent* event)
{
    QPoint p;

    if(m_ImgContextPtr == NULL)
    {
        if(event->type() == QEvent::User)
            QWidget::mouseMoveEvent(event);
        return;
    }

    if(event->type() == QEvent::User)
        p = event->pos();
    else
    {
        p = event->pos() - m_userPoint;
        if(event->modifiers()&Qt::ControlModifier)
            emit sendMouseMoveEvent(this, p);
    }

    if((m_ImgContextPtr->getFlag(IMGCX_SNAP_TO_GRID_check))&&(m_zoomFactor>=1))
    {
       QPoint delta;

       delta.setX(0);
       delta.setY(0);

       if(abs(p.x())>=m_zoomFactor)
       {
           m_userPoint.setX(event->pos().x());
           delta.setX(roundTo(p.x(),m_zoomFactor));
       }
       if(abs(p.y())>=m_zoomFactor)
       {
           m_userPoint.setY(event->pos().y());
           delta.setY(roundTo(p.y(),m_zoomFactor));
       }

       m_ImgContextPtr->moveViewTo(delta.x(), delta.y());
    }
    else
    {
       m_userPoint = event->pos();
       m_ImgContextPtr->moveViewTo(p.x(), p.y());
    }

    Globals::sharedPosition[0] = m_ImgContextPtr->getImgOffset(axX);
    Globals::sharedPosition[1] = m_ImgContextPtr->getImgOffset(axY);

    if(event->type() != QEvent::User)
        QWidget::mouseMoveEvent(event);

    repaint();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void qwGridCanvas::mousePressEvent(QMouseEvent *event)
{
    m_userPoint = event->pos();
    m_pressPoint = m_userPoint;

    if(m_ImgContextPtr == NULL)
        goto _EXIT_POINT;

    if(m_ImgContextPtr->getMyState()!= STATE_READY)
        goto _EXIT_POINT;

_EXIT_POINT:
    QWidget::mousePressEvent(event);
    return;

}

///////////////////////////////////////////////////////////////////////////////////////////////////
void qwGridCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint p = event->pos();
    QPoint delta = p-m_pressPoint;

    if(m_ImgContextPtr == NULL)
        goto _EXIT_POINT;

    if(m_ImgContextPtr->getMyState()!= STATE_READY)
        goto _EXIT_POINT;


    if(event->button() == Qt::LeftButton)
    {
        if(delta.manhattanLength() < PRESS_THRESHOLD)
        {
            p = whichPixel(p);
            m_ImgContextPtr->setSelectedCoords(axX, p.x());
            m_ImgContextPtr->setSelectedCoords(axY, p.y());
        }
    }
    else if(event->button() == Qt::RightButton)
    {

    }

    repaint();
    emit(clickedIHaveBeen());

_EXIT_POINT:
    QWidget::mouseReleaseEvent(event);
    return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void qwGridCanvas::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//----widged paint

inline void qwGridCanvas::displayText(QPainter &painter, QString &text)
{
    painter.setPen(Qt::white);
    painter.setFont(myFont);
    QFontMetrics fMetrics = painter.fontMetrics();
    QSize sz = fMetrics.size( Qt::TextSingleLine, text);
    QRect txtRect(0,0,sz.width(), sz.height());
    painter.fillRect(txtRect, Qt::darkBlue);
    painter.drawText( txtRect, Qt::TextSingleLine, text);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void qwGridCanvas::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QRect    sourceRect;
    qint32   markerX=-1, markerY=-1;
    quint32  itmp, jtmp;

    calcSizeParams();

    painter.beginNativePainting();


    if(m_ImgContextPtr == NULL)
    {
        if(textMessage.size()==0)
            textMessage = "Empty view";
    }
    else if(m_ImgContextPtr->getMyState() == STATE_BUSY)
    {
        textMessage = "Busy. " + m_ImgContextPtr->auxInfo;
    }
    else if((m_ImgContextPtr->getMyState() == STATE_BAD)||
            (m_ImgContextPtr->getRenderDataPtr()== NULL))
    {
        textMessage = "Not renderable";
    }
    else
    {
        textMessage = "";
    }


    if(textMessage.length()>0)
    {
        for(itmp=0; itmp<(quint32)clientW/BKG_SIZE+1; itmp++)
            for(jtmp=0; jtmp<(quint32)clientH/BKG_SIZE+1; jtmp++)
                painter.drawPixmap(itmp*BKG_SIZE,jtmp*BKG_SIZE,BKG_SIZE,BKG_SIZE,m_BkgTile);

        displayText(painter, textMessage);

        QWidget::paintEvent(event);
        return;
    }

    if(m_ImgContextPtr->getFlag(IMGCX_TRANSPARENT_BKG_check))
    for(itmp=0; itmp<(quint32)clientW/BKG_SIZE+1; itmp++)
        for(jtmp=0; jtmp<(quint32)clientH/BKG_SIZE+1; jtmp++)
            painter.drawPixmap(itmp*BKG_SIZE,jtmp*BKG_SIZE,BKG_SIZE,BKG_SIZE,m_BkgTile);
    else
        painter.fillRect(0,0,clientW,clientH, Qt::white);

    sourceRect.setRect(m_lastSourceX,
                       m_lastSourceY,
                       sourceW+1,
                       sourceH+1);

    QPixmap clampedImage = m_ImgContextPtr->getRenderDataPtr()->copy(sourceRect);
    m_rollerW = s_rollerW.copy(m_lastSourceX,0,clampedImage.width(), 1);
    m_rollerH = s_rollerH.copy(0,m_lastSourceY,1, clampedImage.height());
    clampedImage = clampedImage.scaled(m_zoomFactor*(sourceRect.width()),m_zoomFactor*(sourceRect.height()));

    m_rollerW = m_rollerW.scaled(m_zoomFactor*(sourceRect.width()),1);
    m_rollerH = m_rollerH.scaled(1,m_zoomFactor*(sourceRect.height()));


    QPainter maskPainter;
    maskPainter.begin(&clampedImage);
    maskPainter.beginNativePainting();
    maskPainter.setCompositionMode(QPainter::RasterOp_SourceAndDestination);
    maskPainter.fillRect(0,0, clampedImage.width(), clampedImage.height(),
                      m_ImgContextPtr->getDisplayMask());
    maskPainter.endNativePainting();
    maskPainter.end();

    if((sourceW<0)||(sourceH <0))
    {
        QString outOfScopeInfo = "Out of scope [";
        outOfScopeInfo += QString::number(m_imgBoundaryX) +", ";
        outOfScopeInfo += QString::number(m_imgBoundaryY) +"]";
        displayText(painter, outOfScopeInfo);
        QWidget::paintEvent(event);
        return;
    }

    painter.drawPixmap(m_imgBoundaryX,
                       m_imgBoundaryY,
                       clampedImage);

    painter.setPen(s_gridPenT);
    itmp = m_rollerW.width();
    jtmp = m_rollerH.height();

    painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    painter.drawLine(m_imgBoundaryX - GRID_BOUND_WIDTH,
                  m_imgBoundaryY - GRID_BOUND_WIDTH,
                  m_imgBoundaryX - GRID_BOUND_WIDTH,
                  m_imgBoundaryY + jtmp + GRID_BOUND_WIDTH);
    painter.drawLine(m_imgBoundaryX + itmp + GRID_BOUND_WIDTH,
                  m_imgBoundaryY - GRID_BOUND_WIDTH,
                  m_imgBoundaryX + itmp + GRID_BOUND_WIDTH,
                  m_imgBoundaryY + jtmp + GRID_BOUND_WIDTH);
    painter.drawLine(m_imgBoundaryX - GRID_BOUND_WIDTH,
                  m_imgBoundaryY - GRID_BOUND_WIDTH,
                  m_imgBoundaryX + itmp + GRID_BOUND_WIDTH,
                  m_imgBoundaryY - GRID_BOUND_WIDTH);
    painter.drawLine(m_imgBoundaryX + GRID_BOUND_WIDTH,
                  m_imgBoundaryY + jtmp + GRID_BOUND_WIDTH,
                  m_imgBoundaryX + itmp + GRID_BOUND_WIDTH,
                  m_imgBoundaryY + jtmp + GRID_BOUND_WIDTH);

    markerX=-1, markerY=-1;
    if(m_zoomFactor > GRID_THRESHOLD)
    {
        quint32  prevColor;
        qint32   range;
        quint32* p32;

        painter.setPen(s_gridPen);
        painter.setCompositionMode(QPainter::RasterOp_NotSourceXorDestination);
        p32 = (quint32*)m_rollerW.scanLine(0);

        prevColor = p32[0];
        if((qint32)(0x00FFFFFF&prevColor) == m_ImgContextPtr->getSelectedPixel(axX))
            markerX = 0;

        for(range = 1; range <= (qint32)itmp; range++)
         {
            if(prevColor != p32[range])
             {
                prevColor = p32[range];
                painter.drawLine(m_imgBoundaryX + range,
                                 m_imgBoundaryY,
                                 m_imgBoundaryX + range,
                                 m_imgBoundaryY + jtmp);
                if((qint32)(0x00FFFFFF&prevColor) == m_ImgContextPtr->getSelectedPixel(axX))
                    markerX = range;
             }
         }

        p32 = (quint32*)m_rollerH.scanLine(0);
        prevColor = p32[0];
        if((qint32)(0x00FFFFFF&prevColor) == m_ImgContextPtr->getSelectedPixel(axY))
            markerY = 0;

        for(range = 1; range <= (qint32)jtmp; range++)
         {
            p32 = (quint32*)m_rollerH.scanLine(range);
            if(prevColor != *p32)
             {
                prevColor = *p32;
                painter.drawLine(m_imgBoundaryX,
                                 m_imgBoundaryY + range,
                                 m_imgBoundaryX + itmp,
                                 m_imgBoundaryY + range);

                if((qint32)(0x00FFFFFF&prevColor) == m_ImgContextPtr->getSelectedPixel(axY))
                    markerY = range;
             }
         }
     }
    //draw selected pixel
    painter.setCompositionMode(QPainter::CompositionMode_Difference);
    if ((markerX>=0) && (markerY>=0))
    {
        painter.setPen(s_pixelSelector);

        painter.drawEllipse(m_imgBoundaryX + markerX,
                            m_imgBoundaryY + markerY,
                            m_zoomFactor,
                            m_zoomFactor
                        );
    }
    painter.endNativePainting();

    QWidget::paintEvent(event);
}


///////////////////////////////////////////////////////////////////////////////
//Public interface
///////////////////////////////////////////////////////////////////////////////

void qwGridCanvas::setImgContext(const QSharedPointer<CImgContext> &_ptr)
{
    m_ImgContextPtr = _ptr;
    if(!_ptr.isNull())
        m_zoomFactor = _ptr->getZoomFactor();
    else
        m_zoomFactor = 1.0f;

    m_lastSourceX = m_lastSourceY = 0;

}

///////////////////////////////////////////////////////////////////////////////////////////////////
void qwGridCanvas::showMessage(QString str){
    textMessage = str;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void qwGridCanvas::refreshView(QString overrideString){

    if(overrideString.size()>0)
    {
        QSharedPointer<CImgContext> tmpPtr = m_ImgContextPtr;
        m_ImgContextPtr = QSharedPointer<CImgContext>();
        textMessage = overrideString;
        repaint();
        m_ImgContextPtr = tmpPtr;
    }
    else
    {
        if(!m_ImgContextPtr.isNull())
        {
            m_zoomFactor = m_ImgContextPtr->getZoomFactor();
            if(m_zoomFactor == 0)
                zoom(0, -1, -1);
        }
        repaint();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void qwGridCanvas::showContextMenu(const QPoint& pos)
{

    QPoint globalPos = mapToGlobal(pos);
    QPoint ipos = whichPixel(pos);
    quint32 colorValue;

    if(m_ImgContextPtr == NULL)
        return;

    if(m_ImgContextPtr->getMyState()!= STATE_READY)
        return;

    if((ipos.x()<0)||(ipos.y()<0))
        return;

    captionContainer.setText("[" + QString::number(ipos.x())+
                             ", "+ QString::number(ipos.y())+"]\n"+
                             m_ImgContextPtr->getPixelValueString(ipos.x(),
                                                                  ipos.y(),
                                                                  Globals::option_colorBase,
                                                                  &colorValue));
    pixelIcon.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&pixelIcon);
    painter.beginNativePainting();
    painter.setPen(QColor(colorValue));
    painter.setOpacity(qAlpha(colorValue)/255.0f);
    painter.fillRect(pixelIcon.rect(), colorValue);
    painter.endNativePainting();
    painter.end();

    iconContainer.setPixmap(pixelIcon);

    m_myContextMenu.exec(globalPos);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void qwGridCanvas::zoom(int mag, int pposx, int pposy)
{

    if(m_ImgContextPtr.isNull())
      return;
    if((m_ImgContextPtr->getIWidth()==0) || (m_ImgContextPtr->getIHeight()==0))
      return;
    if(m_ImgContextPtr->getRenderDataPtr()==NULL)
      return;

    float  preValue = m_ImgContextPtr->getZoomFactor();

    if(pposx < 0)
    {
        if(m_ImgContextPtr->getImgOffset(axX) > 0)
            pposx = m_ImgContextPtr->getImgOffset(axX);
        else
            pposx = m_ImgContextPtr->getImgOffset(axX) + +m_ImgContextPtr->getIWidth()*m_zoomFactor;

        if(m_ImgContextPtr->getImgOffset(axY) > 0)
            pposy = m_ImgContextPtr->getImgOffset(axY);
        else
            pposy = m_ImgContextPtr->getImgOffset(axY) + +m_ImgContextPtr->getIHeight()*m_zoomFactor;
    }

    QPoint p(pposx, pposy);

    if((mag == 0)&&(isVisible()))
    {
      preValue = (float)width()/m_ImgContextPtr->getIWidth();
      if(preValue > (float)height()/m_ImgContextPtr->getIHeight())
          preValue = (float)height()/m_ImgContextPtr->getIHeight();
    }
    else if(mag < 0)
    {
        preValue *= ZOOM_MULTIPLIER;
        if(preValue*ZOOM_MULTIPLIER>1.0f)
            preValue = floor(preValue+0.5f)-1;
    }
    else
    {
        preValue /= ZOOM_MULTIPLIER;
        if(preValue>1.0f)
            preValue = floor(preValue+0.5f)+1;
    }

    if((preValue > MIN_ZOOM)&&(preValue < MAX_ZOOM))
    {

            if(mag == 0)
            {
                m_ImgContextPtr->setImgOffset(axX, Globals::sharedPositionEnabled?Globals::sharedPosition[0]:0);
                m_ImgContextPtr->setImgOffset(axY, Globals::sharedPositionEnabled?Globals::sharedPosition[1]:0);
            }
            else if(mag > 0)
            {
                m_ImgContextPtr->setImgOffset(axX,
                m_ImgContextPtr->getImgOffset(axX) + (p.x() - (m_ImgContextPtr->getImgOffset(axX)))*(m_zoomFactor/preValue-1)*ZOOM_MULTIPLIER);
                m_ImgContextPtr->setImgOffset(axY,
                m_ImgContextPtr->getImgOffset(axY) + (p.y() - (m_ImgContextPtr->getImgOffset(axY)))*(m_zoomFactor/preValue-1)*ZOOM_MULTIPLIER);
            }
            else
            {
                m_ImgContextPtr->setImgOffset(axX,
                m_ImgContextPtr->getImgOffset(axX) - (p.x() - (m_ImgContextPtr->getImgOffset(axX)))*(preValue/m_zoomFactor-1));
                m_ImgContextPtr->setImgOffset(axY,
                m_ImgContextPtr->getImgOffset(axY) - (p.y()-(m_ImgContextPtr->getImgOffset(axY)))*(preValue/m_zoomFactor-1));
            }

            if(preValue>1.0f) preValue = floor(preValue);
            m_zoomFactor = preValue;
            m_ImgContextPtr->setZoomFactor(preValue);
            Globals::sharedZoom = preValue;
    }

    if(m_ImgContextPtr->getImgOffset(axX) + m_ImgContextPtr->getRenderDataPtr()->width()*m_zoomFactor<0)
        m_ImgContextPtr->setImgOffset(axX, -(m_ImgContextPtr->getRenderDataPtr()->width()-1)*m_zoomFactor);
    else if(m_ImgContextPtr->getImgOffset(axX)>width())
        m_ImgContextPtr->setImgOffset(axX, width() - m_zoomFactor);

    if(m_ImgContextPtr->getImgOffset(axY) + m_ImgContextPtr->getRenderDataPtr()->height()*m_zoomFactor<0)
        m_ImgContextPtr->setImgOffset(axY, -(m_ImgContextPtr->getRenderDataPtr()->height()-1)*m_zoomFactor);
    else if(m_ImgContextPtr->getImgOffset(axY)>height())
        m_ImgContextPtr->setImgOffset(axY, height() - m_zoomFactor);


    Globals::sharedPosition[0] = m_ImgContextPtr->getImgOffset(axX);
    Globals::sharedPosition[1] = m_ImgContextPtr->getImgOffset(axY);

    repaint();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void qwGridCanvas::timerShot()
{

    if(m_ImgContextPtr == NULL)
      return;
    if((m_ImgContextPtr->getMyState() == STATE_BUSY)||(lastImageState == STATE_BUSY))
    {
        refreshView();
    }

    lastImageState = m_ImgContextPtr->getMyState();
}
