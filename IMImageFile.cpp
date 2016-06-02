/*
 *  IMImageFile.cpp
 *  Билиотека IM
 *  
 *  IMImageFile - класс для работы изображениями
 *
 *  Created by Владимир Князь on 27.01.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <IMLib/IMImageFile.h>
#include <IMLib/IMImageFileFormat.h>
#include <IMLib/IMTIFFImageFileFormat.h>
#include <IMLib/IMBMPImageFileFormat.h>

// конструктор без параметров
IMImageFile::IMImageFile()
{
	// инициализируем всех членов класса нулевыми значениями
	m_fileName = "";
	m_imageFormat.pixelsWide = 0;
	m_imageFormat.pixelsHigh = 0;
	m_fileFormat = 0;
	m_imageFormat.bitsPerSample = 0;
	m_imageFormat.samplesPerPixel = 0;
	m_imageFormat.bytesPerRow = 0;
	m_imageFormat.hasAlpha = 0;
	m_imageFormat.isAlphaFirst = 0;
	m_imageFormat.isPlanar = 0;
	m_image = 0;
	m_lastError = 0;
    m_needsToFreeImage = 0;
}

// копирующий конструктор
IMImageFile::IMImageFile(const IMImageFile& other):
m_imageFormat(other.m_imageFormat),
m_fileFormat(other.m_fileFormat),
m_image(other.m_image),
m_lastError(other.m_lastError)
{
    m_fileName = "";
    m_fileFormat = 0;
    m_needsToFreeImage = 0;
    
	// копируем имя файла
    setFileName(other.fileName());
    
    // копируем формат файла
    setFileFormat(other.fileFormat());
}

// конструктор с одним параметром
IMImageFile::IMImageFile(std::string fileName)
{
	// инициализируем всех членов класса нулевыми значениями
	m_fileName = "";
    m_imageFormat.pixelsWide = 0;
    m_imageFormat.pixelsHigh = 0;
    m_fileFormat = 0;
    m_imageFormat.bitsPerSample = 0;
    m_imageFormat.samplesPerPixel = 0;
    m_imageFormat.bytesPerRow = 0;
    m_imageFormat.hasAlpha = 0;
    m_imageFormat.isAlphaFirst = 0;
    m_imageFormat.isPlanar = 0;
	m_image = 0;
	m_lastError = 0;
	
	// пробуем открыть файл
	open(fileName);
}

// деструктор
IMImageFile::~IMImageFile()
{    
	// освобождаем ресурсы
	if(m_image != 0)
	{
        //printf("Free image\n");
		free(m_image);
	}
	if(m_fileFormat != 0 && m_fileFormat->imageFile() == this)
	{
		free(m_fileFormat);
	}
}

// пытается загрузить свойства изображения из зданного файла
bool IMImageFile::open(std::string fileName)
{
	// определяем тип файл и пробуем открыть его
	if(IMTIFFImageFileFormat::canOpenFile(fileName))
	{		
		setFileName(fileName);
		
		// создаём объект загрузки изображений формата TIFF
		setFileFormat(new IMTIFFImageFileFormat());
				
		// читаем свойства изображения
		return m_fileFormat->open();
	}

	if (IMBMPImageFileFormat::canOpenFile(fileName))
	{
		setFileName(fileName);

		// создаём объект загрузки изображений формата TIFF
		setFileFormat(new IMBMPImageFileFormat());

		// читаем свойства изображения
		return m_fileFormat->open();
	}
	
	return false;
}

// сохраняет изображение в заданный файл
bool IMImageFile::save(std::string fileName)
{
	if(m_fileFormat != 0)
	{
		setFileName(fileName);
		return m_fileFormat->save();
	}
	else
	{
		return false;
	}
}

// загружает изображение
bool IMImageFile::loadImage() const
{
	// проверяем формат файла
	if(m_fileFormat == 0)
	{
		return false;
	}
	else
	{
		return m_fileFormat->loadImage();		
	}
}

// выделяет память под изображение
bool IMImageFile::allocImage()
{
    //printf("Alloc image\n");
    
	// проверяем настройки изображения
	if(m_imageFormat.pixelsWide == 0 || m_imageFormat.pixelsHigh == 0 || m_imageFormat.bitsPerSample == 0 || m_imageFormat.samplesPerPixel == 0 || m_imageFormat.bytesPerRow == 0)
	{
		return false;
	}
	else
	{
		int imageSize = m_imageFormat.bytesPerRow * m_imageFormat.pixelsHigh;
		m_image = (unsigned char*)malloc(imageSize);
        m_needsToFreeImage = true;
		
		return true;
	}
}

// освобождает память изображения
bool IMImageFile::freeImage()
{
    //printf("Free image\n");
    
	// проверяем изображение
	if(m_image == 0)
	{
		return false;
	}
	else
	{
		free(m_image);
		
		return true;
	}
}

// загружает изображение и выделяет под него память
bool IMImageFile::allocAndLoadImage()
{
	// проверяем формат файла 
	if(m_fileFormat == 0)
	{
		return false;
	}
	
	allocImage();
	return m_fileFormat->loadImage();
}

// создаёт копию изображения и массива данных
IMImageFile* IMImageFile::copy()
{
    // проверяем формат файла
    IMImageFile *newImage = new IMImageFile(*this);
    
    newImage->allocImage();
    memcpy(newImage->m_image, m_image, m_imageFormat.bytesPerRow*m_imageFormat.pixelsHigh);
    
    return newImage;
}

// -------------------------------------------------------------
// функции доступа к членам класса
// -------------------------------------------------------------

// устанавливает имя файла
void IMImageFile::setFileName(std::string fileName)
{
    m_fileName = fileName;
}

// устанавливает формат файла
void IMImageFile::setFileFormat(IMImageFileFormat* fileFormat)
{
	if(m_fileFormat != fileFormat)
	{
		if(m_fileFormat != 0)
		{
			delete m_fileFormat;
		}
		m_fileFormat = fileFormat;
	}
    if(m_fileFormat != 0)
    {
        m_fileFormat->setImageFile(this);
    }
}