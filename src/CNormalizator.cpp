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

#include "./inc/CNormalizator.h"
#include "./inc/CImgContext.h"
#include "./inc/globals.h"

#include <QImage>
#include <math.h>

#define MIN(a, b) (a<b)?a:b
#define MAX(a, b) (a<b)?b:a


///////////////////////////////////////////////////////////////////////////////////////////////////
inline quint64 MASK(int offset, int onesCount)
{
    return ((quint64)(-1L)>>(64-offset-onesCount))&((quint64)(-1L)<<offset);
}


inline quint8 sat8(quint32 r)
{
    return MIN(r, 255);
}

inline quint8 sat8(qint32 r)
{
    if(!MAX(r, 0)) return 0;
    return MIN(r, 255);
}

inline quint8 sat8(float r)
{
    if(!MAX(r, 0)) return 0;
    return MIN(r, 255);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/*!
 * Direct X to 32bit integer
 */

inline qint32 signedInt_2_i32(quint32 bits, quint8 signbit)
{
    qint32 sign = 1;

    if(bits&MASK(signbit, 1)) sign = -1;
    bits &=MASK(0, signbit);
    return bits*sign;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/*!
 * Direct X to 32bit float converters
 */

inline float unsignedInt_2_f32(quint32 bits, quint8 capacity)
{
    if(capacity == 0) return 0;
    return (float)bits/capacity;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float signedInt_2_f32(quint32 bits, quint8 signbit, quint8 capacity)
{
    if(capacity < 2) return 0;
    float ftmp = 0.5f;

    if(bits&MASK(signbit, 1)) ftmp = 0;
    bits &=MASK(0, signbit);
    return (float)bits/(capacity) + ftmp;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
inline float float32_2_f32(quint32 bits)
{
    return *((float*)&bits);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float float16_2_f32(quint16 bits)
{
   quint8   sign;
   quint32  E,M;
   float    res = 0;

   sign = bits&MASK(15,1);
   if(sign) return 0;

   E = bits&MASK(10,5)>>10;
   M = bits&MASK(0,10);

   if(E==31){
       if(M==0)return 255;
       else return 0;}

   if(E==0){
       if(M==0) return 0;
       else res = 2E-24f*M;}
   else
   {   res = pow(2.0f, -15);
       res += (1+M/2E10);}

   return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float float11_2_f32(quint16 bits)
{
   quint32  E,M;
   float    res = 0;

   E = bits&MASK(6,5)>>6;
   M = bits&MASK(0,6);

   if(E==31){
       if(M==0) return 255;
       else return 0;}

   if(E==0){
       if(M==0) return 0;
       else res = 2E-20f*M;}
   else
   {   res = pow(2.0f, -15);
       res += (1+M/64);}

   return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float float10_2_f32(quint16 bits)
{
   quint32  E,M;
   float    res = 0;

   E = bits&MASK(5,4)>>5;
   M = bits&MASK(0,5);

   if(E==31){
       if(M==0)return 255;
       else return 0;}

   if(E==0){
       if(M==0) return 0;
       else res = 2E-19f*M;}
   else
   {   res = pow(2.0f, -15);
       res += (1+M/32);}

   return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/*!
 * Direct X to normalized 8bit converters
 */

inline quint8 unsignedInt_2_disp(quint32 bits, quint8 capacity)
{
    return sat8((float)bits/capacity * 255);
}

inline quint8 signedInt_2_disp(quint32 bits, quint8 signbit, quint8 capacity)
{
    float ftmp = 0.5f;

    if(bits&MASK(signbit, 1)) ftmp = 0;
    bits &=MASK(0, signbit);
    return sat8((float)bits/capacity * 127 + ftmp);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline quint8 float32_2_disp(quint32 bits)
{
    float ftmp = *(float*)&bits;
    return sat8(ftmp*255);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline quint8 float16_2_disp(quint16 bits)
{
   return sat8(float16_2_f32(bits)*255);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline quint8 float11_2_disp(quint16 bits)
{
   return sat8(float11_2_f32(bits)*255);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline quint8 float10_2_disp(quint16 bits)
{
   return sat8(float10_2_f32(bits)*255);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/*!
 * X to 32bit float converters with linear filtering
 */

inline float unsignedInt_2_f32_lf(quint32 bits, quint8 capacity, float gain, float bias)
{
    float ftmp;
    if(capacity == 0) return 0;
    ftmp = (float)bits/capacity;
    ftmp *= gain;
    ftmp += bias;
    return ftmp;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float signedInt_2_f32_lf(quint32 bits, quint8 signbit, quint8 capacity, float gain, float bias)
{
    float ftmp = 0.5f;

    if(bits&MASK(signbit, 1)) ftmp = 0;
    bits &=MASK(0, signbit);
    ftmp = (float)bits/capacity + ftmp;
    ftmp *= gain;
    ftmp += bias;

    return ftmp;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float float16_2_f32_lf(quint16 bits, float gain, float bias)
{
   quint8   sign;
   quint32  E,M;
   float    res = 0;

   sign = bits&MASK(15,1);
   if(sign) return 0;

   E = bits&MASK(10,5)>>10;
   M = bits&MASK(0,10);

   if(E==31){
       if(M==0)return 255;
       else return 0;}

   if(E==0){
       if(M==0) return 0;
       else res = 2E-24f*M;}
   else
   {   res = pow(2.0f, -15);
       res += (1+M/2E10);}

   res *= gain;
   res += bias;

   return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float float11_2_f32_lf(quint16 bits,  float gain, float bias)
{
   quint32  E,M;
   float    res = 0;

   E = bits&MASK(6,5)>>6;
   M = bits&MASK(0,6);

   if(E==31){
       if(M==0) return 255;
       else return 0;}

   if(E==0){
       if(M==0) return 0;
       else res = 2E-20f*M;}
   else
   {   res = pow(2.0f, -15);
       res += (1+M/64);}

   res *= gain;
   res +=bias;

   return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float float10_2_f32_lf(quint16 bits, float gain, float bias)
{
   quint32  E,M;
   float    res = 0;

   E = bits&MASK(5,4)>>5;
   M = bits&MASK(0,5);

   if(E==31){
       if(M==0)return 255;
       else return 0;}

   if(E==0){
       if(M==0) return 0;
       else res = 2E-19f*M;}
   else
   {   res = pow(2.0f, -15);
       res += (1+M/32);}

   res *= gain;
   res += bias;

   return res;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/*!
 * X to normalized 8bit converters with linear filtering
 */

inline quint8 unsignedInt_2_disp_lf(quint32 bits, quint8 capacity, float gain, float bias)
{
    float ftmp;
    //if(capacity == 0) return 0;
    ftmp = (float)bits/capacity;
    ftmp *= gain;
    ftmp += bias;
    return sat8(ftmp * 255);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline quint8 signedInt_2_disp_lf(quint32 bits, quint8 signbit ,quint8 capacity, float gain, float bias)
{
    //if(capacity < 2) return 0;
    float ftmp = 0.5f;

    if(bits&MASK(signbit, 1)) ftmp = 0;
    bits &=MASK(0, signbit);
    ftmp = (float)bits/capacity + ftmp;
    ftmp *= gain;
    ftmp += bias;
    return sat8(ftmp * 127);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline quint8 float32_2_disp_lf(quint32 bits, float gain, float bias)
{
    float ftmp = *(float*)&bits;
    ftmp *= gain;
    ftmp += bias;
    return sat8(ftmp*255);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline quint8 float16_2_disp_lf(quint16 bits, float gain, float bias)
{
   float ftmp = float16_2_f32(bits);
   ftmp *= gain;
   ftmp += bias;
   return sat8(ftmp*255);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline quint8 float11_2_disp_lf(quint16 bits, float gain, float bias)
{
   float ftmp = float11_2_f32(bits);
   ftmp *= gain;
   ftmp += bias;
   return sat8(ftmp*255);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline quint8 float10_2_disp_lf(quint16 bits, float gain, float bias)
{
   float ftmp = float10_2_f32(bits);
   ftmp *= gain;
   ftmp += bias;
   return sat8(ftmp*255);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CNormalizator::CNormalizator()
{
    framePtr = NULL;
    rowStride = 0;
    columnStride = 0;
    effectivePixelBitsCount = 0;
    dummyBitsCount = 0;
    gain[0] = gain[1] = gain[2] = gain[3] = 1.0f;
    bias[0] = bias[1] = bias[2] = bias[3] = 0.0f;

    channelBitPattern[0] = channelBitPattern[1]=\
    channelBitPattern[2] = channelBitPattern[3] = 0;

    channelAbsCapacity[0] = channelAbsCapacity[1]=\
    channelAbsCapacity[2] = channelAbsCapacity[3] = 0;

    channelBitCount[0] = channelBitCount[1]=\
    channelBitCount[2] = channelBitCount[3] = 0;

    absREDValueFlag = false;
    absGREENValueFlag = false;
    absBLUEValueFlag = false;
    absALPHAValueFlag = false;

    mType[0]= mType[1] = mType[2] = mType[3] = NORM_EMPTY;

    width = height = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CNormalizator::addREDBitsMask(BitIndexAndCount bic)
{
    channelBitPattern[0]++;
    channelBitCount[0] += bic.bitsCount;
    effectivePixelBitsCount += bic.bitsCount;
    myREDBitsIndices.append(bic);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CNormalizator::addGREENBitsMask(BitIndexAndCount bic)
{
    channelBitPattern[1]++;
    channelBitCount[1] += bic.bitsCount;
    effectivePixelBitsCount += bic.bitsCount;
    myGREENBitsIndices.append(bic);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CNormalizator::addBLUEBitsMask(BitIndexAndCount bic)
{
    channelBitPattern[2]++;
    channelBitCount[2] += bic.bitsCount;
    effectivePixelBitsCount += bic.bitsCount;
    myBLUEBitsIndices.append(bic);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CNormalizator::addALPHABitsMask(BitIndexAndCount bic)
{
    channelBitPattern[3]++;
    channelBitCount[3] += bic.bitsCount;
    effectivePixelBitsCount += bic.bitsCount;
    myALPHABitsIndices.append(bic);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CNormalizator::addXBitsMask(BitIndexAndCount bic)
{
    dummyBitsCount += bic.bitsCount;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CNormalizator::resetMasks()
{
    channelBitPattern[0] = channelBitPattern[1] = channelBitPattern[2] = channelBitPattern[3] = 0;
    channelBitCount[0] = channelBitCount[1] = channelBitCount[2] = channelBitCount[3] = 0;
    effectivePixelBitsCount = 0;
    myREDBitsIndices.clear();
    myGREENBitsIndices.clear();
    myBLUEBitsIndices.clear();
    myALPHABitsIndices.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CNormalizator::adjustCapacity()
{
    //Calculate the column stride value.
    columnStride = effectivePixelBitsCount + dummyBitsCount;
    channelAbsCapacity[0] = (quint32)pow(2.0f, (int)channelBitCount[0])-1;
    channelAbsCapacity[1] = (quint32)pow(2.0f, (int)channelBitCount[1])-1;
    channelAbsCapacity[2] = (quint32)pow(2.0f, (int)channelBitCount[2])-1;
    channelAbsCapacity[3] = (quint32)pow(2.0f, (int)channelBitCount[3])-1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CNormalizator::setNativeDataPtr(void *ptr)
{
    framePtr = (quint64*)ptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CNormalizator::setImageWidth(quint32 w)
{
    Q_ASSERT_X(w, "CNormalizator::setImageWidth", "zero width.");
    width = w;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CNormalizator::setImageHeight(quint32 h)
{
    Q_ASSERT_X(h, "CNormalizator::setImageWidth", "zero height.");
    height = h;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CNormalizator::calibrate()
{
    quint32 iw, ih;
    quint32 bitCounter = 0;
    float minV[4], maxV[4];
    int   tmpV[4];
    float ftmp;

    adjustCapacity();

    getPixelValue(bitCounter,&minV[0]);
    bitCounter = 0;
    maxV[0] = maxV[1] = maxV[2] = maxV[3] = -MAX_FLOAT;
    minV[0] = minV[1] = minV[2] = minV[3] = MAX_FLOAT;

    for(iw = 0; iw < width; iw++)
    {
        for(ih = 0; ih < height; ih++)
        {
            getPixelValue(bitCounter,(float*)&tmpV[0]);
            tmpV[0]= absREDValueFlag  ?tmpV[0]&0x7FFFFFFF:tmpV[0];
            tmpV[1]= absGREENValueFlag?tmpV[1]&0x7FFFFFFF:tmpV[1];
            tmpV[2]= absBLUEValueFlag ?tmpV[2]&0x7FFFFFFF:tmpV[2];
            tmpV[3]= absALPHAValueFlag?tmpV[3]&0x7FFFFFFF:tmpV[3];


            ftmp = *(float*)&tmpV[0]; if(ftmp>maxV[0])maxV[0]=ftmp; if(ftmp<minV[0])minV[0]=ftmp;
            ftmp = *(float*)&tmpV[1]; if(ftmp>maxV[1])maxV[1]=ftmp; if(ftmp<minV[1])minV[1]=ftmp;
            ftmp = *(float*)&tmpV[2]; if(ftmp>maxV[2])maxV[2]=ftmp; if(ftmp<minV[2])minV[2]=ftmp;
            ftmp = *(float*)&tmpV[3]; if(ftmp>maxV[3])maxV[3]=ftmp; if(ftmp<minV[3])minV[3]=ftmp;

            bitCounter += columnStride;
        }
        bitCounter += rowStride;
        (reinterpret_cast<CImgContext*>(myParentPtr))->auxInfo = "Pre-parsing: " + QString::number((ulong)iw*100/width) + "%";
    }


    for(iw = 0; iw <4; iw++)
    {
        gain[iw] = 1/(maxV[iw]-minV[iw]);  bias[iw] = -minV[iw];
        if((gain[iw]>MAX_FLOAT)||(gain[iw]<-MAX_FLOAT))
         { gain[iw] = 1;  bias[iw] = 0.0;}
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline QRgb CNormalizator::normSinglePixel(quint32 &bitCounter)
{
    quint32 channelBits[] = {0,0,0,0};
    quint32 fragBitsCount;
    qint32  ib;
    quint64 ltmp;
    unsigned int fragBase;
    unsigned int fragOffset;

    Q_ASSERT_X(framePtr, "CNormalizator::normSinglePixel","Native data pointer is NULL.");

    //red bits
    channelBits[0] = 0;
    fragBitsCount = 0;
    for(ib=0; ib < myREDBitsIndices.count(); ib++)
    {
        fragBase = (unsigned int)((bitCounter+myREDBitsIndices[ib].bitIndex)/64);
        fragOffset = (unsigned int)((bitCounter+myREDBitsIndices[ib].bitIndex)%64);
        ltmp = MASK(fragOffset, myREDBitsIndices[ib].bitsCount);
        ltmp &= *(quint64*)(framePtr + fragBase);
        ltmp >>= fragOffset;
        channelBits[0] |= (ltmp << fragBitsCount);
        fragBitsCount += myREDBitsIndices[ib].bitsCount;
    }

    switch(mType[0])
    {
        case NORM_IUNSIGNED:
            channelBits[0] = unsignedInt_2_disp(channelBits[0], channelAbsCapacity[0]);
        break;
        case NORM_ISIGNED:
            channelBits[0] = signedInt_2_disp(absREDValueFlag?channelBits[0]&MASK(0,fragBitsCount-1):channelBits[0], fragBitsCount-1, channelAbsCapacity[0]);
        break;
        case NORM_FLOAT:
            if(fragBitsCount==32)
                channelBits[0] = float32_2_disp(absREDValueFlag?channelBits[0]&MASK(0,31):channelBits[0]);
            else if(fragBitsCount==16)
                channelBits[0] = float16_2_disp((quint16)(channelBits[0]&(absREDValueFlag?channelBits[0]&MASK(0,15):channelBits[0])));
            else if(fragBitsCount==11)
                channelBits[0] = float11_2_disp((quint16)(channelBits[0]&(absREDValueFlag?channelBits[0]&MASK(0,10):channelBits[0])));
            else if(fragBitsCount==10)
                channelBits[0] = float10_2_disp((quint16)(channelBits[0]&0xFFFF));
            else
                channelBits[0] = 0;
        break;
            default:
            channelBits[0] = 0;
    };

    //green bits
    channelBits[1] = 0;
    fragBitsCount = 0;
    for(ib=0; ib < myGREENBitsIndices.count(); ib++)
    {
        fragBase = (unsigned int)((bitCounter+myGREENBitsIndices[ib].bitIndex)/64);
        fragOffset = (unsigned int)((bitCounter+myGREENBitsIndices[ib].bitIndex)%64);
        ltmp = MASK(fragOffset, myGREENBitsIndices[ib].bitsCount);
        ltmp &= *(quint64*)(framePtr + fragBase);
        ltmp >>= fragOffset;
        channelBits[1] |= (ltmp << fragBitsCount);
        fragBitsCount += myGREENBitsIndices[ib].bitsCount;
    }
    switch(mType[1])
    {
        case NORM_IUNSIGNED:
            channelBits[1] = unsignedInt_2_disp(channelBits[1], channelAbsCapacity[1]);
        break;
        case NORM_ISIGNED:
            channelBits[1] = signedInt_2_disp(absGREENValueFlag?channelBits[1]&MASK(0,fragBitsCount-1):channelBits[1], fragBitsCount-1, channelAbsCapacity[1]);
        break;
        case NORM_FLOAT:
            if(fragBitsCount==32)
                channelBits[1] = float32_2_disp(absGREENValueFlag?channelBits[1]&MASK(0,31):channelBits[1]);
            else if(fragBitsCount==16)
                channelBits[1] = float16_2_disp((quint16)(channelBits[1]&(absGREENValueFlag?channelBits[1]&MASK(0,15):channelBits[1])));
            else if(fragBitsCount==11)
                channelBits[1] = float11_2_disp((quint16)(channelBits[0]&(absGREENValueFlag?channelBits[1]&MASK(0,10):channelBits[1])));
            else if(fragBitsCount==10)
                channelBits[1] = float10_2_disp((quint16)(channelBits[1]&0xFFFF));
            else
                channelBits[1] = 0;
        break;
            default:
                channelBits[1] = 0;
    };

    //blue bits
    channelBits[2] = 0;
    fragBitsCount = 0;
    for(ib=0; ib < myBLUEBitsIndices.count(); ib++)
    {
        fragBase = (unsigned int)((bitCounter+myBLUEBitsIndices[ib].bitIndex)/64);
        fragOffset = (unsigned int)((bitCounter+myBLUEBitsIndices[ib].bitIndex)%64);
        ltmp = MASK(fragOffset, myBLUEBitsIndices[ib].bitsCount);
        ltmp &= *(quint64*)(framePtr + fragBase);
        ltmp >>= fragOffset;
        channelBits[2] |= (ltmp << fragBitsCount);
        fragBitsCount += myBLUEBitsIndices[ib].bitsCount;
    }
    switch(mType[2])
    {
        case NORM_IUNSIGNED:
            channelBits[2] = unsignedInt_2_disp(channelBits[2], channelAbsCapacity[2]);
        break;
        case NORM_ISIGNED:
            channelBits[2] = signedInt_2_disp(absBLUEValueFlag?channelBits[2]&MASK(0,fragBitsCount-1):channelBits[2], fragBitsCount-1, channelAbsCapacity[2]);
        break;
        case NORM_FLOAT:
            if(fragBitsCount==32)
                channelBits[2] = float32_2_disp(absBLUEValueFlag?channelBits[2]&MASK(0,31):channelBits[2]);
            else if(fragBitsCount==16)
                channelBits[2] = float16_2_disp((quint16)(channelBits[2]&(absBLUEValueFlag?channelBits[2]&MASK(0,15):channelBits[2])));
            else if(fragBitsCount==11)
                channelBits[2] = float11_2_disp((quint16)(channelBits[2]&(absBLUEValueFlag?channelBits[2]&MASK(0,10):channelBits[2])));
            else if(fragBitsCount==10)
                channelBits[2] = float10_2_disp((quint16)(channelBits[2]&0xFFFF));
            else
                channelBits[2] = 0;
            break;
            default:
                channelBits[2] = 0;
    };

    //alpha
    channelBits[3] = 0;
    fragBitsCount = 0;
    for(ib=0; ib < myALPHABitsIndices.count(); ib++)
    {
        fragBase = (unsigned int)((bitCounter+myALPHABitsIndices[ib].bitIndex)/64);
        fragOffset = (unsigned int)((bitCounter+myALPHABitsIndices[ib].bitIndex)%64);
        ltmp = MASK(fragOffset, myALPHABitsIndices[ib].bitsCount);
        ltmp &= *(quint64*)(framePtr + fragBase);
        ltmp >>= fragOffset;
        channelBits[3] |= (ltmp << fragBitsCount);
        fragBitsCount += myALPHABitsIndices[ib].bitsCount;
    }
    switch(mType[3])
    {
        case NORM_IUNSIGNED:
            channelBits[3] = unsignedInt_2_disp(channelBits[3], channelAbsCapacity[3]);
        break;
        case NORM_ISIGNED:
            channelBits[3] = signedInt_2_disp(absREDValueFlag?channelBits[3]&MASK(0,fragBitsCount-1):channelBits[3], fragBitsCount-1, channelAbsCapacity[3]);
        break;
        case NORM_FLOAT:
            if(fragBitsCount==32)
                channelBits[3] = float32_2_disp(absREDValueFlag?channelBits[3]&MASK(0,31):channelBits[3]);
            else if(fragBitsCount==16)
                channelBits[3] = float16_2_disp((quint16)(channelBits[3]&(absREDValueFlag?channelBits[3]&MASK(0,15):channelBits[3])));
            else if(fragBitsCount==11)
                channelBits[3] = float11_2_disp((quint16)(channelBits[3]&(absREDValueFlag?channelBits[3]&MASK(0,10):channelBits[3])));
            else if(fragBitsCount==10)
                channelBits[3] = float10_2_disp((quint16)(channelBits[3]&0xFFFF));
            else
                channelBits[3] = 0;
        break;
            default:
                channelBits[3] = 255;
    };

    return qRgba(channelBits[0],channelBits[1],channelBits[2],channelBits[3]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline QRgb CNormalizator::normSinglePixelWithFiltering(quint32 &bitCounter)
{
    quint32 channelBits[] = {0,0,0,0};
    quint32 fragBitsCount;
    qint32  ib;
    quint64 ltmp;
    unsigned int fragBase;
    unsigned int fragOffset;

    //red bits
    channelBits[0] = 0;
    fragBitsCount = 0;
    for(ib=0; ib < myREDBitsIndices.count(); ib++)
    {
        fragBase = (unsigned int)((bitCounter+myREDBitsIndices[ib].bitIndex)/64);
        fragOffset = (unsigned int)((bitCounter+myREDBitsIndices[ib].bitIndex)%64);
        ltmp = MASK(fragOffset, myREDBitsIndices[ib].bitsCount);
        ltmp &= *(quint64*)(framePtr + fragBase);
        ltmp >>= fragOffset;
        channelBits[0] |= (ltmp << fragBitsCount);
        fragBitsCount += myREDBitsIndices[ib].bitsCount;
    }

    switch(mType[0])
    {
        case NORM_IUNSIGNED:
            channelBits[0] = unsignedInt_2_disp_lf(channelBits[0], channelAbsCapacity[0], gain[0], bias[0]);
        break;
        case NORM_ISIGNED:
            channelBits[0] = signedInt_2_disp_lf(absREDValueFlag?channelBits[0]&MASK(0,fragBitsCount-1):channelBits[0], fragBitsCount-1, channelAbsCapacity[0], gain[0], bias[0]);
        break;
        case NORM_FLOAT:
            if(fragBitsCount==32)
                channelBits[0] = float32_2_disp_lf(absREDValueFlag?channelBits[0]&MASK(0,31):channelBits[0], gain[0], bias[0]);
            else if(fragBitsCount==16)
                channelBits[0] = float16_2_disp_lf((quint16)(channelBits[0]&(absREDValueFlag?channelBits[0]&MASK(0,15):channelBits[0])), gain[0], bias[0]);
            else if(fragBitsCount==11)
                channelBits[0] = float11_2_disp_lf((quint16)(channelBits[0]&(absREDValueFlag?channelBits[0]&MASK(0,10):channelBits[0])), gain[0], bias[0]);
            else if(fragBitsCount==10)
                channelBits[0] = float10_2_disp_lf((quint16)(channelBits[0]&0xFFFF), gain[0], bias[0]);
            else
                channelBits[0] = 0;
        break;
            default:
            channelBits[0] = 0;
    };

    //green bits
    channelBits[1] = 0;
    fragBitsCount = 0;
    for(ib=0; ib < myGREENBitsIndices.count(); ib++)
    {
        fragBase = (unsigned int)((bitCounter+myGREENBitsIndices[ib].bitIndex)/64);
        fragOffset = (unsigned int)((bitCounter+myGREENBitsIndices[ib].bitIndex)%64);
        ltmp = MASK(fragOffset, myGREENBitsIndices[ib].bitsCount);
        ltmp &= *(quint64*)(framePtr + fragBase);
        ltmp >>= fragOffset;
        channelBits[1] |= (ltmp << fragBitsCount);
        fragBitsCount += myGREENBitsIndices[ib].bitsCount;
    }
    switch(mType[1])
    {
        case NORM_IUNSIGNED:
            channelBits[1] = unsignedInt_2_disp_lf(channelBits[1], channelAbsCapacity[1], gain[1], bias[1]);
        break;
        case NORM_ISIGNED:
            channelBits[1] = signedInt_2_disp_lf(absGREENValueFlag?channelBits[1]&MASK(0,fragBitsCount-1):channelBits[1], fragBitsCount-1, channelAbsCapacity[1], gain[1], bias[1]);
        break;
        case NORM_FLOAT:
            if(fragBitsCount==32)
                channelBits[1] = float32_2_disp_lf(absGREENValueFlag?channelBits[1]&MASK(0,31):channelBits[1], gain[1], bias[1]);
            else if(fragBitsCount==16)
                channelBits[1] = float16_2_disp_lf((quint16)(channelBits[1]&(absGREENValueFlag?channelBits[1]&MASK(0,15):channelBits[1])), gain[1], bias[1]);
            else if(fragBitsCount==11)
                channelBits[1] = float11_2_disp_lf((quint16)(channelBits[1]&(absGREENValueFlag?channelBits[1]&MASK(0,10):channelBits[1])), gain[1], bias[1]);
            else if(fragBitsCount==10)
                channelBits[1] = float10_2_disp_lf((quint16)(channelBits[1]&0xFFFF), gain[1], bias[1]);
            else
                channelBits[1] = 0;
        break;
            default:
                channelBits[1] = 0;
    };

    //blue bits
    channelBits[2] = 0;
    fragBitsCount = 0;
    for(ib=0; ib < myBLUEBitsIndices.count(); ib++)
    {
        fragBase = (unsigned int)((bitCounter+myBLUEBitsIndices[ib].bitIndex)/64);
        fragOffset = (unsigned int)((bitCounter+myBLUEBitsIndices[ib].bitIndex)%64);
        ltmp = MASK(fragOffset, myBLUEBitsIndices[ib].bitsCount);
        ltmp &= *(quint64*)(framePtr + fragBase);
        ltmp >>= fragOffset;
        channelBits[2] |= (ltmp << fragBitsCount);
        fragBitsCount += myBLUEBitsIndices[ib].bitsCount;
    }
    switch(mType[2])
    {
        case NORM_IUNSIGNED:
            channelBits[2] = unsignedInt_2_disp_lf(channelBits[2], channelAbsCapacity[2], gain[2], bias[2]);
        break;
        case NORM_ISIGNED:
            channelBits[2] = signedInt_2_disp_lf(absBLUEValueFlag?channelBits[2]&MASK(0,fragBitsCount-1):channelBits[2], fragBitsCount-1, channelAbsCapacity[2], gain[2], bias[2]);
        break;
        case NORM_FLOAT:
            if(fragBitsCount==32)
                channelBits[2] = float32_2_disp_lf(absBLUEValueFlag?channelBits[2]&MASK(0,31):channelBits[2], gain[2], bias[2]);
            else if(fragBitsCount==16)
                channelBits[2] = float16_2_disp_lf((quint16)(channelBits[2]&(absBLUEValueFlag?channelBits[2]&MASK(0,15):channelBits[2])), gain[2], bias[2]);
            else if(fragBitsCount==11)
                channelBits[2] = float11_2_disp_lf((quint16)(channelBits[0]&(absBLUEValueFlag?channelBits[2]&MASK(0,10):channelBits[2])), gain[2], bias[2]);
            else if(fragBitsCount==10)
                channelBits[2] = float10_2_disp_lf((quint16)(channelBits[2]&0xFFFF), gain[2], bias[2]);
            else
                channelBits[2] = 0;
        break;
            default:
                channelBits[2] = 0;
    };

    //alpha
    channelBits[3] = 0;
    fragBitsCount = 0;
    for(ib=0; ib < myALPHABitsIndices.count(); ib++)
    {
        fragBase = (unsigned int)((bitCounter+myALPHABitsIndices[ib].bitIndex)/64);
        fragOffset = (unsigned int)((bitCounter+myALPHABitsIndices[ib].bitIndex)%64);
        ltmp = MASK(fragOffset, myALPHABitsIndices[ib].bitsCount);
        ltmp &= *(quint64*)(framePtr + fragBase);
        ltmp >>= fragOffset;
        channelBits[3] |= (ltmp << fragBitsCount);
        fragBitsCount += myALPHABitsIndices[ib].bitsCount;
    }
    switch(mType[3])
    {
        case NORM_IUNSIGNED:
            channelBits[3] = unsignedInt_2_disp_lf(channelBits[3], channelAbsCapacity[3], gain[3], bias[3]);
        break;
        case NORM_ISIGNED:
            channelBits[3] = signedInt_2_disp_lf(absALPHAValueFlag?channelBits[3]&MASK(0,fragBitsCount-1):channelBits[3], fragBitsCount-1, channelAbsCapacity[3], gain[3], bias[3]);
        break;
        case NORM_FLOAT:
            if(fragBitsCount==32)
                channelBits[3] = float32_2_disp_lf(absALPHAValueFlag?channelBits[3]&MASK(0,31):channelBits[3], gain[3], bias[3]);
            else if(fragBitsCount==16)
                channelBits[3] = float16_2_disp_lf((quint16)(channelBits[3]&(absALPHAValueFlag?channelBits[3]&MASK(0,15):channelBits[3])), gain[3], bias[3]);
            else if(fragBitsCount==11)
                channelBits[3] = float11_2_disp_lf((quint16)(channelBits[3]&(absALPHAValueFlag?channelBits[3]&MASK(0,10):channelBits[3])), gain[3], bias[3]);
            else if(fragBitsCount==10)
                channelBits[3] = float10_2_disp_lf((quint16)(channelBits[3]&0xFFFF), gain[3], bias[3]);
            else
                channelBits[3] = 0;
        break;
            default:
                channelBits[3] = 255;
    };

    return qRgba(channelBits[0],channelBits[1],channelBits[2],channelBits[3]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

QString CNormalizator::getPixelValueStr(qint32 iw, qint32 ih, int dispBase)
{
    quint32 channelBits[] = {0,0,0,0};
    quint32 fragBitsCount;
    qint32  ib;
    quint64 ltmp;
    unsigned int fragBase;
    unsigned int fragOffset;
    quint32  bitCounter = columnStride*iw + (rowStride + columnStride*width)*ih;
    QString pixelValueStr;
    QString prefixStr;

    switch(dispBase)
    {
        case 16 : prefixStr = "0x"; break;
        case 10 : prefixStr = ""; break;
        default:
            prefixStr = "";
    }

    //red bits
    pixelValueStr += " |R:";
    fragBitsCount = 0;
    for(ib=0; ib < myREDBitsIndices.count(); ib++)
    {
        fragBase = (unsigned int)((bitCounter+myREDBitsIndices[ib].bitIndex)/64);
        fragOffset = (unsigned int)((bitCounter+myREDBitsIndices[ib].bitIndex)%64);
        ltmp = MASK(fragOffset, myREDBitsIndices[ib].bitsCount);
        ltmp &= *(quint64*)(framePtr + fragBase);
        ltmp >>= fragOffset;
        channelBits[0] |= (ltmp << fragBitsCount);
        fragBitsCount += myREDBitsIndices[ib].bitsCount;
    }

    switch(mType[0])
    {
        case NORM_IUNSIGNED:
            pixelValueStr += "u" + prefixStr + QString::number(channelBits[0], dispBase);
        break;
        case NORM_ISIGNED:
            pixelValueStr += "i" + prefixStr + QString::number(signedInt_2_i32(channelBits[0], fragBitsCount-1), dispBase);
        break;
        case NORM_FLOAT:
            if(fragBitsCount==32)
                pixelValueStr += "f" + QString::number(float32_2_f32(channelBits[0]), 'g', 3);
            else if(fragBitsCount==16)
                pixelValueStr += "f" + QString::number(float16_2_f32((quint16)(channelBits[0]&0xFFFF)), 'g', 3);
            else if(fragBitsCount==11)
                pixelValueStr += "f" + QString::number(float11_2_f32((quint16)(channelBits[0]&0xFFFF)), 'g', 3);
            else if(fragBitsCount==10)
                pixelValueStr += "f" + QString::number(float10_2_f32((quint16)(channelBits[0]&0xFFFF)), 'g', 3);
            else
                pixelValueStr += "-";
        break;
        default:
            pixelValueStr += "-";
    };

    //green bits
    pixelValueStr += " |G:";
    fragBitsCount = 0;
    for(ib=0; ib < myGREENBitsIndices.count(); ib++)
    {
        fragBase = (unsigned int)((bitCounter+myGREENBitsIndices[ib].bitIndex)/64);
        fragOffset = (unsigned int)((bitCounter+myGREENBitsIndices[ib].bitIndex)%64);
        ltmp = MASK(fragOffset, myGREENBitsIndices[ib].bitsCount);
        ltmp &= *(quint64*)(framePtr + fragBase);
        ltmp >>= fragOffset;
        channelBits[1] |= (ltmp << fragBitsCount);
        fragBitsCount += myGREENBitsIndices[ib].bitsCount;
    }
    switch(mType[1])
    {
        case NORM_IUNSIGNED:
            pixelValueStr += "u" + prefixStr + QString::number(channelBits[1], dispBase);
        break;
        case NORM_ISIGNED:
            pixelValueStr += "i" + prefixStr + QString::number(signedInt_2_i32(channelBits[1], fragBitsCount-1), dispBase);
        break;
        case NORM_FLOAT:
            if(fragBitsCount==32)
                pixelValueStr += "f" + QString::number(float32_2_f32(channelBits[1]), 'g', 3);
            else if(fragBitsCount==16)
                pixelValueStr += "f" + QString::number(float16_2_f32((quint16)(channelBits[1]&0xFFFF)), 'g', 3);
            else if(fragBitsCount==11)
                pixelValueStr += "f" + QString::number(float11_2_f32((quint16)(channelBits[1]&0xFFFF)), 'g', 3);
            else if(fragBitsCount==10)
                pixelValueStr += "f" + QString::number(float10_2_f32((quint16)(channelBits[1]&0xFFFF)), 'g', 3);
            else
                pixelValueStr += "-";
        break;
        default:
            pixelValueStr += "-";
    };

    //blue bits
    pixelValueStr += " |B:";
    fragBitsCount = 0;
    for(ib=0; ib < myBLUEBitsIndices.count(); ib++)
    {
        fragBase = (unsigned int)((bitCounter+myBLUEBitsIndices[ib].bitIndex)/64);
        fragOffset = (unsigned int)((bitCounter+myBLUEBitsIndices[ib].bitIndex)%64);
        ltmp = MASK(fragOffset, myBLUEBitsIndices[ib].bitsCount);
        ltmp &= *(quint64*)(framePtr + fragBase);
        ltmp >>= fragOffset;
        channelBits[2] |= (ltmp << fragBitsCount);
        fragBitsCount += myBLUEBitsIndices[ib].bitsCount;
    }
    switch(mType[2])
    {
        case NORM_IUNSIGNED:
            pixelValueStr += "u" + prefixStr + QString::number(channelBits[2], dispBase);
        break;
        case NORM_ISIGNED:
            pixelValueStr += "i" + prefixStr + QString::number(signedInt_2_i32(channelBits[2], fragBitsCount-1), dispBase);
        break;
        case NORM_FLOAT:
            if(fragBitsCount==32)
                pixelValueStr += "f" + QString::number(float32_2_f32(channelBits[2]), 'g', 3);
            else if(fragBitsCount==16)
                pixelValueStr += "f" + QString::number(float16_2_f32((quint16)(channelBits[2]&0xFFFF)), 'g', 3);
            else if(fragBitsCount==11)
                pixelValueStr += "f" + QString::number(float11_2_f32((quint16)(channelBits[2]&0xFFFF)), 'g', 3);
            else if(fragBitsCount==10)
                pixelValueStr += "f" + QString::number(float10_2_f32((quint16)(channelBits[2]&0xFFFF)), 'g', 3);
            else
                pixelValueStr += "-";
        break;
        default:
            pixelValueStr += "-";
    };

    //alpha
    pixelValueStr += " |A:";
    fragBitsCount = 0;
    for(ib=0; ib < myALPHABitsIndices.count(); ib++)
    {
        fragBase = (unsigned int)((bitCounter+myALPHABitsIndices[ib].bitIndex)/64);
        fragOffset = (unsigned int)((bitCounter+myALPHABitsIndices[ib].bitIndex)%64);
        ltmp = MASK(fragOffset, myALPHABitsIndices[ib].bitsCount);
        ltmp &= *(quint64*)(framePtr + fragBase);
        ltmp >>= fragOffset;
        channelBits[3] |= (ltmp << fragBitsCount);
        fragBitsCount += myALPHABitsIndices[ib].bitsCount;
    }
    switch(mType[3])
    {
        case NORM_IUNSIGNED:
            pixelValueStr += "u" + prefixStr + QString::number(channelBits[3], dispBase);
        break;
        case NORM_ISIGNED:
            pixelValueStr += "i" + prefixStr + QString::number(signedInt_2_i32(channelBits[3], fragBitsCount-1), dispBase);
        break;
        case NORM_FLOAT:
            if(fragBitsCount==32)
                pixelValueStr += "f" + QString::number(float32_2_f32(channelBits[3]), 'g', 3);
            else if(fragBitsCount==16)
                pixelValueStr += "f" + QString::number(float16_2_f32((quint16)(channelBits[3]&0xFFFF)), 'g', 3);
            else if(fragBitsCount==11)
                pixelValueStr += "f" + QString::number(float11_2_f32((quint16)(channelBits[3]&0xFFFF)), 'g', 3);
            else if(fragBitsCount==10)
                pixelValueStr += "f" + QString::number(float10_2_f32((quint16)(channelBits[3]&0xFFFF)), 'g', 3);
            else
                pixelValueStr += "-";
        break;
        default:
            pixelValueStr += "-";
    };

    return pixelValueStr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void CNormalizator::getPixelValue(quint32 &bitCounter, float* pixelValue)
{
    quint32 channelBits[] = {0,0,0,0};
    quint32 fragBitsCount;
    qint32  ib;
    quint64 ltmp;
    unsigned int fragBase;
    unsigned int fragOffset;

    //red bits
    pixelValue[0] = 0;
    fragBitsCount = 0;
    for(ib=0; ib < myREDBitsIndices.count(); ib++)
    {
        fragBase = (unsigned int)((bitCounter+myREDBitsIndices[ib].bitIndex)/64);
        fragOffset = (unsigned int)((bitCounter+myREDBitsIndices[ib].bitIndex)%64);
        ltmp = MASK(fragOffset, myREDBitsIndices[ib].bitsCount);
        ltmp &= *(quint64*)(framePtr + fragBase);
        ltmp >>= fragOffset;
        channelBits[0] |= (ltmp << fragBitsCount);
        fragBitsCount += myREDBitsIndices[ib].bitsCount;
    }

    switch(mType[0])
    {
        case NORM_IUNSIGNED:
            pixelValue[0] = unsignedInt_2_f32(channelBits[0], channelAbsCapacity[0]);
        break;
        case NORM_ISIGNED:
            pixelValue[0] = signedInt_2_f32(channelBits[0], fragBitsCount-1,channelAbsCapacity[0]);
        break;
        case NORM_FLOAT:
            if(fragBitsCount==32)
                pixelValue[0] = float32_2_f32(channelBits[0]);
            else if(fragBitsCount==16)
                pixelValue[0] = float16_2_f32((quint16)(channelBits[0]&0xFFFF));
            else if(fragBitsCount==11)
                pixelValue[0] = float11_2_f32((quint16)(channelBits[0]&0xFFFF));
            else if(fragBitsCount==10)
                pixelValue[0] = float10_2_f32((quint16)(channelBits[0]&0xFFFF));
            else
                pixelValue[0] = 0;
        break;
        default:
            pixelValue[0] = 0;
    };

    //green bits
    channelBits[1] = 0;
    fragBitsCount = 0;
    for(ib=0; ib < myGREENBitsIndices.count(); ib++)
    {
        fragBase = (unsigned int)((bitCounter+myGREENBitsIndices[ib].bitIndex)/64);
        fragOffset = (unsigned int)((bitCounter+myGREENBitsIndices[ib].bitIndex)%64);
        ltmp = MASK(fragOffset, myGREENBitsIndices[ib].bitsCount);
        ltmp &= *(quint64*)(framePtr + fragBase);
        ltmp >>= fragOffset;
        channelBits[1] |= (ltmp << fragBitsCount);
        fragBitsCount += myGREENBitsIndices[ib].bitsCount;
    }
    switch(mType[1])
    {
        case NORM_IUNSIGNED:
            pixelValue[1] = unsignedInt_2_f32(channelBits[1], channelAbsCapacity[1]);
        break;
        case NORM_ISIGNED:
            pixelValue[1] = signedInt_2_f32(channelBits[1], fragBitsCount-1,channelAbsCapacity[1]);
        break;
        case NORM_FLOAT:
            if(fragBitsCount==32)
                pixelValue[1] = float32_2_f32(channelBits[1]);
            else if(fragBitsCount==16)
                pixelValue[1] = float16_2_f32((quint16)(channelBits[1]&0xFFFF));
            else if(fragBitsCount==11)
                pixelValue[1] = float11_2_f32((quint16)(channelBits[1]&0xFFFF));
            else if(fragBitsCount==10)
                pixelValue[1] = float10_2_f32((quint16)(channelBits[1]&0xFFFF));
            else
                pixelValue[1] = 0;
        break;
        default:
            pixelValue[1] = 0;
    };

    //blue bits
    channelBits[2] = 0;
    fragBitsCount = 0;
    for(ib=0; ib < myBLUEBitsIndices.count(); ib++)
    {
        fragBase = (unsigned int)((bitCounter+myBLUEBitsIndices[ib].bitIndex)/64);
        fragOffset = (unsigned int)((bitCounter+myBLUEBitsIndices[ib].bitIndex)%64);
        ltmp = MASK(fragOffset, myBLUEBitsIndices[ib].bitsCount);
        ltmp &= *(quint64*)(framePtr + fragBase);
        ltmp >>= fragOffset;
        channelBits[2] |= (ltmp << fragBitsCount);
        fragBitsCount += myBLUEBitsIndices[ib].bitsCount;
    }
    switch(mType[2])
    {
        case NORM_IUNSIGNED:
            pixelValue[2] = unsignedInt_2_f32(channelBits[2], channelAbsCapacity[2]);
        break;
        case NORM_ISIGNED:
            pixelValue[2] = signedInt_2_f32(channelBits[2], fragBitsCount-1,channelAbsCapacity[2]);
        break;
        case NORM_FLOAT:
            if(fragBitsCount==32)
                pixelValue[2] = float32_2_f32(channelBits[2]);
            else if(fragBitsCount==16)
                pixelValue[2] = float16_2_f32((quint16)(channelBits[2]&0xFFFF));
            else if(fragBitsCount==11)
                pixelValue[2] = float11_2_f32((quint16)(channelBits[2]&0xFFFF));
            else if(fragBitsCount==10)
                pixelValue[2] = float10_2_f32((quint16)(channelBits[2]&0xFFFF));
            else
                pixelValue[2] = 0;
        break;
        default:
            pixelValue[2] = 0;
    };

    //alpha
    channelBits[3] = 0;
    fragBitsCount = 0;
    for(ib=0; ib < myALPHABitsIndices.count(); ib++)
    {
        fragBase = (unsigned int)((bitCounter+myALPHABitsIndices[ib].bitIndex)/64);
        fragOffset = (unsigned int)((bitCounter+myALPHABitsIndices[ib].bitIndex)%64);
        ltmp = MASK(fragOffset, myALPHABitsIndices[ib].bitsCount);
        ltmp &= *(quint64*)(framePtr + fragBase);
        ltmp >>= fragOffset;
        channelBits[3] |= (ltmp << fragBitsCount);
        fragBitsCount += myALPHABitsIndices[ib].bitsCount;
    }
    switch(mType[3])
    {
        case NORM_IUNSIGNED:
            pixelValue[3] = unsignedInt_2_f32(channelBits[3], channelAbsCapacity[3]);
        break;
        case NORM_ISIGNED:
            pixelValue[3] = signedInt_2_f32(channelBits[3], fragBitsCount-1,channelAbsCapacity[3]);
        break;
        case NORM_FLOAT:
            if(fragBitsCount==32)
                pixelValue[3] = float32_2_f32(channelBits[3]);
            else if(fragBitsCount==16)
                pixelValue[3] = float16_2_f32((quint16)(channelBits[3]&0xFFFF));
            else if(fragBitsCount==11)
                pixelValue[3] = float11_2_f32((quint16)(channelBits[3]&0xFFFF));
            else if(fragBitsCount==10)
                pixelValue[3] = float10_2_f32((quint16)(channelBits[3]&0xFFFF));
            else
                pixelValue[3] = 0;
        break;
        default:
            pixelValue[3] = 1.0f;
    };
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QImage CNormalizator::getImage()
{
    quint32 iw, ih;
    quint32 bitCounter = 0;

    adjustCapacity();

    QImage resImage(width, height, (channelAbsCapacity[3]>0)?QImage::Format_ARGB32:QImage::Format_RGB32);

    for(ih = 0; ih < height; ih++)
    {
        for(iw = 0; iw < width; iw++)
        {
            resImage.setPixel(iw, ih, normSinglePixel(bitCounter));
            bitCounter += columnStride;
        }
        bitCounter += rowStride;
        //progress update by ih coordinate
        (reinterpret_cast<CImgContext*>(myParentPtr))->auxInfo = "Parsing: " + QString::number((ulong)ih*100/height) + "%";
    }
  return resImage;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QImage CNormalizator::getImageWithFiltering()
{
    quint32 iw, ih;
    quint32 bitCounter = 0;

    adjustCapacity();

    QImage resImage(width, height, QImage::Format_ARGB32);

    for(ih = 0; ih < height; ih++)
    {
        for(iw = 0; iw < width; iw++)
        {
            resImage.setPixel(iw, ih,normSinglePixelWithFiltering(bitCounter));
            bitCounter += columnStride;
        }
        bitCounter += rowStride;
        //progress update by ih coordinate
        (reinterpret_cast<CImgContext*>(myParentPtr))->auxInfo = "Filtering: " + QString::number((ulong)ih*100/height) + "%";
    }
  return resImage;
}
