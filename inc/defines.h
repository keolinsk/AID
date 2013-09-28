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

#ifndef DEFINES_H
#define DEFINES_H

typedef unsigned int uint;
const float MAX_FLOAT = 3.40282e+038f;

/* Application command line arguments. */
const char    CL_WIN_POSITION[]                 ="-winpos";
const char    CL_WIN_SIZE[]                     ="-winsize";
const char    CL_COM_PORT[]                     ="-port";
const char    CL_COM_TIMEOUT[]                  ="-tout";
const char    CL_VIEW_HEX_VALUES[]              ="-dhex";
const char    CL_MAX_IMAGES[]                   ="-maximgs";
const char    CL_GLOBAL_POSITION[]              ="-gpos";
const char    CL_GLOBAL_ZOOM[]                  ="-gzoom";
const char    CL_GLOBAL_FLAGS[]                 ="-gflags";
const char    CL_PANEL_HORIZONTAL[]             ="-panelh";
const char    CL_PANEL_VERTICAL[]               ="-panelv";
const char    CL_FONT_SCALE[]                   ="-fontscale";



/* Comunication parameters. */

const int     COM_DEFAULT_PORT                  =5999;
const int     COM_TIMER_INTERVAL_MS             =250;
const int     COM_TIMEOUT_SEC                   =60;
const int     COM_MAX_DATA_SIZE                 =0x10004000;
const int     COM_MAX_PENDING_CONNECTIONS       =30;
const int     COM_MAX_PROCESSING_THREADS        =2;
const int     COM_CACHE                         =0xA00000;
const int     COM_ALIGN_MARGIN_SIZE             =8;
const char    COM_ALIGN_CHARS[]                 ="\0\0\0\0\0\0\0";
const char    COM_ACK_CHAR[]                    = "$";

/* Header flags and limits. */
const uint    MAX_FORMAT_STRING_LENGTH          =128;
const uint    MAX_IMG_NAME_LENGTH               =128;
const uint    MAX_IMG_NOTES_LENGTH              =4096;

const int     AUX_FILTERIN_AUTO_LINEAR          =1;
const int     AUX_FILTERING_SRGB                =2;

const int     FILTER_FLAG_AUTO_GAIN_BIAS        =0x01;

/* User interface specific parameters.*/
const uint    UI_SWBUTTON_R_CHANNEL             =0x01;
const uint    UI_SWBUTTON_G_CHANNEL             =0x02;
const uint    UI_SWBUTTON_B_CHANNEL             =0x03;
const uint    UI_SWBUTTON_A_CHANNEL             =0x04;
const uint    UI_SWBUTTON_RB_SWAP               =0x05;
const uint    UI_SWBUTTON_RG_SWAP               =0x06;
const uint    UI_MENUBUTTON_LINEAR_TRANSFORM    =0x208;
const uint    UI_SWBUTTON_HORIZONTAL_FLIP       =0x09;
const uint    UI_SWBUTTON_VERTICAL_FLIP         =0x0A;
const uint    UI_SWBUTTON_SNAP_TO_GRID          =0x0B;
const uint    UI_BUTTON_ZOOMIN                  =0x10D;
const uint    UI_BUTTON_ZOOMOUT                 =0x10E;
const uint    UI_BUTTON_ZOOMAUTO                =0x10F;
const uint    UI_SWBUTTON_THUMBNAILSBAR         =0x10;
const uint    UI_SWBUTTON_BACKGROUND            =0x11;

const uint    UI_PANEL_LEFT_TOP                 =0x0;
const uint    UI_PANEL_BOTTOM_RIGHT             =0x1;

const uint    UI_SELECT_BEHAVIOR_FIRST          =0x0;
const uint    UI_SELECT_BEHAVIOR_PRESENT        =0x1;

const uint    UI_SWITCH_BUTTON_SIZE             =32;
const uint    UI_BUTTON_SPACING_SMALL           =5;
const uint    UI_BUTTON_SPACING_LARGE           =15;
const uint    UI_INFO_FONT_SIZE                 =8;

const uint    UI_MIN_PANEL_WIDTH                =600;
const uint    UI_MIN_PANEL_HEIGHT               =300;

const uint    UI_ACTIVE_PANEL_INDICATOR_SIZE    =3;
const uint    UI_TOOLBAR_ICON_SIZE              =24;

//Aux dialogs
const int     UI_NOTEBOX_MIN_WIDTH              =600;
const int     UI_NOTEBOX_MIN_HEIGHT             =300;

const uint    MAX_IMAGE_SIZE                    =2048;
const uint    MAX_IMAGE_BLOCK_SIZE              =0x10000000;

//Thumbnail size.
const int     UI_THUMBNAIL_SIZE                 =80;

//Status bar
const int     UI_STATUS_TIP                     =0x01;
const int     UI_STATUS_INFO                    =0x02;
const int     UI_STATUS_ERROR                   =0x03;
const int     UI_STATUS_NETWORK                 =0x04;
const int     UI_STATUS_QUOTE                   =0x05;

const char    UI_SPEC_STATUS_NETWORK[]          ="#.0";
const char    UI_SPEC_STATUS_IMAGES_LOADED[]    ="#.1";

const int     UI_STATUS_REFRESH_INTERVAL        =10000;

/* Local command queue message IDs. */
const int     CMD_CREATE_RENDERABLE_DATA        =0x01;
//const int     CMD_DELETE_IMAGE_CONTEXT          =0x02;
const int     CMD_REFRESH_THUMBNAILS_LIST       =0x03;
const int     CMD_SHOW_MSGBOX_SAVE_GFILE_FAILED =0x04;
const int     CMD_SHOW_MSGBOX_SAVE_RIC_FAILED   =0x05;
const int     CMD_CREATE_THUMBNAIL              =0x06;
const int     CMD_ADD_STATUS_MESSAGE            =0x07;
const int     CMD_REMOVE_ALL                    =0x08;
const int     CMD_SHOW_MSGBOX_TOOL_SLOT_BUSY    =0x09;
const int     CMD_REFRESH_VIEW_PANLES           =0x0A;

//Message status queue
const int     MSG_STATUS_QUEUE_MAX              =20;

/* Enums. */
enum axis     {axX=0, axY};
enum channel  {R=0, G, B, A};
enum panelID  {panelLeftTop=0, panelRightBottom};


/* GridCanvas specific parameters. */
const float    ZOOM_MULTIPLIER                  =0.85f;
const float    MIN_ZOOM                         =0.005f;
const float    MAX_ZOOM                         =100.0f;
const int      BKG_COL1                         =0xFFDFDFDF;
const int      BKG_COL2                         =0xFFA0A0A0;
const float    GRID_THRESHOLD                   =10.0f;
const int      GRID_COL                         =0x00000000;
const int      GRID_BOUND_COL                   =0xFFFFFFFF;
const int      GRID_WIDTH                       =1;
const int      GRID_BOUND_WIDTH                 =2;
const int      PIXEL_SELECT_WIDTH               =3;
const int      PIXEL_SELECTOR_COL               =0xF0F0F0F0;
const uint     MAX_IMG_SIZE                     =4096;
const int      PRESS_THRESHOLD                  =2;
const int      OFFSCREEN_RENDER_MARGIN          =20;
const int      MESSAGE_FONT_SIZE                =10;
const int      PIXEL_ICON_SIZE                  =16;

/* Bit-parser */
//Tokens.
const char     FORMAT_TOKEN_ABSOLUTE_VALUE      ='a';
const char     FORMAT_TOKEN_IUNSIGNED           ='u';
const char     FORMAT_TOKEN_ISIGNED             ='i';
const char     FORMAT_TOKEN_FLOAT               ='f';
const char     FORMAT_TOKEN_sRGB                ='s';

const char     FORMAT_TOKEN_COLOR_RED           ='R';
const char     FORMAT_TOKEN_COLOR_GREEN         ='G';
const char     FORMAT_TOKEN_COLOR_BLUE          ='B';
const char     FORMAT_TOKEN_COLOR_SHARED        ='E';
const char     FORMAT_TOKEN_COLOR_ALPHA         ='A';
const char     FORMAT_TOKEN_COLOR_X             ='X';

// Validation params.
const uint     MAX_BITS_PER_CHANNEL             =32;
const uint     MAX_FRAME_SIZE                   =4096;

// Error messages.
const char     ERROR_MAX_BITS_PER_RCHANNEL[]    ="Invalid bits count for RED channel.";
const char     ERROR_MAX_BITS_PER_GCHANNEL[]    ="Invalid bits count for GREEN channel.";
const char     ERROR_MAX_BITS_PER_BCHANNEL[]    ="Invalid bits count for BLUE channel.";
const char     ERROR_MAX_BITS_PER_ACHANNEL[]    ="Invalid bits count for ALPHA channel.";

const char     ERROR_FLOAT_FORMAT_RCHANNEL[]    ="Invalid float format for RED channel.";
const char     ERROR_FLOAT_FORMAT_GCHANNEL[]    ="Invalid float format for GREEN channel.";
const char     ERROR_FLOAT_FORMAT_BCHANNEL[]    ="Invalid float format for BLUE channel.";
const char     ERROR_FLOAT_FORMAT_ACHANNEL[]    ="Invalid float format for ALPHA channel.";

const char     ERROR_MAX_FRAMSE_SIZE[]          ="Invalid frame size.";
const char     ERROR_NO_BITS_LOADED[]           ="No bits have been read.";

/* Image context pending flags. */
const int      PENDING_FLAG_RELEASED            =0;
const int      PENDING_FLAG_LOCKED              =1;
const int      PENDING_FLAG_MARKED_FOR_DELETION =-1;
const int      PENDING_FLAG_UNDEFINED           =-2;

/* Other commons. */
const int      RES_OK                           =0;
const int      RES_ERROR                        =1;


/* Tips strings. */
const char     TIP_MAIN_CHANNEL_SW[]            ="Image channel visibility switch.";
const char     TIP_MAIN_RBCHANNEL_SWAP[]        ="Swaps RED/BLUE channels.";
const char     TIP_MAIN_RGCHANNEL_SWAP[]        ="Swaps RED/GREEN channels.";
const char     TIP_MAIN_LINEAR_FILTER[]         ="RGB channels gain and bias.";
const char     TIP_MAIN_ZOOM_IN[]               ="Zoom in.";
const char     TIP_MAIN_ZOOM_OUT[]              ="Zoom out.";
const char     TIP_MAIN_ZOOM_AUTO[]             ="Auto zoom.";
const char     TIP_MAIN_FLIP_V[]                ="Vertical flip.";
const char     TIP_MAIN_FLIP_H[]                ="Horizontal flip.";
const char     TIP_MAIN_SNAP_TO_GRID[]          ="Snap to grid.";
const char     TIP_MAIN_BCKGROUND_MODE[]        ="Background mode.";
const char     TIP_MAIN_IMAGE_NOTES[]           ="Image notes.";
const char     TIP_MAIN_DELETE[]                ="Delete current image.";
const char     TIP_MAIN_SHOW_IPPBAR[]           ="Show/hide image pickup bar.";

#endif // DEFINES_H
