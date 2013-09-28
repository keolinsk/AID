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

#include "./inc/qwAuxDialogs.h"
#include "./inc/commons.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
qwReinterpretDialog           *qwReinterpretDialog::myHandler                  = NULL;
qwAboutDialog                 *qwAboutDialog::myHandler                        = NULL;

////////////////////////////////////////////////////////////////////////////////////////////////////
qwImageComparatorDialog       *qwImageComparatorDialog::myHandler              = NULL;

float                          qwImageComparatorDialog::customThresholds[4]    = {0.0f};
float                          qwImageComparatorDialog::customShift[2]         = {0};
bool                           qwImageComparatorDialog::customChkcks[3]        = {false};

////////////////////////////////////////////////////////////////////////////////////////////////////
qwRecastDataDialog            *qwRecastDataDialog::myHandler                   = NULL;
const char                    *qwRecastDataDialog::predefinedTransformItems[]  = {"Custom",
                                                                              "Derived",
                                                                              "Identity",
                                                                              "CMY to RGB (A=1)",
                                                                              "YIQ to RGB (A=1)",
                                                                              "YUV to RGB (A=1)"
                                                                              };

float                          qwRecastDataDialog::customParameters[20]        = {0.0f};
bool                           qwRecastDataDialog::customChcks[3]              = {false};

////////////////////////////////////////////////////////////////////////////////////////////////////

qwRawHeaderEditor             *qwRawHeaderEditor::myHandler                    = NULL;

unsigned int width;
unsigned int height;
unsigned int formatStrLength;
unsigned int sizeInBytes;
unsigned int nameLength;
unsigned int notesLength;
unsigned int rowStrideInBits;
float        normGain[4];
float        normBias[4];
unsigned int auxFiltering;


dHeader                        qwRawHeaderEditor::rawHeader                    = {1,
                                                                                  1,
                                                                                  0,
                                                                                  0,
                                                                                  0,
                                                                                  0,
                                                                                  0,
                                                                                  {
                                                                                  1.0f,
                                                                                  1.0f,
                                                                                  1.0f,
                                                                                  1.0f},
                                                                                  {
                                                                                  0.0f,
                                                                                  0.0f,
                                                                                  0.0f,
                                                                                  0.0f},
                                                                                  0};

QString                         qwRawHeaderEditor::pixelFormatString            = "R8G8B8A8";
