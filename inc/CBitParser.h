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

#ifndef CBitParser_H
#define CBitParser_H

#include "CNormalizator.h"

/*!
 * \brief    The CBitParser class.
 *           The CBitParser class encapsulates functionality of a mechanism
 *           which allows to parse a given bit-stream according to a specific
 *           format determined by the <formatString>.
 *           This class has no explicit default constructor.
 *           A bit-parser error is signalized by setting an appropirate error
 *           message for the <lastLog> member.
 */

class CBitParser : public QObject{

public:
    /*!
     * \brief Bit-parser main method.
     * \param myNormalizator Pointer to a normalizator instance.
     * \param formatString String containing format description.
     * \return RES_OK if all OK; RES_ERROR otherwise (<lastLog> contains an error message).
     */
    int parse(CNormalizator* myNormalizator, QString _formatString);

    /**
     * \return Returns the pointer to a normalizator instance.
     */
    CNormalizator* getNormalizator(){return myNormalizatorPtr;}

    //Error message string.
    QString lastLog;

private:
    //Declared format type (so far).
    vType currentType;
    //The format string container.
    QString formatString;
    //Pointer to a normalizator instance.
    CNormalizator* myNormalizatorPtr;
};

#endif // CBitParser_H
