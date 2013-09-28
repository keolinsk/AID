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
#include <QApplication>
#include <QStringList>

#define SHOW_WARNING(message) QMessageBox::warning(&mainWindow, "Argument error.", message)

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(resources);

    Globals::fontSizeMul = 2;

    //QApplication::setGraphicsSystem("raster");
    QApplication aid_app(argc, argv);
    aidMainWindow mainWindow;

    aid_app.setStyleSheet("QPushButton::menu-indicator {\
                            subcontrol-position: right center;\
                            subcontrol-origin: padding;\
                            left: -200px;}");

    //command line argument parser
    QStringList cmdArgs = aid_app.arguments();

    for(int i = 1; i<cmdArgs.size(); i++)
    {
        if(cmdArgs.at(i) == CL_WIN_POSITION)
        {
            if(cmdArgs.size()< i+3)
            {
                SHOW_WARNING("Invalid window position.");
                break;
            }
            int x=cmdArgs.at(++i).toInt();
            int y=cmdArgs.at(++i).toInt();
            mainWindow.move(x, y);
        }
        else if(cmdArgs.at(i) == CL_WIN_SIZE)
        {
            if(cmdArgs.size()< i+3)
            {
                SHOW_WARNING("Invalid window size.");
                break;
            }
            int w=cmdArgs.at(++i).toInt();
            int h=cmdArgs.at(++i).toInt();
            mainWindow.resize(w, h);
        }
        else if(cmdArgs.at(i) == CL_COM_PORT)
        {
            if(cmdArgs.size()< i+2)
            {
                SHOW_WARNING("Invalid port argument.");
                break;
            }
            int port=cmdArgs.at(++i).toInt();
            if((port <=0)||(port > 65535))
            {
                SHOW_WARNING("Invalid port value.");
                break;
            }
            Globals::serverPort = port;
        }
        else if(cmdArgs.at(i) == CL_COM_TIMEOUT)
        {
            if(cmdArgs.size()< i+2)
            {
                SHOW_WARNING("Invalid timeout argument.");
                break;
            }
            int tt=cmdArgs.at(++i).toInt();
            if((tt <=0)||(tt > 600))
            {
                SHOW_WARNING("Invalid timeout value.");
                break;
            }
            Globals::idleSocketTimeoutInSecs = tt;
        }
        else if(cmdArgs.at(i) == CL_VIEW_HEX_VALUES)
        {
            mainWindow.menuView_HexValuesDisplay();
        }
        else if(cmdArgs.at(i) == CL_MAX_IMAGES)
        {
            if(cmdArgs.size()< i+2)
            {
                SHOW_WARNING("Invalid max images argument.");
                break;
            }
            int v=cmdArgs.at(++i).toInt();
            if((v <=0)||(v > 128))
            {
                SHOW_WARNING("Invalid max images value.");
                break;
            }
            Globals::imgCountLimit =v;
        }
        else if(cmdArgs.at(i) == CL_GLOBAL_POSITION)
        {
            mainWindow.menuView_sharedPosition();
        }
        else if(cmdArgs.at(i) == CL_GLOBAL_ZOOM)
        {
            mainWindow.menuView_sharedZoom();
        }
        else if(cmdArgs.at(i) == CL_GLOBAL_FLAGS)
        {
            mainWindow.menuView_sharedViewParams();
        }
        else if(cmdArgs.at(i) == CL_PANEL_HORIZONTAL)
        {
            mainWindow.setDualPanelHorizontalMode();
        }
        else if(cmdArgs.at(i) == CL_PANEL_VERTICAL)
        {
            mainWindow.setDualPanelVerticalMode();
        }
        else if(cmdArgs.at(i) == CL_FONT_SCALE)
        {
            if(cmdArgs.size()< i+2)
            {
                SHOW_WARNING("Invalid font scale argument.");
                break;
            }
            int v=cmdArgs.at(++i).toInt();
            if((v <=0)||(v > 10))
            {
                SHOW_WARNING("Invalid font scale value.");
                break;
            }
            Globals::fontSizeMul = v+1;
        }
    }

    QFont font = aid_app.font();
    font.setPointSize(font.pointSize()*Globals::fontSizeMul/2);
    aid_app.setFont(font);

    mainWindow.delayedInit();
    mainWindow.show();
    
    return aid_app.exec();
}
