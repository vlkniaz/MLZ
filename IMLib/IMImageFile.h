/*
 *  IMImageFile.h
 *  Билиотека IM
 *  
 *  IMImageFile - класс для работы изображениями
 *
 *  Created by Владимир Князь on 27.01.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include <string>

#pragma pack(push, 1)

struct IMImageFormat
{
    // ширина изображения в пикселах
    int pixelsWide;
    // высота изображения в пикселах
    int pixelsHigh;
    // число бит на один канал одного пиксела изображения
    int bitsPerSample;
    // число каналов изображения
    int samplesPerPixel;
    // число байт в одной строке изображения
    int bytesPerRow;
    // определяет, есть ли в изображении альфа-канал
    bool hasAlpha;
    // определяет положение альфа-канала
    bool isAlphaFirst;
    // определяет последовательность записи каналов изображения RGBA|RGBA|RGBA.../RRR...|GGG...|BBB...|AAA...
    bool isPlanar;
};

#pragma pop()

class IMImageFileFormat;

class IMImageFile
{
private:	
	// имя файла
    std::string m_fileName;
    
    // описание изображения
    IMImageFormat m_imageFormat;
    
	// формат файла изображения
	IMImageFileFormat *m_fileFormat;
    // массив указателей на изображения
	unsigned char* m_image;
	// код последней ошибки
	int m_lastError;
    // определяет нужно ли автоматически освобождать память под изображение
    bool m_needsToFreeImage;

protected:
	// устанавливает имя файла
	void setFileName(std::string fileName);
	
public:
	// конструктор без параметров
	IMImageFile();
	
	// конструктор с одним параметром
	IMImageFile(std::string fileName);
    
    // копирующий конструктор
    IMImageFile(const IMImageFile& other);
	
	// деструктор
	~IMImageFile();
	
	// пытается загрузить свойства изображения из зданного файла
	bool open(std::string fileName);
	
	// сохраняет изображение в заданный файл
	bool save(std::string fileName);
	
	// загружает изображение
	bool loadImage() const;

	// выделяет память под изображение
	bool allocImage();
	
	// освобождает память изображения
	bool freeImage();
	
	// загружает изображение и выделяет под него память
	bool allocAndLoadImage();
    
    // создаёт копию изображения и массива данных
    IMImageFile* copy();
	
	// функции доступа к членам класса
	std::string fileName() const
	{ return m_fileName; }
	int pixelsWide() const
	{ return m_imageFormat.pixelsWide; }
	int pixelsHigh() const
	{ return m_imageFormat.pixelsHigh; }
	IMImageFileFormat* fileFormat() const
	{ return m_fileFormat; }
	int bitsPerSample() const
	{ return m_imageFormat.bitsPerSample; }
	int samplesPerPixel() const
	{ return m_imageFormat.samplesPerPixel; }
	int bytesPerRow() const
	{ return m_imageFormat.bytesPerRow; }
	bool hasAlpha() const
	{ return m_imageFormat.hasAlpha; }
	bool isAlphaFirst() const
	{ return m_imageFormat.isAlphaFirst; }
	bool isPlanar() const
	{ return m_imageFormat.isPlanar; }
	unsigned char* image() const
	{ return m_image; }
	int lastError() const
	{ return m_lastError; }
    IMImageFormat imageFormat() const
    {
        return m_imageFormat;
    }
    
	
	void setPixelsWide(int pixelsWide)
	{ m_imageFormat.pixelsWide = pixelsWide; }
	void setPixelsHigh(int pixelsHigh)
	{ m_imageFormat.pixelsHigh = pixelsHigh; }
	void setFileFormat(IMImageFileFormat* fileFormat);
	void setBitsPerSample(int bitsPerSample)
	{ m_imageFormat.bitsPerSample = bitsPerSample; }
	void setSamplesPerPixel(int samplesPerPixel)
	{ m_imageFormat.samplesPerPixel = samplesPerPixel; }
	void setBytesPerRow(int bytesPerRow)
	{ m_imageFormat.bytesPerRow = bytesPerRow; }
	void setHasAlpha(bool hasAlpha)
	{ m_imageFormat.hasAlpha = hasAlpha; }
	void setIsAlphaFirst(bool isAlphaFirst)
	{ m_imageFormat.isAlphaFirst = isAlphaFirst; }
	void setIsPlanar(bool isPlanar)
	{ m_imageFormat.isPlanar = isPlanar; }
	void setImage(unsigned char* image)
	{ m_image = image; }
    void setImageFormat(IMImageFormat imageFormat)
    {
        m_imageFormat = imageFormat;
    }
};

//IMImageFile* convert64biTo32bit(IMImageFile* image);
//void saveFlowTo32bit(IMImageFile *convolvedImage, const char* path, const char* filename);
//void save32BitFlowTo32bit(IMImageFile *convolvedImage, const char* path, const char* filename);
