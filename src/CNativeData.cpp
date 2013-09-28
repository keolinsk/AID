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

#include<QDebug>

#include "./inc/CNativeData.h"
#include "./inc/commons.h"

CNativeData::CNativeData(QObject *parent):
    QObject(parent)
{
     fromRAWBuffer = NULL;
}


CNativeData::CNativeData(const QByteArray &data, QObject *parent):
    QObject(parent)
{
    fromRAWBuffer = NULL;
    this->data = data;
}

CNativeData::CNativeData(const char* rawBufferDataWithHeader, QObject *parent):
    QObject(parent)
{
    int offset;

    //Assuming the header is valid.
    offset = MAGIC_CHARS_SIZE;
    offset += sizeof(dHeader);
    offset += ((dHeader*)(rawBufferDataWithHeader+MAGIC_CHARS_SIZE))->formatStrLength;
    offset += ((dHeader*)(rawBufferDataWithHeader+MAGIC_CHARS_SIZE))->nameLength;
    offset += ((dHeader*)(rawBufferDataWithHeader+MAGIC_CHARS_SIZE))->notesLength;

    fromRAWBuffer = const_cast<char*>(rawBufferDataWithHeader);
    data.setRawData(fromRAWBuffer + offset, ((dHeader*)(rawBufferDataWithHeader + MAGIC_CHARS_SIZE))->sizeInBytes);
}

CNativeData::~CNativeData()
{
    if(fromRAWBuffer)
    {
        delete fromRAWBuffer;
    }
}
