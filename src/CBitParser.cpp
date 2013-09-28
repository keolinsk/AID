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

#include "./inc/CBitParser.h"
#include "./inc/commons.h"

#include<QRegExp>
#include<QDebug>

///////////////////////////////////////////////////////////////////////////////////////////////////
int CBitParser::parse(CNormalizator* myNormalizatorPtr, QString formatString)
{

    QString          tmpStr;
    QRegExp          numExp = QRegExp("^\\d+");
    int              i;
    BitIndexAndCount tmpBIC;
    bool             absValFlag = false;

    //Default format
    currentType = NORM_IUNSIGNED;
    //Default log
    lastLog = "OK.";

    tmpBIC.bitIndex = 0;

    for(i=0; i < formatString.length(); i++)
    {
          if(formatString[i] == FORMAT_TOKEN_IUNSIGNED)
          {
               currentType = NORM_IUNSIGNED;
               absValFlag = false;
          }
          else if(formatString[i] == FORMAT_TOKEN_ISIGNED)
          {
               currentType = NORM_ISIGNED;
               absValFlag = false;
          }
          else if(formatString[i] == FORMAT_TOKEN_FLOAT)
          {
               currentType = NORM_FLOAT;
               absValFlag = false;
          }
          else if(formatString[i] == FORMAT_TOKEN_sRGB)
          {
          }
          else if(formatString[i] == FORMAT_TOKEN_ABSOLUTE_VALUE)
          {
               absValFlag = true;
          }
          else if(formatString[i] == FORMAT_TOKEN_COLOR_RED)
          {
            numExp.indexIn(formatString, i+1, QRegExp::CaretAtOffset);
            tmpStr = numExp.cap();
            i+=tmpStr.length();
            tmpBIC.bitsCount = (quint32)tmpStr.toInt();
            myNormalizatorPtr->addREDBitsMask(tmpBIC);
            myNormalizatorPtr->setREDType(currentType);
            myNormalizatorPtr->setREDAbsValueFlag(absValFlag);
            tmpBIC.bitIndex += tmpBIC.bitsCount;
          }
          else if(formatString[i] == FORMAT_TOKEN_COLOR_GREEN)
          {
            numExp.indexIn(formatString, i+1, QRegExp::CaretAtOffset);
            tmpStr = numExp.cap();
            i+=tmpStr.length();
            tmpBIC.bitsCount = (quint32)tmpStr.toInt();
            myNormalizatorPtr->addGREENBitsMask(tmpBIC);
            myNormalizatorPtr->setGREENType(currentType);
            myNormalizatorPtr->setGREENAbsValueFlag(absValFlag);
            tmpBIC.bitIndex += tmpBIC.bitsCount;
          }
          else if(formatString[i] == FORMAT_TOKEN_COLOR_BLUE)
          {
            numExp.indexIn(formatString, i+1, QRegExp::CaretAtOffset);
            tmpStr = numExp.cap();
            i+=tmpStr.length();
            tmpBIC.bitsCount = (quint32)tmpStr.toInt();
            myNormalizatorPtr->addBLUEBitsMask(tmpBIC);
            myNormalizatorPtr->setBLUEType(currentType);
            myNormalizatorPtr->setBLUEAbsValueFlag(absValFlag);
            tmpBIC.bitIndex += tmpBIC.bitsCount;
          }
          else if(formatString[i] == FORMAT_TOKEN_COLOR_SHARED)
          {
            numExp.indexIn(formatString, i+1, QRegExp::CaretAtOffset);
            tmpStr = numExp.cap();
            i+=tmpStr.length();
            tmpBIC.bitsCount = (quint32)tmpStr.toInt();
            myNormalizatorPtr->addREDBitsMask(tmpBIC);
            myNormalizatorPtr->addGREENBitsMask(tmpBIC);
            myNormalizatorPtr->addBLUEBitsMask(tmpBIC);
            tmpBIC.bitIndex += tmpBIC.bitsCount;
          }
          else if(formatString[i] == FORMAT_TOKEN_COLOR_ALPHA)
          {
            numExp.indexIn(formatString, i+1, QRegExp::CaretAtOffset);
            tmpStr = numExp.cap();
            i+=tmpStr.length();
            tmpBIC.bitsCount = (quint32)tmpStr.toInt();
            myNormalizatorPtr->addALPHABitsMask(tmpBIC);
            myNormalizatorPtr->setALPHAType(currentType);
            myNormalizatorPtr->setALPHAAbsValueFlag(absValFlag);
            tmpBIC.bitIndex += tmpBIC.bitsCount;
          }
          else if(formatString[i] == FORMAT_TOKEN_COLOR_X)
          {
            numExp.indexIn(formatString, i+1, QRegExp::CaretAtOffset);
            tmpStr = numExp.cap();
            i+=tmpStr.length();
            tmpBIC.bitsCount = (quint32)tmpStr.toInt();
            myNormalizatorPtr->addXBitsMask(tmpBIC);
            tmpBIC.bitIndex += tmpBIC.bitsCount;
          }
    }

    if(tmpBIC.bitIndex==0)
    {
        lastLog = ERROR_NO_BITS_LOADED;
        return RES_ERROR;
    }
    else if(tmpBIC.bitsCount > MAX_FRAME_SIZE)
    {
        lastLog = ERROR_MAX_FRAMSE_SIZE;
        return RES_ERROR;
    }
    else if(myNormalizatorPtr->getREDBitsCount()> MAX_BITS_PER_CHANNEL)
    {
        lastLog = ERROR_MAX_BITS_PER_RCHANNEL;
        return RES_ERROR;
    }
    else if(myNormalizatorPtr->getGREENBitsCount()> MAX_BITS_PER_CHANNEL)
    {
        lastLog = ERROR_MAX_BITS_PER_GCHANNEL;
        return RES_ERROR;
    }
    else if(myNormalizatorPtr->getBLUEBitsCount()> MAX_BITS_PER_CHANNEL)
    {
        lastLog = ERROR_MAX_BITS_PER_BCHANNEL;
        return RES_ERROR;
    }
    else if(myNormalizatorPtr->getALPHABitsCount()> MAX_BITS_PER_CHANNEL)
    {
        lastLog = ERROR_MAX_BITS_PER_ACHANNEL;
        return RES_ERROR;
    }
    else if((myNormalizatorPtr->getREDType()==NORM_ISIGNED) && (myNormalizatorPtr->getREDBitsCount()<2))
    {
        lastLog = ERROR_MAX_BITS_PER_RCHANNEL;
        return RES_ERROR;
    }
    else if((myNormalizatorPtr->getGREENType()==NORM_ISIGNED) && (myNormalizatorPtr->getGREENBitsCount()<2))
    {
        lastLog = ERROR_MAX_BITS_PER_GCHANNEL;
        return RES_ERROR;
    }
    else if((myNormalizatorPtr->getBLUEType()==NORM_ISIGNED) && (myNormalizatorPtr->getBLUEBitsCount()<2))
    {
        lastLog = ERROR_MAX_BITS_PER_BCHANNEL;
        return RES_ERROR;
    }
    else if((myNormalizatorPtr->getALPHAType()==NORM_ISIGNED) && (myNormalizatorPtr->getALPHABitsCount()<2))
    {
        lastLog = ERROR_MAX_BITS_PER_ACHANNEL;
        return RES_ERROR;
    }
    else if((myNormalizatorPtr->getREDType()==NORM_FLOAT))
    {
        if((myNormalizatorPtr->getREDBitsCount()!=10)&&
           (myNormalizatorPtr->getREDBitsCount()!=11)&&
           (myNormalizatorPtr->getREDBitsCount()!=12)&&
           (myNormalizatorPtr->getREDBitsCount()!=16)&&
           (myNormalizatorPtr->getREDBitsCount()!=32))
        {
            lastLog = ERROR_FLOAT_FORMAT_RCHANNEL;
            return RES_ERROR;
        }
    }
    else if((myNormalizatorPtr->getGREENType()==NORM_FLOAT))
    {
        if((myNormalizatorPtr->getGREENBitsCount()!=10)&&
           (myNormalizatorPtr->getGREENBitsCount()!=11)&&
           (myNormalizatorPtr->getGREENBitsCount()!=12)&&
           (myNormalizatorPtr->getGREENBitsCount()!=16)&&
           (myNormalizatorPtr->getGREENBitsCount()!=32))
        {
            lastLog = ERROR_FLOAT_FORMAT_GCHANNEL;
            return RES_ERROR;
        }
    }
    else if((myNormalizatorPtr->getBLUEType()==NORM_FLOAT))
    {
        if((myNormalizatorPtr->getBLUEBitsCount()!=10)&&
           (myNormalizatorPtr->getBLUEBitsCount()!=11)&&
           (myNormalizatorPtr->getBLUEBitsCount()!=12)&&
           (myNormalizatorPtr->getBLUEBitsCount()!=16)&&
           (myNormalizatorPtr->getBLUEBitsCount()!=32))
        {
            lastLog = ERROR_FLOAT_FORMAT_BCHANNEL;
            return RES_ERROR;
        }
    }
    else if((myNormalizatorPtr->getALPHAType()==NORM_FLOAT))
    {
        if((myNormalizatorPtr->getALPHABitsCount()!=10)&&
           (myNormalizatorPtr->getALPHABitsCount()!=11)&&
           (myNormalizatorPtr->getALPHABitsCount()!=12)&&
           (myNormalizatorPtr->getALPHABitsCount()!=16)&&
           (myNormalizatorPtr->getALPHABitsCount()!=32))
        {
            lastLog = ERROR_FLOAT_FORMAT_ACHANNEL;
            return RES_ERROR;
        }
    }
  return RES_OK;
}
