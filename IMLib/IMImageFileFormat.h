/*
 *  IMImageFileFormat.h
 *  Билиотека IM
 *  
 *  IMImageFileFormat - абстрактный класс для загрузки изображений из файла
 *
 *  Created by Владимир Князь on 27.01.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

class IMImageFile;

class IMImageFileFormat
{
protected:
	// указатель на файл изображения
	IMImageFile *m_imageFile;
	// расширение файла данного формата
	const char *m_extension;
	
public:
	// конструктор без параметров
	IMImageFileFormat()
	{ m_imageFile = 0; }
	
	// деструктор
	virtual ~IMImageFileFormat()
	{ }
	
	// возвращает расширение файла данного формата
	const char* extension() const
	{ return m_extension; }
	
	// пытается загрузить свойства изображения из зданного файла
	virtual bool open() = 0;
	
	// сохраняет изображение в заданный файл
	virtual bool save() = 0;
	
	// загружает изображение
	virtual bool loadImage() const = 0;
	
	// устанавливает указатель на файл изображения
	void setImageFile(IMImageFile* imageFile)
	{ m_imageFile = imageFile; }
    
    // возвращает указатель на файл изображения
    IMImageFile* imageFile() const
    { return m_imageFile; }
};