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

#ifndef aidMainWindow_H
#define aidMainWindow_H

#include "qwDecoratedCanvas.h"
#include "qwStatusBar.h"
#include "CTcpServer.h"

#ifdef QT4_HEADERS
    #include <QMainWindow>
    #include <QListWidget>
    #include <QVBoxLayout>
    #include <QSplitter>
    #include <QToolBar>
    #include <QFileDialog>
#elif QT5_HEADERS
    #include <QtWidgets/QMainWindow>
    #include <QtWidgets/QListWidget>
    #include <QtWidgets/QVBoxLayout>
    #include <QtWidgets/QSplitter>
    #include <QtWidgets/QToolbar>
    #include <QtWidgets/QFileDialog>
#endif

#include <QPointer>

class aidMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    aidMainWindow(QWidget *parent = 0);
    void delayedInit();
    ~aidMainWindow();

    bool isServerWorking();

public slots:
    void setSinglePanelMode();
    void setDualPanelVerticalMode();
    void setDualPanelHorizontalMode();

    void menuApp_LoadFromFile();
    void menuApp_LoadRAW();
    void menuApp_SaveAs(){menuApp_SaveAs(Globals::activePanel);}
    void menuApp_RemoveAll();

    void menuView_sharedViewParams();
    void menuView_sharedPosition();
    void menuView_sharedZoom();
    void menuView_ChangeAutoScaleOnLoad();
    void menuView_ChangeImageCountLimit();
    void menuView_HexValuesDisplay();
    void menuView_ShowToolbar();

    void menuNetwork_GoStop();
    void menuNetwork_ChangePortNumber();
    void menuNetwork_ChangeSocketTimeout();

    void menuTools_ReinterpretData();
    void menuTools_RecastData();
    void menuTools_DataComparator();

    void menuHelpAboutaid();
    void menuHelpAboutQt();

    void setActiveStateForStartStopButton(bool enabled);

    void menuExit();

public slots:
    void refreshThumbnailsLists();
    void commandExecutor();
    void mimicMouseMoveOnTheOtherPanel(qwGridCanvas* submitter, QPoint p);
    void mimicWheelOnTheOtherPanel(qwGridCanvas* submitter, QWheelEvent *event);

private:

    QPointer<qwDecoratedCanvas> leftTopPanelPtr;
    QPointer<qwDecoratedCanvas> rightBottomPanelPtr;

    QLabel       myCentralWidget;
    QSplitter    mySplitter;
    QVBoxLayout  myLayout;

    qwStatusBar  myStatusBar;

    QMenu        myMenu;

    CTcpServer   myTCPServer;
    QTimer       commandQueuePoller;

    void         loadFromFile();
    void         createMenu();

    bool         dualPanelMode;
    bool         verticalPanlelLayout;

    /* Menu */
    //////////
    QMenu       *menuApplication;
    QMenu       *menuTools;
    QMenu       *menuView;
    QMenu       *menuNetwork;
    QMenu       *menuHelp;

    //Application
    QAction     *actOpen;
    QAction     *actOpenRAW;
    QAction     *actSaveAs;
    QAction     *actRemoveAll;
    QAction     *actExit;

    //Tools
    QAction     *actReinterpretData;
    QAction     *actRecastData;
    QAction     *actDataComparator;

    //Help
    QAction     *actHelp;

    //View
    QAction     *actOnePanelView;
    QAction     *actDualPanelHorizontalView;
    QAction     *actDualPanelVerticalView;

    QAction     *actSharedViewParams;
    QAction     *actSharedPosition;
    QAction     *actSharedZoom;

    QAction     *actChangeAutoScaleOnLoad;
    QAction     *actHexValuesDisplay;

    QAction     *actChangeMaxImagesNumber;

    QAction     *actToolbarVisibility;

    //Network
    QAction     *actGoStop;
    QAction     *actChangePortNumber;
    QAction     *actChangeSocketTimeout;

    //About
    QAction     *actAboutaid;
    QAction     *actAboutQt;

    /* Toolbar */
    /////////////
    QToolBar    *myToolbar;

    QAction     *tactOpen;
    QAction     *tactOpenRAW;
    QAction     *tactSaveAs;
    QAction     *tactRemoveAll;

    QAction     *tactGoStop;

    QAction     *tactOnePanelView;
    QAction     *tactDualPanelHorizontalView;
    QAction     *tactDualPanelVerticalView;

    QAction     *tactSharedViewParams;
    QAction     *tactSharedPosition;
    QAction     *tactSharedZoom;


    //Other helpers
    QFileDialog  fdialog;
    void         menuApp_SaveAs(panelID pID);
};

#endif // aidMainWindow_H
