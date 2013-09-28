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

#ifndef QWSTATUSBAR_H
#define QWSTATUSBAR_H

#include "globals.h"
#include "defines.h"

#include <QTimer>

#ifdef QT4_HEADERS
    #include <QPushButton>
#elif QT5_HEADERS
    #include <QtWidgets/QPushButton>
#endif

class qwStatusBar : public QPushButton
{
    Q_OBJECT

public:
    explicit  qwStatusBar(QWidget *parent = 0);
             ~qwStatusBar();
    void      popAndShow();

public slots:
    void nextMsg();

private:
    QTimer    changeMsgTrigger;
    bool      showMessageFromQueue;

};


#endif // QWSTATUSBAR_H
