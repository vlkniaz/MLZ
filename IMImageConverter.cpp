/*
*  IMImageConverter.cpp
*  Билиотека IM
*
*  IMImageResizer - класс для изменения размеров и обрезки изображений
*
*  Created by Владимир Князь on 27.01.10.
*  Copyright 2010 __MyCompanyName__. All rights reserved.
*
*/

#include <IMLib/IMImageConverter.h>
#include <iostream>
#include <stdio.h>
#include <string.h>

using namespace std;

const int IMImageConverter::DITHERING_THRESHOLD = 1;
const int IMImageConverter::DITHERING_RANDOM = 2;

// создаёт новое изображение с помощью дизеринга
IMImageFile* IMImageConverter::newImageWithDithering(int mode, int threshold)
{
	IMImageFile* newImage = 0;
    
    if (m_image == 0)
        return 0;
    
    int width, height, x, y, t = threshold;
    unsigned char *pData, *pNewData, val;
    width = m_image->pixelsWide();
    height = m_image->pixelsHigh();
    
    // создаём новое изображение
    newImage = new IMImageFile;
    newImage->setPixelsWide(width);
    newImage->setPixelsHigh(height);
    newImage->setBitsPerSample(m_image->bitsPerSample());
    newImage->setSamplesPerPixel(m_image->samplesPerPixel());
    newImage->setBytesPerRow(m_image->bytesPerRow());
    newImage->allocImage();
    memset(newImage->image(), 0, newImage->bytesPerRow()*newImage->pixelsHigh());
    
    pData = m_image->image();
    pNewData = newImage->image();
    
    for(y = 0; y < height; y++)
    {
        for(x = 0; x < width; x++)
        {
            switch(mode)
            {
                case DITHERING_RANDOM:
                    t = rand() % 256;
                    break;
            }
            val = *pData;
            if(val >= t)
            {
                val = 255;
            }
            else
            {
                val = 0;
            }
            *pNewData = val;
            pNewData++;
            pData++;
        }
    }

	return newImage;
}

