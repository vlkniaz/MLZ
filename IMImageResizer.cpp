/*
*  IMImageFile.cpp
*  Билиотека IM
*
*  IMImageResizer - класс для изменения размеров и обрезки изображений
*
*  Created by Владимир Князь on 27.01.10.
*  Copyright 2010 __MyCompanyName__. All rights reserved.
*
*/

#include <IMLib/IMImageResizer.h>
#include <iostream>
#include <stdio.h>
#include <string.h>

using namespace std;

// создаёт новое изображение из заданной области исходного изображения
IMImageFile* IMImageResizer::newImageFromRegionWithPadding(IMRegion region)
{
	IMImageFile* newImage = 0;

	if (m_image == 0)
		return 0;

	int width, height, y, start, jump, bytesPerPixel, rowSize, shift, startDst, sizeToCopy;
	// перекрывающаяся область в системе координат заданного изображения
	IMRegion overlapRegion, overlapRegionDst;
	unsigned char *curSrcPixel, *curDstPixel;
	width = m_image->pixelsWide();
	height = m_image->pixelsHigh();
	bytesPerPixel = m_image->bitsPerSample() * m_image->samplesPerPixel() / 8;



	overlapRegion = region;
	if (overlapRegion.origin.x < 0)
	{
		overlapRegion.size.width += overlapRegion.origin.x;
		overlapRegion.origin.x = 0;		
	}
	if (overlapRegion.origin.y < 0)
	{
		overlapRegion.size.height += overlapRegion.origin.y;
		overlapRegion.origin.y = 0;		
	}
	if (overlapRegion.size.width + overlapRegion.origin.x > width)
	{
		overlapRegion.size.width = width - overlapRegion.origin.x;
	}
	if (overlapRegion.size.height + overlapRegion.origin.y > height)
	{
		overlapRegion.size.height = height - overlapRegion.origin.y;
	}

	if (overlapRegion.size.width <= 0 || overlapRegion.size.height <= 0)
	{
		return 0;
	}

	overlapRegionDst = overlapRegion;
	if (region.origin.x < 0)
	{
		overlapRegionDst.origin.x = -region.origin.x;
	}
	else
	{
		overlapRegionDst.origin.x = 0.0;
	}
	if (region.origin.y < 0)
	{
		overlapRegionDst.origin.y = -region.origin.y;
	}
	else
	{
		overlapRegionDst.origin.y = 0.0;
	}

	// создаём новое изображение
	newImage = new IMImageFile;
	newImage->setPixelsWide(static_cast<int>(region.size.width));
	newImage->setPixelsHigh(static_cast<int>(region.size.height));
	newImage->setBitsPerSample(m_image->bitsPerSample());
	newImage->setSamplesPerPixel(m_image->samplesPerPixel());
	newImage->setBytesPerRow(static_cast<int>(region.size.width * bytesPerPixel));
	newImage->allocImage();
	memset(newImage->image(), 0, newImage->bytesPerRow()*newImage->pixelsHigh());

	start = static_cast<int>((overlapRegion.origin.x + overlapRegion.origin.y * width) * bytesPerPixel);
	startDst = static_cast<int>((overlapRegionDst.origin.x + overlapRegionDst.origin.y * newImage->pixelsWide()) * bytesPerPixel);
	rowSize = newImage->pixelsWide() * bytesPerPixel;
	jump = width * bytesPerPixel;
	curSrcPixel = m_image->image() + start;
	curDstPixel = newImage->image() + startDst;
	sizeToCopy = overlapRegion.size.width * bytesPerPixel;

	// копируем заданную область в новое изображение
	for (y = 0; y < overlapRegion.size.height; y++)
	{
		memcpy(curDstPixel, curSrcPixel, sizeToCopy);
		curDstPixel += rowSize;
		curSrcPixel += jump;
	}

	return newImage;
}

// создаёт новое изображение из заданной области исходного изображения
IMImageFile* IMImageResizer::newImageFromRegion(IMRegion region)
{
	IMImageFile* newImage = 0;

	if (m_image == 0)
		return 0;

	int width, height, y, start, jump, bytesPerPixel, rowSize;
	unsigned char *curSrcPixel, *curDstPixel;
	width = m_image->pixelsWide();
	height = m_image->pixelsHigh();
	bytesPerPixel = m_image->bitsPerSample() * m_image->samplesPerPixel() / 8;

	if (region.origin.x < 0 || region.origin.y < 0 || region.origin.x + region.size.width > width || region.origin.y + region.size.height > height)
	{
		return newImageFromRegionWithPadding(region);
	}
	

	// создаём новое изображение
	newImage = new IMImageFile;
	newImage->setPixelsWide(static_cast<int>(region.size.width));
	newImage->setPixelsHigh(static_cast<int>(region.size.height));
	newImage->setBitsPerSample(m_image->bitsPerSample());
	newImage->setSamplesPerPixel(m_image->samplesPerPixel());
	newImage->setBytesPerRow(static_cast<int>(region.size.width * bytesPerPixel));
	newImage->allocImage();

	start = static_cast<int>((region.origin.x + region.origin.y * width) * bytesPerPixel);
	rowSize = newImage->bytesPerRow();
	jump = width * bytesPerPixel;
	curSrcPixel = m_image->image() + start;
	curDstPixel = newImage->image();

	// копируем заданную область в новое изображение
	for (y = 0; y < region.size.height; y++)
	{
		memcpy(curDstPixel, curSrcPixel, rowSize);
		curDstPixel += rowSize;
		curSrcPixel += jump;
	}

	return newImage;
}

// создаёт новое изображение с заданным числои бит на пиксел из заданной области исходного изображения
IMImageFile* IMImageResizer::newImageFromRegion(IMRegion region, int bitsPerSample)
{
    IMImageFile* newImage;
    int bytesPerPixel = bitsPerSample * m_image->samplesPerPixel() / 8;
    int x, y, srcX, srcY;
    int srcWidth, srcHeight, width, height;
    
    srcWidth = m_image->pixelsWide();
    srcHeight = m_image->pixelsHigh();
    width = region.size.width;
    height = region.size.height;
    
    // создаём новое изображение
    newImage = new IMImageFile;
    newImage->setPixelsWide(static_cast<int>(width));
    newImage->setPixelsHigh(static_cast<int>(height));
    newImage->setBitsPerSample(bitsPerSample);
    newImage->setSamplesPerPixel(m_image->samplesPerPixel());
    newImage->setBytesPerRow(static_cast<int>(region.size.width * bytesPerPixel));
    newImage->allocImage();
    
    if(bitsPerSample == 64)
    {
        unsigned char *srcImage8 = reinterpret_cast<unsigned char*>(m_image->image());
        double *newImage64 = reinterpret_cast<double*>(newImage->image());

        // копируем заданную область в новое изображение
        for (y = 0; y < region.size.height; y++)
        {
            for(x = 0; x < region.size.width; x++)
            {
                srcX = x + region.origin.x;
                srcY = y + region.origin.y;
                
                newImage64[x + y * width] = srcImage8[srcX + srcY * srcWidth];
            }
        }
    }
    if(bitsPerSample == 32)
    {
        unsigned char *srcImage8 = reinterpret_cast<unsigned char*>(m_image->image());
        float *newImage32 = reinterpret_cast<float*>(newImage->image());
        
        // копируем заданную область в новое изображение
        for (y = 0; y < region.size.height; y++)
        {
            for(x = 0; x < region.size.width; x++)
            {
                srcX = x + region.origin.x;
                srcY = y + region.origin.y;
                
                newImage32[x + y * width] = srcImage8[srcX + srcY * srcWidth];
            }
        }
    }
    
    return newImage;
}
