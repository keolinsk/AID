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

#include "./inc/aidMainWindow.h"
#include "./inc/globals.h"
#include "./inc/defines.h"
#include "./inc/CTcpServer.h"

#ifdef QT4_HEADERS
    #include <QDesktopWidget>
    #include <QAction>
    #include <QMenuBar>
    #include <QFileDialog>
    #include <QInputDialog>
#elif QT5_HEADERS
    #include <QtWidgets/QDesktopWidget>
    #include <QtWidgets/QAction>
    #include <QtWidgets/QMenuBar>
    #include <QtWidgets/QFileDialog>
    #include <QtWidgets/QInputDialog>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
aidMainWindow::aidMainWindow(QWidget *parent)
    : QMainWindow(parent)
{

    Globals::mainWindowPtr = reinterpret_cast<QMainWindow*>(this);
    Globals::statusBarPtr = reinterpret_cast<QLabel*>(&myStatusBar);

    dualPanelMode = true;
    verticalPanlelLayout = false;

    setMinimumHeight(UI_MIN_PANEL_HEIGHT+10);

    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());

    //Initial size setup.
    QDesktopWidget* desktopWidget = qApp->desktop();
    QRect screenGeometry = desktopWidget->screenGeometry();
    resize(screenGeometry.width()/1.6, screenGeometry.height()/1.6);

    setWindowIcon(QIcon(":/icos/aid.png"));
    setWindowTitle("Another image debugger");

    leftTopPanelPtr = QPointer<qwDecoratedCanvas>(new qwDecoratedCanvas(UI_SELECT_BEHAVIOR_PRESENT, panelLeftTop));
    rightBottomPanelPtr = QPointer<qwDecoratedCanvas>(new qwDecoratedCanvas(UI_SELECT_BEHAVIOR_FIRST, panelRightBottom));
    connect(leftTopPanelPtr, SIGNAL(iHaveFocus()), rightBottomPanelPtr, SLOT(childLostFocus()));
    connect(rightBottomPanelPtr, SIGNAL(iHaveFocus()), leftTopPanelPtr, SLOT(childLostFocus()));

    connect(&commandQueuePoller,SIGNAL(timeout()), this, SLOT(commandExecutor()));

    myLayout.setSpacing(0);
    myLayout.setContentsMargins(0, 0, 0, 0);
    mySplitter.addWidget(leftTopPanelPtr);
    mySplitter.addWidget(rightBottomPanelPtr);
    mySplitter.setOrientation(Qt::Horizontal);
    mySplitter.setChildrenCollapsible(false);
    myLayout.addWidget(&mySplitter);
    myLayout.addWidget(&myStatusBar);
    myCentralWidget.setLayout(&myLayout);
    setCentralWidget((QWidget*)&myCentralWidget);

    //Menu setup
    createMenu();

    setSinglePanelMode();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void  aidMainWindow::delayedInit()
{
    leftTopPanelPtr->applyFontScale();
    rightBottomPanelPtr->applyFontScale();
    commandQueuePoller.start(250);
    myTCPServer.delayedInit();
    myTCPServer.restart();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
#define REMOVE_IMG_CTX_HOOK \
if(Globals::sharedPointerHooks.count((*i).auxParam)>0)\
    Globals::sharedPointerHooks.take((*i).auxParam)

void  aidMainWindow::commandExecutor()
{
    QVector<cmdS>::iterator i;
    QVector<cmdS> artifacts;
    int pflag;

    if(!Globals::commandQueueLock.tryLock(10)) return;

    i = Globals::commandQueue.begin();
    while(i != Globals::commandQueue.end())
    {
        switch((*i).commandID)
        {
            case CMD_CREATE_RENDERABLE_DATA:
                pflag = ((CImgContext*)(*i).auxParam)->pendingFlag(PENDING_FLAG_LOCKED);

                if(pflag == PENDING_FLAG_MARKED_FOR_DELETION)
                {

                    break;
                }
                else if(pflag == PENDING_FLAG_UNDEFINED)
                {
                    artifacts.append(*i);
                    break;
                }
                else
                {
                    if(!((CImgContext*)(*i).auxParam)->need_thumbnail_refresh)
                    {
                       while(((CImgContext*)(*i).auxParam)->pendingFlag(PENDING_FLAG_RELEASED)==PENDING_FLAG_UNDEFINED); //should not stall - CMD_CREATE_RENDERABLE_DATA can be queued after processing
                       REMOVE_IMG_CTX_HOOK;
                       break;
                    }
                }

            case CMD_CREATE_THUMBNAIL:
                 pflag = ((CImgContext*)(*i).auxParam)->pendingFlag(PENDING_FLAG_LOCKED);

                 if(pflag == PENDING_FLAG_MARKED_FOR_DELETION)
                 {
                     REMOVE_IMG_CTX_HOOK;
                     break;
                 }
                 else if(pflag == PENDING_FLAG_UNDEFINED)
                 {
                     artifacts.append(*i);
                     break;
                 }
                 else
                 {

                    ((CImgContext*)(*i).auxParam)->makeThumbnail();
                    if(Globals::imgContextListLock.tryLock())
                    {
                        leftTopPanelPtr->refreshThumbnailsList(Globals::imgListHeadPtr);
                        rightBottomPanelPtr->refreshThumbnailsList(Globals::imgListHeadPtr);
                        while(((CImgContext*)(*i).auxParam)->pendingFlag(PENDING_FLAG_RELEASED)==PENDING_FLAG_UNDEFINED); //should not stall
                        Globals::imgContextListLock.unlock();

                        REMOVE_IMG_CTX_HOOK;
                    }
                    else
                        artifacts.append(*i);
                 }
            break;
            case CMD_REFRESH_VIEW_PANLES:
                leftTopPanelPtr->refreshViewPanel();
                rightBottomPanelPtr->refreshViewPanel();;
            break;

            case CMD_REFRESH_THUMBNAILS_LIST:
                 if(Globals::imgContextListLock.tryLock())
                 {
                     leftTopPanelPtr->refreshThumbnailsList(Globals::imgListHeadPtr);
                     rightBottomPanelPtr->refreshThumbnailsList(Globals::imgListHeadPtr);
                     Globals::imgContextListLock.unlock();
                 }
                 else
                    artifacts.append(*i);
            break;
            case CMD_SHOW_MSGBOX_SAVE_GFILE_FAILED:
                QMessageBox::critical(Globals::mainWindowPtr, "Error", "Failed to save to a graphics file.");
            break;
            case CMD_SHOW_MSGBOX_SAVE_RIC_FAILED:
                QMessageBox::critical(Globals::mainWindowPtr, "Error", "Failed to save to the RIC file.");
            break;
            case CMD_SHOW_MSGBOX_TOOL_SLOT_BUSY:
                QMessageBox::information(Globals::mainWindowPtr, "Tool thread slot", "It seems that one of the tools is still running.\nPlease wait until the processing ends and then try again.");
            break;
            case CMD_ADD_STATUS_MESSAGE:
                myStatusBar.popAndShow();
            break;
            case CMD_REMOVE_ALL:
                   if(Globals::imgContextListLock.tryLock())
                   {
                       QSharedPointer<CImgContext> tmpHeader = Globals::imgListHeadPtr;
                       Globals::imgListHeadPtr = QSharedPointer<CImgContext>();
                       Globals::imgCount = 0;
                       Globals::removeAll(tmpHeader);
                       Globals::commandQueue.clear();
                       Globals::sharedPointerHooks.clear();
                       leftTopPanelPtr->refreshThumbnailsList(Globals::imgListHeadPtr);
                       rightBottomPanelPtr->refreshThumbnailsList(Globals::imgListHeadPtr);
                       i = Globals::commandQueue.end();
                       Globals::imgContextListLock.unlock();
                   }
                   else
                      artifacts.append(*i);
           break;
        }

        if(i!= Globals::commandQueue.end())
            i = Globals::commandQueue.erase(i);
    }
    Globals::commandQueue.clear();
    Globals::commandQueue = artifacts;
    Globals::commandQueueLock.unlock();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void aidMainWindow::refreshThumbnailsLists()
{
    leftTopPanelPtr->refreshThumbnailsList(Globals::imgListHeadPtr);
    if(dualPanelMode)
    {
        rightBottomPanelPtr->refreshThumbnailsList(Globals::imgListHeadPtr);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
aidMainWindow::~aidMainWindow()
{
    myTCPServer.closeMe();
    commandExecutor();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void aidMainWindow::setSinglePanelMode()
{
   setMinimumWidth(UI_MIN_PANEL_WIDTH+10);
   rightBottomPanelPtr->hide();
   actOnePanelView->setIcon(QIcon(":/icos/onePanel_s.png"));
   actDualPanelHorizontalView->setIcon(QIcon(":/icos/horizontalPanel_n.png"));
   actDualPanelVerticalView->setIcon(QIcon(":/icos/verticalPanel_n.png"));
   tactOnePanelView->setIcon(QIcon(":/icos/onePanel_s.png"));
   tactDualPanelHorizontalView->setIcon(QIcon(":/icos/horizontalPanel_n.png"));
   tactDualPanelVerticalView->setIcon(QIcon(":/icos/verticalPanel_n.png"));
   leftTopPanelPtr->getGridCanvasPtr()->zoom(0,-1,-1);
   leftTopPanelPtr->childSetFocus();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void aidMainWindow::setDualPanelVerticalMode()
{
   setMinimumWidth(2*UI_MIN_PANEL_WIDTH+10);
   rightBottomPanelPtr->show();
   mySplitter.setOrientation(Qt::Vertical);
   actOnePanelView->setIcon(QIcon(":/icos/onePanel_n.png"));
   actDualPanelHorizontalView->setIcon(QIcon(":/icos/horizontalPanel_n.png"));
   actDualPanelVerticalView->setIcon(QIcon(":/icos/verticalPanel_s.png"));
   tactOnePanelView->setIcon(QIcon(":/icos/onePanel_n.png"));
   tactDualPanelHorizontalView->setIcon(QIcon(":/icos/horizontalPanel_n.png"));
   tactDualPanelVerticalView->setIcon(QIcon(":/icos/verticalPanel_s.png"));
   leftTopPanelPtr->getGridCanvasPtr()->zoom(0,-1,-1);
   rightBottomPanelPtr->getGridCanvasPtr()->zoom(0,-1,-1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void aidMainWindow::setDualPanelHorizontalMode()
{
   setMinimumWidth(2*UI_MIN_PANEL_WIDTH+10);
   rightBottomPanelPtr->show();
   mySplitter.setOrientation(Qt::Horizontal);
   actOnePanelView->setIcon(QIcon(":/icos/onePanel_n.png"));
   actDualPanelHorizontalView->setIcon(QIcon(":/icos/horizontalPanel_s.png"));
   actDualPanelVerticalView->setIcon(QIcon(":/icos/verticalPanel_n.png"));
   tactOnePanelView->setIcon(QIcon(":/icos/onePanel_n.png"));
   tactDualPanelHorizontalView->setIcon(QIcon(":/icos/horizontalPanel_s.png"));
   tactDualPanelVerticalView->setIcon(QIcon(":/icos/verticalPanel_n.png"));
   leftTopPanelPtr->getGridCanvasPtr()->zoom(0,-1,-1);
   rightBottomPanelPtr->getGridCanvasPtr()->zoom(0,-1,-1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void aidMainWindow::mimicMouseMoveOnTheOtherPanel(qwGridCanvas* submitter, QPoint p)
{
    QMouseEvent* tmpEvent = new QMouseEvent(QEvent::User,
                                            p,
                                            Qt::NoButton,
                                            Qt::NoButton,
                                            Qt::NoModifier);

    if(submitter == leftTopPanelPtr->getGridCanvasPtr())
    {
        if(leftTopPanelPtr->getSelectedImage() != rightBottomPanelPtr->getSelectedImage())
            rightBottomPanelPtr->getGridCanvasPtr()->mouseMoveEvent(tmpEvent);
        else
            rightBottomPanelPtr->getGridCanvasPtr()->refreshView();
    }
    else
    {
        if(leftTopPanelPtr->getSelectedImage() != rightBottomPanelPtr->getSelectedImage())
            leftTopPanelPtr->getGridCanvasPtr()->mouseMoveEvent(tmpEvent);
        else
            leftTopPanelPtr->getGridCanvasPtr()->refreshView();
    }

    delete tmpEvent;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void aidMainWindow::mimicWheelOnTheOtherPanel(qwGridCanvas* submitter, QWheelEvent *event)
{
    QWheelEvent* tmpEvent = new QWheelEvent(event->pos(),
                                            event->delta(),
                                            (Qt::MouseButton)0x1F,
                                            Qt::NoModifier);

    if(submitter == leftTopPanelPtr->getGridCanvasPtr())
    {
        if(leftTopPanelPtr->getSelectedImage() != rightBottomPanelPtr->getSelectedImage())
            rightBottomPanelPtr->getGridCanvasPtr()->wheelEvent(tmpEvent);
        else
            rightBottomPanelPtr->getGridCanvasPtr()->refreshView();
    }
    else
    {
        if(leftTopPanelPtr->getSelectedImage() != rightBottomPanelPtr->getSelectedImage())
            leftTopPanelPtr->getGridCanvasPtr()->wheelEvent(tmpEvent);
        else
            leftTopPanelPtr->getGridCanvasPtr()->refreshView();
    }

    delete tmpEvent;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool aidMainWindow::isServerWorking()
{
    if(myTCPServer.isWorking())
    {
        setActiveStateForStartStopButton(true);
        return true;
    }
    else
    {
        setActiveStateForStartStopButton(false);
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void aidMainWindow::createMenu()
{
    //App
    menuApplication = menuBar()->addMenu("&App");
        actOpen  = new QAction("&Load an image from file", this);
        actOpen->setIcon(QIcon(":/icos/open.png"));
        connect(actOpen, SIGNAL(triggered()), this, SLOT(menuApp_LoadFromFile()));
        menuApplication->addAction(actOpen);

        actOpenRAW  = new QAction("&Load RAW data", this);
        actOpenRAW->setIcon(QIcon(":/icos/openRAW.png"));
        connect(actOpenRAW, SIGNAL(triggered()), this, SLOT(menuApp_LoadRAW()));
        menuApplication->addAction(actOpenRAW);

        actSaveAs  = new QAction("Save panel content as", this);
        actSaveAs->setIcon(QIcon(":/icos/save.png"));
        connect(actSaveAs, SIGNAL(triggered()), this, SLOT(menuApp_SaveAs()));
        menuApplication->addAction(actSaveAs);

        actRemoveAll = new QAction("Remove all", this);
        actRemoveAll->setIcon(QIcon(":/icos/remove_all.png"));
        connect(actRemoveAll, SIGNAL(triggered()), this, SLOT(menuApp_RemoveAll()));
        menuApplication->addAction(actRemoveAll);

        menuApplication->addSeparator();

        actExit  = new QAction("&Exit", this);
        actExit->setIcon(QIcon(":/icos/exit.png"));
        connect(actExit, SIGNAL(triggered()), this, SLOT(menuExit()));
        menuApplication->addAction(actExit);

    //View
    menuView = menuBar()->addMenu("&View");

        actSharedViewParams = new QAction("Global params", this);
        actSharedViewParams->setIcon(QIcon(":/icos/sharedParams_off.png"));
        connect(actSharedViewParams, SIGNAL(triggered()), this, SLOT(menuView_sharedViewParams()));
        menuView->addAction(actSharedViewParams);

        actSharedPosition = new QAction("Global position", this);
        actSharedPosition->setIcon(QIcon(":/icos/sharedPosition_off.png"));
        connect(actSharedPosition, SIGNAL(triggered()), this, SLOT(menuView_sharedPosition()));
        menuView->addAction(actSharedPosition);

        actSharedZoom = new QAction("Global zoom", this);
        actSharedZoom->setIcon(QIcon(":/icos/sharedZoom_off.png"));
        connect(actSharedZoom, SIGNAL(triggered()), this, SLOT(menuView_sharedZoom()));
        menuView->addAction(actSharedZoom);

        menuView->addSeparator();

        actOnePanelView = new QAction("One panel", this);
        actOnePanelView->setIcon(QIcon(":/icos/onePanel_s.png"));
        connect(actOnePanelView, SIGNAL(triggered()), this, SLOT(setSinglePanelMode()));
        menuView->addAction(actOnePanelView);

        actDualPanelHorizontalView = new QAction("Two panels horizontally", this);
        actDualPanelHorizontalView->setIcon(QIcon(":/icos/horizontalPanel_n.png"));
        connect(actDualPanelHorizontalView, SIGNAL(triggered()), this, SLOT(setDualPanelHorizontalMode()));
        menuView->addAction(actDualPanelHorizontalView);

        actDualPanelVerticalView = new QAction("Two panels vertically", this);
        actDualPanelVerticalView->setIcon(QIcon(":/icos/verticalPanel_n.png"));
        connect(actDualPanelVerticalView, SIGNAL(triggered()), this, SLOT(setDualPanelVerticalMode()));
        menuView->addAction(actDualPanelVerticalView);

        menuView->addSeparator();

        actToolbarVisibility = new QAction("Show toolbar", this);
        actToolbarVisibility->setCheckable(true);
        actToolbarVisibility->setChecked(true);
        connect(actToolbarVisibility, SIGNAL(triggered()), this, SLOT(menuView_ShowToolbar()));
        menuView->addAction(actToolbarVisibility);

        menuView->addSeparator();

        actChangeMaxImagesNumber = new QAction("Loaded images limit", this);
        actChangeMaxImagesNumber->setIcon(QIcon(":/icos/dot.png"));
        connect(actChangeMaxImagesNumber, SIGNAL(triggered()), this, SLOT(menuView_ChangeImageCountLimit()));
        menuView->addAction(actChangeMaxImagesNumber);

        actChangeAutoScaleOnLoad = new QAction("Image auto-scale on load", this);
        actChangeAutoScaleOnLoad->setCheckable(true);
        actChangeAutoScaleOnLoad->setChecked(true);
        connect(actChangeAutoScaleOnLoad, SIGNAL(triggered()), this, SLOT(menuView_ChangeAutoScaleOnLoad()));
        menuView->addAction(actChangeAutoScaleOnLoad);

        menuView->addSeparator();

        actHexValuesDisplay = new QAction("Hex pixel value display for integers", this);
        actHexValuesDisplay->setCheckable(true);
        actHexValuesDisplay->setChecked(false);
        connect(actHexValuesDisplay, SIGNAL(triggered()), this, SLOT(menuView_HexValuesDisplay()));
        menuView->addAction(actHexValuesDisplay);

    //Network
    menuNetwork = menuBar()->addMenu("&Netwok");

        actGoStop = new QAction("Listening switch", this);
        actGoStop->setIcon(QIcon(":/icos/go.png"));
        connect(actGoStop, SIGNAL(triggered()), this, SLOT(menuNetwork_GoStop()));
        menuNetwork->addAction(actGoStop);

        actChangePortNumber = new QAction("Port number", this);
        actChangePortNumber->setIcon(QIcon(":/icos/dot.png"));
        connect(actChangePortNumber, SIGNAL(triggered()), this, SLOT(menuNetwork_ChangePortNumber()));
        menuNetwork->addAction(actChangePortNumber);


        actChangeSocketTimeout = new QAction("Idle socket timeout", this);
        actChangeSocketTimeout->setIcon(QIcon(":/icos/dot.png"));
        connect(actChangeSocketTimeout, SIGNAL(triggered()), this, SLOT(menuNetwork_ChangeSocketTimeout()));
        menuNetwork->addAction(actChangeSocketTimeout);

    //Tools
    menuTools = menuBar()->addMenu("&Tools");

        actReinterpretData = new QAction("Reinterpret data", this);
        actReinterpretData->setIcon(QIcon(":/icos/onePanelTool.png"));
        connect(actReinterpretData, SIGNAL(triggered()), this, SLOT(menuTools_ReinterpretData()));
        menuTools->addAction(actReinterpretData);

        actRecastData = new QAction("Recast data", this);
        actRecastData->setIcon(QIcon(":/icos/onePanelTool.png"));
        connect(actRecastData, SIGNAL(triggered()), this, SLOT(menuTools_RecastData()));
        menuTools->addAction(actRecastData);

        actDataComparator = new QAction("Source comparator", this);
        actDataComparator->setIcon(QIcon(":/icos/twoPanelTool.png"));
        connect(actDataComparator, SIGNAL(triggered()), this, SLOT(menuTools_DataComparator()));
        menuTools->addAction(actDataComparator);

    //Help
    menuHelp = menuBar()->addMenu("&Help");

        actAboutaid = new QAction("&About AID", this);
        actAboutaid->setIcon(QIcon(":/icos/dot.png"));
        connect(actAboutaid, SIGNAL(triggered()), this, SLOT(menuHelpAboutaid()));
        menuHelp->addAction(actAboutaid);

        actAboutQt = new QAction("&About Qt(R)", this);
        actAboutQt->setIcon(QIcon(":/icos/dot.png"));
        connect(actAboutQt, SIGNAL(triggered()), this, SLOT(menuHelpAboutQt()));
        menuHelp->addAction(actAboutQt);


    //Toolbar
    myToolbar = addToolBar("Main toolbar");
    myToolbar->setIconSize(QSize(UI_TOOLBAR_ICON_SIZE, UI_TOOLBAR_ICON_SIZE));

    tactOpen  = new QAction("&Load", this);
    tactOpen->setIcon(QIcon(":/icos/open.png"));
    connect(tactOpen, SIGNAL(triggered()), this, SLOT(menuApp_LoadFromFile()));
    myToolbar->addAction(tactOpen);

    tactOpenRAW  = new QAction("&Load RAW", this);
    tactOpenRAW->setIcon(QIcon(":/icos/openRAW.png"));
    connect(tactOpenRAW, SIGNAL(triggered()), this, SLOT(menuApp_LoadRAW()));
    myToolbar->addAction(actOpenRAW);

    tactSaveAs  = new QAction("Save as", this);
    tactSaveAs->setIcon(QIcon(":/icos/save.png"));
    connect(tactSaveAs, SIGNAL(triggered()), this, SLOT(menuApp_SaveAs()));
    myToolbar->addAction(tactSaveAs);

    tactRemoveAll = new QAction("Remove all", this);
    tactRemoveAll->setIcon(QIcon(":/icos/remove_all.png"));
    connect(tactRemoveAll, SIGNAL(triggered()), this, SLOT(menuApp_RemoveAll()));
    myToolbar->addAction(tactRemoveAll);

    myToolbar->addSeparator();

    tactGoStop = new QAction("Listening switch", this);
    tactGoStop->setIcon(QIcon(":/icos/go.png"));
    connect(tactGoStop, SIGNAL(triggered()), this, SLOT(menuNetwork_GoStop()));
    myToolbar->addAction(tactGoStop);

    myToolbar->addSeparator();

    tactSharedViewParams = new QAction("Global params", this);
    tactSharedViewParams->setIcon(QIcon(":/icos/sharedParams_off.png"));
    connect(tactSharedViewParams, SIGNAL(triggered()), this, SLOT(menuView_sharedViewParams()));
    myToolbar->addAction(tactSharedViewParams);

    tactSharedPosition = new QAction("Global position", this);
    tactSharedPosition->setIcon(QIcon(":/icos/sharedPosition_off.png"));
    connect(tactSharedPosition, SIGNAL(triggered()), this, SLOT(menuView_sharedPosition()));
    myToolbar->addAction(tactSharedPosition);

    tactSharedZoom = new QAction("Global zoom", this);
    tactSharedZoom->setIcon(QIcon(":/icos/sharedZoom_off.png"));
    connect(tactSharedZoom, SIGNAL(triggered()), this, SLOT(menuView_sharedZoom()));
    myToolbar->addAction(tactSharedZoom);

    myToolbar->addSeparator();

    tactOnePanelView = new QAction("One panel", this);
    tactOnePanelView->setIcon(QIcon(":/icos/onePanel_s.png"));
    connect(tactOnePanelView, SIGNAL(triggered()), this, SLOT(setSinglePanelMode()));
    myToolbar->addAction(tactOnePanelView);

    tactDualPanelHorizontalView = new QAction("Two panels horizontally", this);
    tactDualPanelHorizontalView->setIcon(QIcon(":/icos/horizontalPanel_n.png"));
    connect(tactDualPanelHorizontalView, SIGNAL(triggered()), this, SLOT(setDualPanelHorizontalMode()));
    myToolbar->addAction(tactDualPanelHorizontalView);

    tactDualPanelVerticalView = new QAction("Two panels vertically", this);
    tactDualPanelVerticalView->setIcon(QIcon(":/icos/verticalPanel_n.png"));
    connect(tactDualPanelVerticalView, SIGNAL(triggered()), this, SLOT(setDualPanelVerticalMode()));
    myToolbar->addAction(tactDualPanelVerticalView);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Menu slots
void aidMainWindow::menuApp_LoadFromFile()
{

    fdialog.setFileMode(QFileDialog::ExistingFile);
    fdialog.setViewMode(QFileDialog::Detail);

    QFileInfo fileName = fdialog.getOpenFileName(this,
         "Open", QDir::home().canonicalPath(), "Raw image container (*.ric);;Graphics files (*.png *.jpg *.jpeg *.tiff *.bmp)");

    if(!fileName.isFile())
        return;

    if(fileName.suffix().toLower() == "ric")
    {
        CWorker_loadFromRICFile* newWorker = new CWorker_loadFromRICFile(fileName.absoluteFilePath());
        newWorker->selfStart();
    }
    else
    {
        //Create a new CImgContext object.
        QSharedPointer<CImgContext> newImgContextPtr = QSharedPointer<CImgContext>(new CImgContext());
        newImgContextPtr->makeThumbnail();
        Globals::addImage(newImgContextPtr);
        refreshThumbnailsLists();
        CWorker_loadFromGraphicsFile* newWorker = new CWorker_loadFromGraphicsFile(newImgContextPtr,
                                                                               fileName.absoluteFilePath());
        newWorker->selfStart();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void aidMainWindow::menuApp_LoadRAW()
{

    fdialog.setFileMode(QFileDialog::ExistingFile);
    fdialog.setViewMode(QFileDialog::Detail);

    QFileInfo fileName = fdialog.getOpenFileName(this,
         "Open", QDir::home().canonicalPath(), "All files (*.*)");

    if(!fileName.isFile())
        return;

    qwRawHeaderEditor *rawHeaderEditorPtr;
    rawHeaderEditorPtr = new qwRawHeaderEditor(NULL, fileName);
    rawHeaderEditorPtr->show();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void aidMainWindow::menuApp_SaveAs(panelID pID)
{

    if(!Globals::imgContextListLock.tryLock(100))
        return;

    //grab a shared pointer
    QSharedPointer<CImgContext> imgPtr = (pID == panelLeftTop)?leftTopPanelPtr->getSelectedImage():rightBottomPanelPtr->getSelectedImage();
    Globals::imgContextListLock.unlock();

    if(imgPtr.isNull())
    {
        showStatusMessage("Selected view panel is empty.", UI_STATUS_ERROR, true);
        return;
    }

    fdialog.setFileMode(QFileDialog::AnyFile);
    fdialog.setViewMode(QFileDialog::Detail);

    QString filterStr = "Raw image container (*.ric);;";
     filterStr += "Portable Network Graphics (*.png);;";
     filterStr += "Joint Photographic Experts Group (*.jpg);;";
     filterStr += "Windows Bitmap (*.bmp)";

    QString fileNameStr = fdialog.getSaveFileName(this,
                                                  "Save",  QDir::home().canonicalPath(),
                                                  filterStr);

    if(fileNameStr.isEmpty())
        return;

    QFileInfo fileName(fileNameStr);

    if(fileName.suffix().toLower() == "ric")
    {
        CWorker_saveToRICFile* newWorker = new CWorker_saveToRICFile(imgPtr,
                                                   fileName.absoluteFilePath());
        if(newWorker)
            newWorker->selfStart();
    }
    else
    {
        CWorker_saveToGraphicsFile* newWorker = new CWorker_saveToGraphicsFile(imgPtr,
                                                   fileName.absoluteFilePath());
        if(newWorker)
            newWorker->selfStart();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void aidMainWindow::menuExit()
{
    Globals::myApp->quit();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void aidMainWindow::menuNetwork_GoStop()
{
    Globals::imageRecEnabled = !Globals::imageRecEnabled;

    if(Globals::imageRecEnabled)
    {
        actGoStop->setIcon(QIcon(":/icos/go.png"));
        tactGoStop->setIcon(QIcon(":/icos/go.png"));
        showStatusMessage("Image receiving is ENABLED.", UI_STATUS_NETWORK, true);
    }
    else
    {
        actGoStop->setIcon(QIcon(":/icos/stop.png"));
        tactGoStop->setIcon(QIcon(":/icos/stop.png"));
        showStatusMessage("Image receiving is DISABLED.", UI_STATUS_NETWORK, true);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void aidMainWindow::menuView_sharedViewParams()
{
    Globals::sharedViewFlagsEnabled = !Globals::sharedViewFlagsEnabled;

    if(Globals::sharedViewFlagsEnabled)
    {
        actSharedViewParams->setIcon(QIcon(":/icos/sharedParams_on.png"));
        tactSharedViewParams->setIcon(QIcon(":/icos/sharedParams_on.png"));
    }
    else
    {
        actSharedViewParams->setIcon(QIcon(":/icos/sharedParams_off.png"));
        tactSharedViewParams->setIcon(QIcon(":/icos/sharedParams_off.png"));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void aidMainWindow::menuView_sharedPosition()
{
    Globals::sharedPositionEnabled = !Globals::sharedPositionEnabled;

    if(Globals::sharedPositionEnabled)
    {
        actSharedPosition->setIcon(QIcon(":/icos/sharedPosition_on.png"));
        tactSharedPosition->setIcon(QIcon(":/icos/sharedPosition_on.png"));
    }
    else
    {
        actSharedPosition->setIcon(QIcon(":/icos/sharedPosition_off.png"));
        tactSharedPosition->setIcon(QIcon(":/icos/sharedPosition_off.png"));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void aidMainWindow::menuView_sharedZoom()
{
    Globals::sharedZoomEnabled = !Globals::sharedZoomEnabled;

    if(Globals::sharedZoomEnabled)
    {
        actSharedZoom->setIcon(QIcon(":/icos/sharedZoom_on.png"));
        tactSharedZoom->setIcon(QIcon(":/icos/sharedZoom_on.png"));
    }
    else
    {
        actSharedZoom->setIcon(QIcon(":/icos/sharedZoom_off.png"));
        tactSharedZoom->setIcon(QIcon(":/icos/sharedZoom_off.png"));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void aidMainWindow::menuApp_RemoveAll()
{
    Globals::addCmdToLocalQueue(CMD_REMOVE_ALL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void aidMainWindow::menuNetwork_ChangePortNumber()
{
    bool ok;
    int newValue =  QInputDialog::getInt(this,
                                         "Port number",
                                         "Enter a new port number (1-65535):",
                                         Globals::serverPort,
                                         1,
                                         65535,
                                         1,
                                         &ok);

    if(ok)
    {
        Globals::serverPort = newValue;
        myTCPServer.restart();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void aidMainWindow::menuView_ChangeImageCountLimit()
{
    bool ok;
    int newValue =  QInputDialog::getInt(this,
                                         "Image number limit",
                                         "Enter a new limit (1-128):",
                                         Globals::imgCountLimit,
                                         1,
                                         128,
                                         1,
                                         &ok);

    if(ok)
    {
        Globals::imgCountLimit = newValue;
        while(Globals::imgCount > Globals::imgCountLimit)
            Globals::removeLastImage();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void aidMainWindow::menuNetwork_ChangeSocketTimeout()
{
    bool ok;
    int newValue =  QInputDialog::getInt(this,
                                         "Idle socket timeout",
                                         "Enter a value (sec) (1-600):",
                                         Globals::idleSocketTimeoutInSecs,
                                         1,
                                         600,
                                         1,
                                         &ok);

    if(ok)
    {
        Globals::idleSocketTimeoutInSecs = newValue;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void aidMainWindow::menuView_ChangeAutoScaleOnLoad()
{
    Globals::autoScaleOnLoad = !Globals::autoScaleOnLoad;

    if(Globals::autoScaleOnLoad)
        actChangeAutoScaleOnLoad->setChecked(true);
    else
        actChangeAutoScaleOnLoad->setChecked(false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void aidMainWindow::menuView_HexValuesDisplay()
{
    if(actHexValuesDisplay->isChecked())
    {
        actHexValuesDisplay->setChecked(true);
        Globals::option_colorBase = 16;
    }
    else
    {
        actHexValuesDisplay->setChecked(false);
        Globals::option_colorBase = 10;
    }

    refreshThumbnailsLists();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void aidMainWindow::menuView_ShowToolbar()
{
    if(actToolbarVisibility->isChecked())
        myToolbar->setVisible(true);
    else
        myToolbar->setVisible(false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void aidMainWindow::menuTools_ReinterpretData()
{
    QMutexLocker lock(&Globals::imgContextListLock);
    qwReinterpretDialog  *reinterpretDataDialogPtr;

    if(Globals::activePanel == panelLeftTop)
    {
        if(!leftTopPanelPtr->getCurrentImage().isNull())
        {
            reinterpretDataDialogPtr = new qwReinterpretDialog(this, leftTopPanelPtr->getCurrentImage());
            reinterpretDataDialogPtr->show();
        }
        else
            showStatusMessage("Selected view panel is empty.", UI_STATUS_ERROR, true);
    }
    else if(Globals::activePanel == panelRightBottom)
    {
        if(!rightBottomPanelPtr->getCurrentImage().isNull())
        {
            reinterpretDataDialogPtr = new qwReinterpretDialog(this, rightBottomPanelPtr->getCurrentImage());
            reinterpretDataDialogPtr->show();
        }
        else
            showStatusMessage("Selected view panel is empty.", UI_STATUS_ERROR, true);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void aidMainWindow::menuTools_RecastData()
{
    QMutexLocker lock(&Globals::imgContextListLock);
    qwRecastDataDialog  *recastDataDialogPtr;

    if(Globals::activePanel == panelLeftTop)
    {
        if(!leftTopPanelPtr->getCurrentImage().isNull())
        {
            recastDataDialogPtr = new qwRecastDataDialog(this, leftTopPanelPtr->getCurrentImage());
            recastDataDialogPtr->show();
        }
        else
            showStatusMessage("Selected view panel is empty.", UI_STATUS_ERROR, true);
    }
    else if(Globals::activePanel == panelRightBottom)
    {
        if(!rightBottomPanelPtr->getCurrentImage().isNull())
        {
            recastDataDialogPtr = new qwRecastDataDialog(this, rightBottomPanelPtr->getCurrentImage());
            recastDataDialogPtr->show();
        }
        else
            showStatusMessage("Selected view panel is empty.", UI_STATUS_ERROR, true);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void aidMainWindow::menuTools_DataComparator()
{
    qwImageComparatorDialog  *imageComparatorDialogPtr;


    if(!rightBottomPanelPtr->isVisible())
    {
        QMessageBox::warning(Globals::mainWindowPtr, "Image comparator", "This tool is avaliable in two-panel view mode.");
        return;
    }

    if(leftTopPanelPtr->getCurrentImage() == rightBottomPanelPtr->getCurrentImage())
    {
        QMessageBox::warning(Globals::mainWindowPtr, "Image comparator", "The same entry is selected for both panels.");
        return;
    }


    QMutexLocker lock(&Globals::imgContextListLock);
    if((!leftTopPanelPtr->getCurrentImage().isNull())&&(!rightBottomPanelPtr->getCurrentImage().isNull()))
    {
        imageComparatorDialogPtr = new qwImageComparatorDialog(this, leftTopPanelPtr->getCurrentImage(), rightBottomPanelPtr->getCurrentImage());
        imageComparatorDialogPtr->show();
    }
    else
        showStatusMessage("One of the selected view panel is empty.", UI_STATUS_ERROR, true);

}

////////////////////////////////////////////////////////////////////////////////////////////////////
void aidMainWindow::menuHelpAboutaid()
{
    qwAboutDialog* newHandler = new qwAboutDialog();
    newHandler->show();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void aidMainWindow::menuHelpAboutQt()
{
   QApplication::aboutQt();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void aidMainWindow::setActiveStateForStartStopButton(bool enabled){
    actGoStop->setEnabled(enabled);
    if(!enabled)
    {
        Globals::imageRecEnabled = false;
        actGoStop->setIcon(QIcon(":/icos/_stop.png"));
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
