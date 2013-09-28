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

#ifndef QWPICKUPLIST_H
#define QWPICKUPLIST_H

#include "CImgContext.h"
#include "qwAuxDialogs.h"

#include <QPoint>
#include <QRect>

#ifdef QT4_HEADERS
    #include <QListWidget>
#elif QT5_HEADERS
    #include <QtWidgets/QListWidget>
#endif

class qwPickUpList : public QListWidget
{
     Q_OBJECT

public:

    explicit  qwPickUpList(QWidget *parent = 0);
             ~qwPickUpList();

    void      hidePopUpMenu(){myPopUpMenuPtr->hideMe();}

signals:
    void      saveImageAs();

public slots:
    void      showContextMenu(const QPoint& p);
    void      itemNameChanged(QListWidgetItem*);
    void      popupRename();
    void      popupSaveAs();
    void      popupDelete();

protected:
    void      focusInEvent(QFocusEvent*);

private:
    QPointer<qwPopUpMenuThumbnailList> myPopUpMenuPtr;

};

#endif // QWPICKUPLIST_H
