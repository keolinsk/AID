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

#ifndef QWDECORATEDCANVAS_H
#define QWDECORATEDCANVAS_H

#include "CImgContext.h"
#include "qwGridCanvas.h"
#include "qwBottomPanel.h"
#include "qwPickUpList.h"
#include "qwAuxDialogs.h"

#include <QtGlobal>
#include <QString>
#include <QPointer>
#include <QReadWriteLock>
#include <qimage.h>


#ifdef QT4_HEADERS
    #include <QListWidgetItem>
    #include <QWidget>
    #include <QVBoxLayout>
    #include <QListWidget>
#elif QT5_HEADERS
    #include <QtWidgets/QListWidgetItem>
    #include <QtWidgets/QWidget.h>
    #include <QtWidgets/QVBoxLayout>
    #include <QtWidgets/QListWidget>
#endif

class qwDecoratedCanvas : public QWidget
{
   Q_OBJECT
public:

    /*!
     * \brief qwDecoratedCanvas class constructor.
     * \param defSelBvr defines deault selection after an image load/deletion:
     *                  UI_SELECT_BEHAVIOR_FIRST = switch to the latest (first in the list)
     *                  UI_SELECT_BEHAVIOR_PRESENT = stay at the currently selected image (if possible)
     * \param parent parent object
     */
    explicit                    qwDecoratedCanvas(int defSelBvr, panelID myID, QWidget *parent = 0);
                               ~qwDecoratedCanvas();

    void                        setCurrentImage(const QSharedPointer<CImgContext> &imc);
    QSharedPointer<CImgContext> getCurrentImage(){return currentImgPtr;}
    void                        refreshThumbnailsList(QSharedPointer<CImgContext> header);
    void                        refreshViewPanel(){myGridCanvasPtr->refreshView();}

    /*! Restores button states from the current image context. */
    void                        restoreSWButtonsState();

    /*! Retrieves the current image pointer. */
    QSharedPointer<CImgContext> getSelectedImage(){return currentImgPtr;}

    /*! Retrieves a grid vanvas pointer. */
    qwGridCanvas*               getGridCanvasPtr(){return myGridCanvasPtr;}

    /*! Font scaler */
    void                        applyFontScale()
                                {myGridCanvasPtr->applyFontScale();
                                 QFont font = myBottomPanelPtr->infoBarString.font();
                                 font.setPointSize(font.pointSize()*Globals::fontSizeMul/2);
                                 myBottomPanelPtr->infoBarString.setFont(font);
                                }



signals:
    void                        iHaveFocus();

public slots:
    void                        setCurrentImageSlot(QListWidgetItem* imc);
    void                        reciveSWButtonEvent(quint32, qint32);
    void                        refreshInfoBar();
    void                        deleteCurrentImage();
    void                        saveImageAs();
    void                        childLostFocus();
    void                        childSetFocus(){focusInEvent(0);}


protected:
    void                        hideEvent(QHideEvent *e);
    void                        showEvent(QShowEvent *e);
    void                        enterEvent(QEvent *e);
    void                        focusInEvent(QFocusEvent *e);

private:

    void                        disableButtons();
    void                        enableButtons();
    void                        setSharedViewFlag(quint32 bitToSet, bool value);

    panelID                     myID;

    QSharedPointer<CImgContext> imgListPtr;
    QSharedPointer<CImgContext> currentImgPtr;

    QVBoxLayout                 myLayout;

    QPointer<qwGridCanvas>      myGridCanvasPtr;
    QPointer<qwBottomPanel>     myBottomPanelPtr;
    QPointer<qwPickUpList>      myThumbnailListPtr;

    int                         defaultSelectionBehavior;
};
#endif // QWDECORATEDCANVAS_H
