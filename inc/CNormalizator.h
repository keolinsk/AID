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

#ifndef CNORMALIZATOR_H
#define CNORMALIZATOR_H

#include "defines.h"

#include<QVector>
#include<QImage>

#ifdef QT4
    #include<QListWidgetItem>
#elif QT5
    #include<QtWidgets/QListWidgetItem>
#endif

/*!
 * \brief The vType enum represents a channel coding mode.
 */
enum vType{
    NORM_EMPTY,
    NORM_IUNSIGNED,
    NORM_ISIGNED,
    NORM_FLOAT
};

/*!
 * \brief The BitIndexAndCount struct code represents a channel bit-word chunk by pairing
 *        a start-bit index and a chunk length.
 */
struct BitIndexAndCount
{
    quint8   bitIndex;
    quint32  bitsCount;
};

/*!
 * \brief The CNormalizator class encapsulates methods and data for the normalization procedure.
 */
class CNormalizator : public QObject
{
    Q_OBJECT
public:
                                 CNormalizator();
                                ~CNormalizator(){}

    /* Adders for the bit-word chunks. */
    void                         addREDBitsMask(BitIndexAndCount bic);
    void                         addGREENBitsMask(BitIndexAndCount bic);
    void                         addBLUEBitsMask(BitIndexAndCount bic);
    void                         addALPHABitsMask(BitIndexAndCount bic);
    void                         addXBitsMask(BitIndexAndCount bic);
    void                         resetMasks();

    /* Getters for bit-word chunks lengths. */
    uint                         getREDBitsCount(){return   channelBitCount[0];}
    uint                         getGREENBitsCount(){return channelBitCount[1];}
    uint                         getBLUEBitsCount(){return  channelBitCount[2];}
    uint                         getALPHABitsCount(){return channelBitCount[3];}

    /* Getters for channel coding modes. */
    vType                        getREDType(){return mType[0];}
    vType                        getGREENType(){return mType[1];}
    vType                        getBLUEType(){return mType[2];}
    vType                        getALPHAType(){return mType[3];}

    /* A native pointer data setter. */
    void                         setNativeDataPtr(void* ptr);

    /* Additional filtering parameters setters. */
    void                         setREDGain(float fgain){gain[0] = fgain;}
    void                         setGREENGain(float fgain){gain[1] = fgain;}
    void                         setBLUEGain(float fgain){gain[2] = fgain;}
    void                         setALPHAGain(float fgain){gain[3] = fgain;}
    void                         setREDBias(float fbias){bias[0] = fbias;}
    void                         setGREENBias(float fbias){bias[1] = fbias;}
    void                         setBLUEBias(float fbias){bias[2] = fbias;}
    void                         setALPHABias(float fbias){bias[3] = fbias;}


    /* Additional filtering parameters getters. */
    float                        getChannelBias(channel ch){return bias[ch];}
    float                        getChannelGain(channel ch){return gain[ch];}

    /* Effective bit count getter. */
    quint8                       getEffectiveBitCount(){return effectivePixelBitsCount;}

    /* Image data properties setters. */
    void                         setRowStride(quint32 stride){rowStride = stride;}
    void                         setImageWidth(quint32 width);
    void                         setImageHeight(quint32 height);

    /* Channel coding mode setters. */
    void                         setREDType(vType mType){this->mType[0] = mType;}
    void                         setGREENType(vType mType){this->mType[1] = mType;}
    void                         setBLUEType(vType mType){this->mType[2] = mType;}
    void                         setALPHAType(vType mType){this->mType[3] = mType;}
    void                         setREDAbsValueFlag(bool value){this->absREDValueFlag = value;}
    void                         setGREENAbsValueFlag(bool value){this->absGREENValueFlag = value;}
    void                         setBLUEAbsValueFlag(bool value){this->absBLUEValueFlag = value;}
    void                         setALPHAAbsValueFlag(bool value){this->absALPHAValueFlag = value;}

    /* Image context parent setter. */
    void                         setMyParent(void* ptr){myParentPtr = ptr;}


    /* An image calibration function. */
    void                         calibrate();

    /* Returns a resulting image. */
    QImage                       getImage();

    /* Returns a resulting image with the additional filtering applied. */
    QImage                       getImageWithFiltering();

    /* Returns a string representing a given pixel value. */
    QString                      getPixelValueStr(qint32 iw, qint32 ih, qint32 dispBase = 10);

    /* Writes a normalized pixel value in the form of 32 bit float RGBA. <pixelValue> is a pointer to 4-elements float array. */
    void                         getPixelValue(quint32 &bitCounter, float* pixelValue);

    /* Other helpers. */
    void                         adjustCapacity();
    quint8                       getColumnStride(){return columnStride;}
    quint32                      getRowStride(){return rowStride;}

private:

    QRgb                         normSinglePixel(quint32 &bitCounter);
    QRgb                         normSinglePixelWithFiltering(quint32 &bitCounter);


    vType                        mType[4];
    quint8                       channelBitPattern[4];
    quint64                     *framePtr;
    quint64                     *memCursor;
    QVector<BitIndexAndCount>    myREDBitsIndices;
    QVector<BitIndexAndCount>    myGREENBitsIndices;
    QVector<BitIndexAndCount>    myBLUEBitsIndices;
    QVector<BitIndexAndCount>    myALPHABitsIndices;
    quint8                       effectivePixelBitsCount;
    quint8                       dummyBitsCount;
    quint32                      rowStride;
    quint8                       columnStride;
    quint32                      width, height;
    quint32                      channelAbsCapacity[4];
    quint8                       channelBitCount[4];
    float                        gain[4];
    float                        bias[4];
    void*                        myParentPtr;
    bool                         absREDValueFlag;
    bool                         absGREENValueFlag;
    bool                         absBLUEValueFlag;
    bool                         absALPHAValueFlag;

};

#endif // CBITFIELDCONTAINER_H
