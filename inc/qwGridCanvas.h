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

#ifndef QWGRIDCANVAS_H
#define QWGRIDCANVAS_H

#include <QtGlobal>
#include <QString>
#include <QPointer>
#include <qimage.h>

#include <QPixmap>
#include <QTimer>

#ifdef QT4_HEADERS
    #include <QWidgetAction>
    #include <QWidget>
    #include <QMenu>
    #include <QHBoxLayout>
    #include <QLabel>
#elif QT5_HEADERS
    #include <QtWidgets/QWidgetAction>
    #include <QtWidgets/QWidget.h>
    #include <QtWidgets/QMenu>
    #include <QtWidgets/QHBoxLayout>
    #include <QtWidgets/QLabel>
#endif


#include "CImgContext.h"
#include "globals.h"

struct linearTransformParams
{
    float mul;
    float bias;
};

class qwGridCanvas : public QWidget
{

    Q_OBJECT

public:

        explicit                    qwGridCanvas(QWidget *parent = 0);
                                   ~qwGridCanvas();

            void                    setImgContext(const QSharedPointer<CImgContext> &_ptr);
            void                    refreshView(QString overrideString="");
            void                    showMessage(QString str);
            void                    zoom(int mag, int pposx, int pposy);
            void                    applyFontScale(){myFont.setPointSize(myFont.pointSize()*Globals::fontSizeMul/2);}

public slots:
            void                    showContextMenu(const QPoint &pos);
            void                    timerShot();


signals:
            void                    clickedIHaveBeen();
            void                    sendMouseMoveEvent(qwGridCanvas*, QPoint);
            void                    sendWheelEvent(qwGridCanvas*, QWheelEvent*);


public:
            void                    mouseMoveEvent(QMouseEvent *event);
            void                    wheelEvent(QWheelEvent *event);

protected:
    //event handlers
            void                    paintEvent(QPaintEvent *event);
            void                    mousePressEvent(QMouseEvent *event);
            void                    mouseReleaseEvent(QMouseEvent *event);
            void                    resizeEvent(QResizeEvent*e);

private:

            float                   m_zoomFactor;

            QPoint                  m_userPoint;
            QPoint                  m_pressPoint;
            QMenu                   m_myContextMenu;

            QColor                  m_backGridColor;
            QColor                  m_gridColor;

            QSharedPointer<CImgContext>   m_ImgContextPtr;
            QPixmap                 m_BkgTile;

            QImage                  m_rollerW, m_rollerH;

            qint32                  m_lastSourceX;
            qint32                  m_lastSourceY;
            qint32                  m_sourceX;
            qint32                  m_sourceY;

            qint32                  m_imgBoundaryX;
            qint32                  m_imgBoundaryY;

            qint32                  clientW, clientH, sourceW, sourceH;

    static  QImage                  s_rollerW;
    static  QImage                  s_rollerH;
    static  quint32                 s_references;
    static  QPen                    s_gridPen;
    static  QPen                    s_gridPenT;
    static  QPen                    s_pixelSelector;

            void                    calcSizeParams();
            QPoint                  whichPixel(QPoint userCoordinates);
            QString                 textMessage;

            QFont                   myFont;

    //context menu items
            QPixmap                 pixelIcon;
            QHBoxLayout             contextMenuItem;
            QLabel                  iconContainer;
            QLabel                  captionContainer;
            QPointer<QWidgetAction> contextMenuActionPtr;
            QPointer<QWidget>       contextItemConatinerPtr;

    //other helpers
            void                    displayText(QPainter &painter, QString &text);
            QTimer                  refreshTimer;
            uint                    lastImageState;
};


#endif // QWGRIDCANVAS_H
