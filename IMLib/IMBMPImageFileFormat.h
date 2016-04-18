/*
 *  IMBMPImageFileFormat.h
 *  Билиотека IM
 *  
 *  IMTIFFImageFileFormat - класс для загрузки изображений из файлов формата BMP
 *
 *  Created by Владимир Князь on 27.01.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include <IMLib/IMImageFileFormat.h>
#include <stdlib.h>

static const char* IMBMPImageFileFormatExtension = "bmp";

class IMImageFile;

class IMBMPImageFileFormat : public IMImageFileFormat
{
private:
	int m_imageOffset;

public:	
	// конструктор
	IMBMPImageFileFormat() : IMImageFileFormat()
	{ m_extension = IMBMPImageFileFormatExtension; }
	
	// деструктор
	~IMBMPImageFileFormat();
		
	// определяет, можно ли открыть данный файл
	static bool canOpenFile(std::string fileName);
	
	// пытается загрузить свойства изображения из зданного файла
	bool open();
	
	// сохраняет изображение в заданный файл
	bool save();
	
	// загружает изображение с заданным индексом
	bool loadImage() const;
};