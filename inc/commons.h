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

#ifndef __COMMONS_H__
#define __COMMONS_H__

/*!
 * Helper functions
 */

inline int min(int a, int b){if(a<b)return a; return b;}
inline int max(int a, int b){if(a>b)return a; return b;}

inline int rem(int x, int y)
{
    if(y==0)y=1;
    return (x-(x/y)*y);
}

inline int roundTo(int x, int y)
{
    if(y==0)y=1;
    return (x/y)*y;
}

inline int sign(int x)
{
    if(x>0) return 1;
    else if (x<0) return -1;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/*!
 * Protocol header definition.
 */

typedef struct
{
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
}dHeader;

const char magichars[] = "AID0";
const unsigned int MAGIC_CHARS_SIZE = 4;

#endif // COMMONS_H
