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

#ifndef QWBOTTOMPANEL_H
#define QWBOTTOMPANEL_H

#include <QtGlobal>
#include <QPointer>
#include <QMouseEvent>

#ifdef QT4_HEADERS
    #include <QPushButton>
    #include <QVBoxLayout>
    #include <QHBoxLayout>
    #include <QGridLayout>
    #include <QWidget>
#elif QT5_HEADERS
    #include <QtWidgets/QPushButton>
    #include <QtWidgets/QVBoxLayout>
    #include <QtWidgets/QHBoxLayout>
    #include <QtWidgets/QGridLayout>
    #include <QtWidgets/QWidget>
#endif

#include "qwAuxDialogs.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
/*!
 * Bottom panle class declaration.
 */

class qwBottomPanel : public QWidget
{
    Q_OBJECT

public:

    explicit qwBottomPanel(QWidget* parent = 0);
            ~qwBottomPanel();

/* Publically exposed UI elements */
    QPointer<qwSWButton>      rgbaSwitchButtonPtr[4];
    QPointer<qwSWButton>      redBlueButtonPtr;
    QPointer<qwSWButton>      redGreenButtonPtr;
    QPointer<qwSWButton>      linearTransformButtonPtr;
    QPointer<qwSWButton>      verticalFlipButtonPtr;
    QPointer<qwSWButton>      horizontalFlipButtonPtr;
    QPointer<qwSWButton>      snapToGridButtonPtr;
    QPointer<qwSWButton>      zoomInButtonPtr;
    QPointer<qwSWButton>      zoomOutButtonPtr;
    QPointer<qwSWButton>      zoomAutoButtonPtr;
    QPointer<qwSWButton>      thumbnailsBarButtonPtr;
    QPointer<qwSWButton>      backgroundButtonPtr;

    QPointer<QPushButton>     notesDataButtonPtr;

    QPointer<QPushButton>     deleteImageButtonPtr;

    QPointer<qwDialogLT>      linearTransformDialogPtr;

    qwNoteBox                 myNoteBox;

    QLabel                    infoBarString;

    QFrame                    activePanelIndicator;

private:
    QPointer<QVBoxLayout>     myLayoutPtr;
    QPointer<QVBoxLayout>     topRowPtr;
    QPointer<QHBoxLayout>     bottomRowPtr;

    QFont                     myFont;

    qwMenu                    linearTransformDialogMenu;

    QPointer<QWidgetAction>   linearTransformDialogMenuActionPtr;



protected:

    void hideEvent(QHideEvent *event)
    {
        myNoteBox.setVisible(false);
        QWidget::hideEvent(event);
    }

signals:
    void deleteImageButtonPressed();

private slots:
    void showNoteBox();

};

#endif // QWBOTTOMPANEL_H
