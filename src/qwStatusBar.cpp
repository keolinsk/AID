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

#include "./inc/qwStatusBar.h"
#include "./inc/defines.h"
#include "./inc/aidMainWindow.h"

/*!
 *Tips and hints bank.
 */

const char* statusMsgs[]=
{
    UI_SPEC_STATUS_NETWORK,
    UI_SPEC_STATUS_IMAGES_LOADED
};

const char* tipsAndHints[]=
{
    "CTRL + mouse movement/wheel - will be replicated on the other panel.",
    "Use command line arguments to start AID with your favorite configuration.",
    "Use mouse roller to change an image's zoom factor.",
    "Right click on the image shows pop-up pixel info window.",
    "Left click on the image puts a pixel marker and shows information in the bottom bar.",
    "Any complains? Mail me: keolinsk@gmail.com",
};

const char* quotesBank[]=
{
    "Anyone can build a fast processor. The trick is to build a fast system. (S.Cray)",
    "Computers are useless. They can only give you answers. (P.Picasso)",
    "Computers are like Old Testament gods; lots of rules and no mercy. (J.Campbell)",
    "Never trust a computer you can't throw out a window. (S. Wozniak)",
    "Software comes from heaven when you have good hardware. (K.Olsen)",
    "Any fool can use a computer.  Many do. (T.Nelson)",
    "Don't worry if it doesn't work right. If everything did, you'd be out of a job. (Mosher's Law)",
    "Writing in C or C++ is like running a chain saw with all the safety guards removed. (B.Gray)",
    "C++ : Where friends have access to your private members. (G.R.Baker)",
    "Good code is its own best documentation. (S.McConnell)",
    "I don't care if it works on your machine! We are not shipping your machine! (V.Platon)",
    "Hardware: the parts of a computer that can be kicked. (J.Pesis)",
    "Any sufficiently advanced bug is indistinguishable from a feature. (B.Brown)",
    "Base eight is just like base ten really, if you're missing two fingers. (T.Lehrer)",
    "Before software can be reusable it first has to be usable. (R.Johsnon)",
    "C is quirky, flawed, and an enormous success. (D.M.Ritchie)",
    "Coding styles are like a*es, everyone has one and no one likes anyone elses. (E.Warmenhoven)",
    "If the code and the comments disagree, then both are probably wrong. (N.Schryer)",
    "Logic is a systematic method of coming to the wrong conclusion with confidence. (Murphy's law)",
    "The attention span of a computer is only as long as it electrical cord. (Murphy's law)",
    "Nothing ever gets built on schedule or within budget. (Murphy's law)",
    "The first myth of management is that it exists. (Murphy's law)",
    "A failure will not appear till a unit has passed final inspection. (Murphy's law)",
    "New systems generate new problems. (Murphy's law)",
    "To err is human, but to really foul things up requires a computer. (Murphy's law)",
    "If you can't understand it, it is intuitively obvious. (Murphy's law)",
    "Any simple theory will be worded in the most complicated way. (Murphy's law)",
    "Build a system that even a fool can use and only a fool will want to use it. (Murphy's law)",
    "There is never time to do it right, but always time to do it over. (Murphy's law)",
    "Things get worse under pressure. (Murphy's Law of Thermodynamics)",
    "Matter will be damaged in direct proportion to its value. (Murphy's Constant)",
    "If anything simply cannot go wrong, it will anyway. (Murphy's law)",
    "Any given program, when running, is obsolete. (Murphy's law)",
    "If a program is useful, it will have to be changed. (Murphy's law)",
    "If a program is useless, it will have to be documented. (Murphy's law)",
    "The value of a program is inversely proportional to the weight of its output. (Murphy's law)",
    "Adding manpower to a late software project makes it later. (Murphy's law)",
    "A working program is one that has only unobserved bugs. (Murphy's law)",
    "Software bugs are impossible to detect by anybody except the end user. (Murphy's law)",
    "Every non-trivial program contains at least one bug. (Murphy's law)",
    "If a project is completed on schedule, it wasn't debugged properly. (Murphy's law)",
};

qwStatusBar::qwStatusBar(QWidget *parent):QPushButton(parent)
{
    connect(&changeMsgTrigger,SIGNAL(timeout()), this, SLOT(nextMsg()));
    changeMsgTrigger.start(UI_STATUS_REFRESH_INTERVAL);
    setStyleSheet("text-align: left; border: none; margin: 0px; padding: 0px;");
    setIcon(QIcon(":/icos/aid.png"));
    setText("Welcome! AiD greetings You.");
    showMessageFromQueue = false;
}

qwStatusBar::~qwStatusBar()
{

}

void qwStatusBar::nextMsg()
{
    int          iconIndex = 0;
    unsigned int pickIndex;
    QString      msgString;

    Globals::statusMsgQueueLock.lock();

    if(Globals::statusMsgQueue.size()>0)
    {
        iconIndex = Globals::statusMsgQueue[0].iconID;
        msgString = Globals::statusMsgQueue[0].msgStr;
        Globals::statusMsgQueue.pop_front();
    }
    else
    {
        pickIndex = qrand()%10;
        if(pickIndex<6)
        {
            pickIndex = qrand()%(sizeof(statusMsgs)/sizeof(char*));
            msgString = QString(statusMsgs[pickIndex]);
            iconIndex = UI_STATUS_INFO;
        }
        else if(pickIndex<9)
        {
            pickIndex = qrand()%(sizeof(tipsAndHints)/sizeof(char*));
            msgString = QString(tipsAndHints[pickIndex]);
            iconIndex = UI_STATUS_TIP;
        }
        else
        {
            pickIndex = qrand()%(sizeof(quotesBank)/sizeof(char*));
            msgString = QString(quotesBank[pickIndex]);
            iconIndex = UI_STATUS_QUOTE;
        }
    }
    Globals::statusMsgQueueLock.unlock();

    if(msgString == UI_SPEC_STATUS_NETWORK)
    {
        if( (reinterpret_cast<aidMainWindow*>(Globals::mainWindowPtr))->isServerWorking())
        {
            msgString = "TCP/IP: Listening on port ";
            msgString += QString::number(Globals::serverPort);
            msgString +=".";
            iconIndex = UI_STATUS_NETWORK;
        }
        else
        {
            msgString = "WARNING: TCP/IP port is closed.";
            iconIndex = UI_STATUS_ERROR;
        }
    }
    else if(msgString == UI_SPEC_STATUS_IMAGES_LOADED)
    {
        msgString = "Images loaded: ";
        msgString += QString::number(Globals::imgCount);
        iconIndex = UI_STATUS_INFO;
    }

    switch(iconIndex)
    {
        case UI_STATUS_TIP: setIcon(QIcon(":/icos/tip.png")); break;
        case UI_STATUS_INFO: setIcon(QIcon(":/icos/info.png")); break;
        case UI_STATUS_ERROR: setIcon(QIcon(":/icos/error.png")); break;
        case UI_STATUS_NETWORK: setIcon(QIcon(":/icos/network.png")); break;
        case UI_STATUS_QUOTE: setIcon(QIcon(":/icos/owl.png")); break;
        default:
            setIcon(QIcon());
    }

    setText(msgString);
}

void qwStatusBar::popAndShow()
{
     changeMsgTrigger.stop();
     showMessageFromQueue = true;
     nextMsg();
     showMessageFromQueue = false;
     changeMsgTrigger.start();
}
