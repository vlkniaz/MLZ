/*
 *  IMTIFFImageFileFormat.cpp
 *  Билиотека IM
 *  
 *  IMTIFFImageFileFormat - класс для загрузки изображений из файлов формата TIFF
 *
 *  Created by Владимир Князь on 27.01.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <IMLib/IMImageFile.h>
#include <IMLib/IMTIFFImageFileFormat.h>
#include <tiffio.h>


double secs;


// деструктор
IMTIFFImageFileFormat::~IMTIFFImageFileFormat()
{
}

// определяет, можно ли открыть данный файл
bool IMTIFFImageFileFormat::canOpenFile(std::string fileName)
{
	TIFF *tiffFile;
	// проверяем имя файла
	if(fileName.empty())
	{
		return false;
	}
	// пробуем открыть файл
	tiffFile = TIFFOpen(fileName.c_str(), "r");
	// сообщаем о результате
	if(tiffFile)
	{
		TIFFClose(tiffFile);
		return true;
	}
	else
	{
		return false;
	}
}

// пытается загрузить свойства изображения из зданного файла
bool IMTIFFImageFileFormat::open()
{
	TIFF* tiffFile;
	std::string fileName;
	
	if(m_imageFile == 0)
	{
		return false;
	}
	fileName = m_imageFile->fileName();
	// проверяем имя файла
	if(fileName.empty())
	{
		return false;
	}
	// пробуем открыть файл
	tiffFile = TIFFOpen(fileName.c_str(), "r");
	
	// если удалось, получаем сваойства изображения
	if(tiffFile)
	{
		uint32 pixelsWide, pixelsHigh;
		uint16 bitsPerSample, samplesPerPixel, extraSamplesCount, planarConfig, *extraSamples = 0;
		
		TIFFGetField(tiffFile, TIFFTAG_IMAGEWIDTH, &pixelsWide);
		TIFFGetField(tiffFile, TIFFTAG_IMAGELENGTH, &pixelsHigh);
		TIFFGetFieldDefaulted(tiffFile, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
		TIFFGetFieldDefaulted(tiffFile, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
		TIFFGetFieldDefaulted(tiffFile, TIFFTAG_EXTRASAMPLES, &extraSamplesCount, &extraSamples);
		TIFFGetFieldDefaulted(tiffFile, TIFFTAG_PLANARCONFIG, &planarConfig);
		
		m_imageFile->setPixelsWide(pixelsWide);
		m_imageFile->setPixelsHigh(pixelsHigh);
		m_imageFile->setBitsPerSample(bitsPerSample);
		m_imageFile->setSamplesPerPixel(samplesPerPixel);
		m_imageFile->setBytesPerRow((bitsPerSample / 8) * samplesPerPixel * pixelsWide);
		if(planarConfig == 1)
		{
			m_imageFile->setIsPlanar(false);			
		}
		else
		{
			m_imageFile->setIsPlanar(true);
		}
		if(extraSamplesCount > 0)
		{
			m_imageFile->setHasAlpha(true);
		}
		m_imageFile->setIsAlphaFirst(false);
		
		TIFFClose(tiffFile);
		
		return true;
	}
	// в противном случае сообщаем о неудаче
	else
	{
		return false;
	}
}

// сохраняет изображение в заданный файл
bool IMTIFFImageFileFormat::save()
{
	TIFF* tiffFile;
	std::string fileName;
	
	if(m_imageFile == 0)
	{
		return false;
	}
	if(m_imageFile->pixelsWide() == 0 || m_imageFile->pixelsHigh() == 0)
	{
		return false;
	}
	fileName = m_imageFile->fileName();
	// проверяем имя файла
	if(fileName.empty())
	{
		return false;
	}
	// пробуем открыть файл
	tiffFile = TIFFOpen(fileName.c_str(), "w");
	
	// если удалось, сохраняем изображение
	if(tiffFile)
	{
		int row, imageHeight, imageWidth, bitsPerSample, samplesPerPixel, extraSampleCount, planarConfig, bytesPerRow;
		unsigned char* image = 0;
		
		// получаем указатель на массив изображения
		image = m_imageFile->image();
		if(image == 0)
		{
			return false;
		}
		
		imageHeight = m_imageFile->pixelsHigh();
		imageWidth = m_imageFile->pixelsWide();
		bitsPerSample = m_imageFile->bitsPerSample();
		samplesPerPixel = m_imageFile->samplesPerPixel();
		bytesPerRow = m_imageFile->bytesPerRow();
		
		// устанавливаем параметры TIFF файла
		TIFFSetField(tiffFile, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
		TIFFSetField(tiffFile, TIFFTAG_IMAGEWIDTH, imageWidth);
		TIFFSetField(tiffFile, TIFFTAG_IMAGELENGTH, imageHeight);
		TIFFSetField(tiffFile, TIFFTAG_BITSPERSAMPLE, bitsPerSample);
		TIFFSetField(tiffFile, TIFFTAG_SAMPLESPERPIXEL, samplesPerPixel);
        TIFFSetField(tiffFile, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		
		TIFFSetField(tiffFile, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        
        if(m_imageFile->bitsPerSample() == 32)
        {
            TIFFSetField(tiffFile, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
        }
		
		// засекаем время
//		tim1 = mach_absolute_time();
		//SwapUInt16ArrayBigToHost(image16, 800*600);
		//TIFFSwabArrayOfShort(image16, 1280*960);
		//SwapU16(image16, 800*600);
		
		
			
		// сохраняем изображение по строкам
		for(row = 0; row < imageHeight; row++)
		{
			TIFFWriteScanline(tiffFile, image, row);
			image += bytesPerRow;
		}
		
		// закрываем файл
		TIFFClose(tiffFile);
		
/*		tim2  = mach_absolute_time();
		tim2 = tim2 - tim1; 
		mach_timebase_info_data_t info; 
		mach_timebase_info(&info); 
		secs = tim2 * info.numer / info.denom;
		secs /= 1000000000.0;*/
		//printf("Save %f\n", secs);
		
		return true;
	}
	
	return false;
}

// загружает изображение с заданным индексом
bool IMTIFFImageFileFormat::loadImage() const
{
	TIFF* tiffFile;
	std::string fileName;
	
	if(m_imageFile == 0)
	{
		return false;
	}
	if(m_imageFile->pixelsWide() == 0 || m_imageFile->pixelsHigh() == 0)
	{
		return false;
	}
	fileName = m_imageFile->fileName();
	// проверяем имя файла
	if(fileName.empty())
	{
		return false;
	}
	// пробуем открыть файл
	tiffFile = TIFFOpen(fileName.c_str(), "r");
	
	// если удалось, загружаем изображение
	if(tiffFile)
	{
		int row, imageHeight, bytesPerRow;
		unsigned char* image = 0;
		
		// получаем указатель на массив изображения
		image = m_imageFile->image();
		if(image == 0)
		{
			return false;
		}
		imageHeight = m_imageFile->pixelsHigh();
		bytesPerRow = m_imageFile->bytesPerRow();
		
		// читаем изображение по строкам
		for(row = 0; row < imageHeight; row++)
		{
			TIFFReadScanline(tiffFile, image, row);
			image += bytesPerRow;
		}
		
		// закрываем файл
		TIFFClose(tiffFile);
		
		return true;
	}
		
	return false;
}
