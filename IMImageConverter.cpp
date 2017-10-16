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
const int IMImageConverter::DITHERING_ORDERD = 3;
const int IMImageConverter::DITHERING_DIFFUSION_FORWARD = 4;
const int IMImageConverter::DITHERING_FLOYD = 5;
const int IMImageConverter::DITHERING_DIFFUSION_FORWARD_BACKWARD = 6;

// создаёт новое изображение с помощью дизеринга
IMImageFile* IMImageConverter::newImageWithDithering(int mode, int threshold)
{
	IMImageFile* newImage = 0;
    
    if (m_image == 0)
        return 0;
    
    int width, height, x, y, t = threshold;
    float error = 0.0;
    float *image32 = 0, *pData32;
    unsigned char *pData, *pNewData;
    int val, oldVal;
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
    

    
    // создаём копию изображения
    if(mode >= DITHERING_DIFFUSION_FORWARD)
    {
        image32 = new float[width * height];
        
        pData = m_image->image();
        pData32 = image32;
        
        for(y = 0; y < height; y++)
        {
            for(x = 0; x < width; x++)
            {
                *pData32 = *pData;
                
                pData++;
                pData32++;
            }
        }
    }
    
    pData = m_image->image();
    pNewData = newImage->image();
    pData32 = image32;
    
    for(y = 0; y < height; y++)
    {
        if(mode < DITHERING_DIFFUSION_FORWARD_BACKWARD)
        {
            for(x = 0; x < width; x++)
            {
                val = *pData;
                
                // если дизеринг без распространения ошибки
                if(mode < DITHERING_DIFFUSION_FORWARD)
                {
                    switch(mode)
                    {
                        case DITHERING_RANDOM:
                            t = rand() % 256;
                            break;
                        case DITHERING_ORDERD:
                            int x_odd, y_odd;
                            x_odd = x % 2;
                            y_odd = y % 2;
                            if(x_odd && y_odd)
                            {
                                t = 0 * 64;
                            }
                            else if(x_odd && !y_odd)
                            {
                                t = 2 * 64;
                            }
                            else if(!x_odd && y_odd)
                            {
                                t = 3 * 64;
                            }
                            else
                            {
                                t = 1 * 64;
                            }
                            break;
                    }
                    if(val > t)
                    {
                        val = 255;
                    }
                    else
                    {
                        val = 0;
                    }
                }
                else if(mode == DITHERING_DIFFUSION_FORWARD)
                {
                    val = *pData32;
                    oldVal = val;
                    
                    if(val > t)
                    {
                        val = 255;
                    }
                    else
                    {
                        val = 0;
                    }
                    
                    error =  oldVal - val;
                    if(x < (width - 1))
                    {
                        *(pData32+1) += error;
                    }
                }
                // дизеринг по алгоритму Флойда-Штейнберга
                else if(mode == DITHERING_FLOYD)
                {
                    val = *pData32;
                    oldVal = val;
                    
                    if(val > t)
                    {
                        val = 255;
                    }
                    else
                    {
                        val = 0;
                    }
                    
                    error =  oldVal - val;
                    if(x < (width - 1))
                    {
                        *(pData32+1) += 7.0/16.0 * error;
                    }
                    if(y < (height - 1))
                    {
                        *(pData32+width) += 5.0/16.0 * error;
                        *(pData32+width-1) += 3.0/16.0 * error;
                        if(x < (width - 1))
                        {
                            *(pData32+width+1) += 1.0/16.0 * error;
                        }
                    }
                }
                
                *pNewData = val;
                pNewData++;
                pData++;
                pData32++;
            }
        }
        // методы с чередованием строк
        else
        {
            int doX = 1;
            int step = 1;
            int start = 0;
            if(y % 2)
            {
                step = -1;
                start = width - 1;
            }
            for(x = start; doX; x += step)
            {
                pData = &m_image->image()[y*width + x];
                pNewData = &newImage->image()[y*width + x];
                pData32 = &image32[y*width + x];
                
                if(mode == DITHERING_DIFFUSION_FORWARD_BACKWARD)
                {
                    val = *pData32;
                    oldVal = val;
                    
                    if(val > t)
                    {
                        val = 255;
                    }
                    else
                    {
                        val = 0;
                    }
                    
                    error =  oldVal - val;

                }
                
                *pNewData = val;
                
                if(step > 0)
                {
                    doX = (x < width);
                    
                    if(x < (width - 1))
                    {
                        *(pData32+1) += error;
                    }
                }
                else
                {
                    doX = (x >= 0);
                    
                    if(x > 0)
                    {
                        *(pData32-1) += error;
                    }
                }
            }
        }
    }

    // создаём копию изображения
    if(mode >= DITHERING_DIFFUSION_FORWARD)
    {
        delete [] image32;
    }
    
	return newImage;
}

