/*
 *  IMBMPImageFileFormat.cpp
 *  Билиотека IM
 *  
 *  IMBMPImageFileFormat - класс для загрузки изображений из файлов формата BMP
 *
 *  Created by Владимир Князь on 27.01.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <IMLib/IMImageFile.h>
#include <IMLib/IMBMPImageFileFormat.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#pragma pack(push, 1)

// код BMP файла
uint16_t IMBMPFileFormatMagicNumber = 0x4D42;

// заголовок BMP файла
typedef struct _IMBMPFileFormatHeader
{
	uint32_t fileSize;
	uint16_t customData1;
	uint16_t customData2;
	uint32_t imageOffset;
} IMBMPFileFormatHeader;

// информация для чтения BMP файла
typedef struct _IMBMPFileFormatInformationHeader
{
	uint32_t headerSize;
	int32_t imageWidth;
	int32_t imageHeight;
	uint16_t numberOfColorPlanes;
	uint16_t bitsPerPixel;
	uint32_t compressionType;
	uint32_t imageSize;
	int32_t horizontalResolution;
	int32_t verticalResolution;
	uint32_t paletteColorsCount;
	uint32_t numberOfImportantColors;
} IMBMPFileFormatInformationHeader;

// один элемент палитры BMP файла
typedef struct _IMBMPFileFormatPaletteElement
{
	unsigned char  red;
	unsigned char  green;
	unsigned char  blue;
	unsigned char  reserved;
} IMBMPFileFormatPaletteElement;


// палитра BMP файла
typedef struct _IMBMPFileFormatPalette
{
	IMBMPFileFormatPaletteElement palette[256];
} IMBMPFileFormatPalette;

#pragma pack(pop)

// деструктор
IMBMPImageFileFormat::~IMBMPImageFileFormat()
{
}

// определяет, можно ли открыть данный файл
bool IMBMPImageFileFormat::canOpenFile(std::string fileName)
{
	// проверяем имя файла
	if(fileName.empty())
	{
		return false;
	}
	// TEMP
	return true;
	/*// пробуем открыть файл
	tiffFile = TIFFOpen(fileName, "r");
	// сообщаем о результате
	if(tiffFile)
	{
		TIFFClose(tiffFile);
		return true;
	}
	else
	{
		return false;
	}*/
}

// пытается загрузить свойства изображения из зданного файла
bool IMBMPImageFileFormat::open()
{
	FILE* bmpFile;
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
	bmpFile = fopen(fileName.c_str(), "rb");
	
	// если удалось, получаем сваойства изображения
	if(bmpFile)
	{
		IMBMPFileFormatHeader bmpFileHeader;
		IMBMPFileFormatInformationHeader bmpInformationHeader;
		IMBMPFileFormatPalette bmpPalette;

		// пишем заголовок BMP файла
		fread(&IMBMPFileFormatMagicNumber, 1, 2, bmpFile);
		fread(&bmpFileHeader, 1, sizeof(IMBMPFileFormatHeader), bmpFile);
		fread(&bmpInformationHeader, 1, sizeof(IMBMPFileFormatInformationHeader), bmpFile);
		fread(&bmpPalette, 1, sizeof(IMBMPFileFormatPalette), bmpFile);
				
		m_imageFile->setPixelsWide(bmpInformationHeader.imageWidth);
		m_imageFile->setPixelsHigh(bmpInformationHeader.imageHeight);
		m_imageFile->setSamplesPerPixel(bmpInformationHeader.bitsPerPixel / 8);
		m_imageFile->setBitsPerSample(bmpInformationHeader.bitsPerPixel / m_imageFile->samplesPerPixel());
		m_imageFile->setBytesPerRow((m_imageFile->bitsPerSample() / 8) * m_imageFile->samplesPerPixel() * m_imageFile->pixelsWide());
		m_imageOffset = bmpFileHeader.imageOffset;

		m_imageFile->setHasAlpha(false);

		m_imageFile->setIsAlphaFirst(false);
		
		fclose(bmpFile);
		
		return true;
	}
	// в противном случае сообщаем о неудаче
	else
	{
		return false;
	}
}

// сохраняет изображение в заданный файл
bool IMBMPImageFileFormat::save()
{
	FILE* bmpFile;
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
	bmpFile = fopen(fileName.c_str(), "wb");
	
	// если удалось, сохраняем изображение
	if(bmpFile)
	{
		IMBMPFileFormatHeader bmpFileHeader;
		IMBMPFileFormatInformationHeader bmpInformationHeader;
		IMBMPFileFormatPalette bmpPalette;
		int j, k, imageHeight, imageWidth, bitsPerSample, samplesPerPixel, bytesPerRow, alignedBytesPerRow, imageSize, paddingBytes;
		unsigned char *image = 0, *reverseImage = 0;
		
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
		
		alignedBytesPerRow = samplesPerPixel * imageWidth;
		if (alignedBytesPerRow % 4 != 0)
			alignedBytesPerRow += 4 - alignedBytesPerRow % 4;
		paddingBytes = alignedBytesPerRow - bytesPerRow;
		imageSize = alignedBytesPerRow * imageHeight;
			
		// создаём заголовок BMP файла
		bmpFileHeader.fileSize = sizeof(IMBMPFileFormatMagicNumber) + sizeof(IMBMPFileFormatHeader) + sizeof(IMBMPFileFormatInformationHeader) + imageSize;
		bmpFileHeader.customData1 = 0;
		bmpFileHeader.customData2 = 0;
		bmpFileHeader.imageOffset = sizeof(IMBMPFileFormatMagicNumber) + sizeof(IMBMPFileFormatHeader) + sizeof(IMBMPFileFormatInformationHeader);
		if(samplesPerPixel == 1 && bitsPerSample == 8)
		{
			bmpFileHeader.fileSize += sizeof(IMBMPFileFormatPalette);
			bmpFileHeader.imageOffset += sizeof(IMBMPFileFormatPalette);
		}
		
		bmpInformationHeader.headerSize = sizeof(IMBMPFileFormatInformationHeader);
		bmpInformationHeader.imageWidth = imageWidth;
		bmpInformationHeader.imageHeight = imageHeight;
		bmpInformationHeader.numberOfColorPlanes = 1;
		bmpInformationHeader.bitsPerPixel = bitsPerSample * samplesPerPixel;
		bmpInformationHeader.compressionType = 0;
		bmpInformationHeader.imageSize = 0;
		bmpInformationHeader.horizontalResolution = 0;
		bmpInformationHeader.verticalResolution = 0;
		bmpInformationHeader.paletteColorsCount = 0;
		if (samplesPerPixel == 1 && bitsPerSample == 8)
		{
			bmpInformationHeader.paletteColorsCount = 256;
		}
		bmpInformationHeader.numberOfImportantColors = 0;
		
		// заполняем палитру BMP файла
		for(j = 0; j < 256; j++)
		{
			bmpPalette.palette[j].red = j;
			bmpPalette.palette[j].green = j;
			bmpPalette.palette[j].blue = j;
			bmpPalette.palette[j].reserved = 0;
		}
		
		// пишем заголовок BMP файла
		fwrite(&IMBMPFileFormatMagicNumber, sizeof(IMBMPFileFormatMagicNumber), 1, bmpFile);
		fwrite(&bmpFileHeader, sizeof(IMBMPFileFormatHeader), 1, bmpFile);
		fwrite(&bmpInformationHeader, sizeof(IMBMPFileFormatInformationHeader), 1, bmpFile);
		if (samplesPerPixel == 1 && bitsPerSample == 8)
		{
			fwrite(&bmpPalette, sizeof(IMBMPFileFormatPalette), 1, bmpFile);
		}
		
		// записываем строки изображения в обратном порядке (снизу вверх), как этого требует формат BMP
		reverseImage = image + bytesPerRow * (imageHeight - 1);
		// нецветное изображение
		if (m_imageFile->samplesPerPixel() == 1)
		{
			for (j = 0; j < imageHeight; j++)
			{
				fwrite(reverseImage, alignedBytesPerRow, 1, bmpFile);
				reverseImage -= bytesPerRow;
			}
		}
		// цветное изображение
		else
		{
			uint8_t* rowBuffer = new unsigned char[alignedBytesPerRow];
			uint8_t *curPixel, *curImagePixel;

			for (j = 0; j < imageHeight; j++)
			{
				memset(rowBuffer, 0, alignedBytesPerRow);
				curPixel = rowBuffer;
				curImagePixel = reverseImage;

				for (k = 0; k < imageWidth; k++)
				{
					curPixel[0] = curImagePixel[2];
					curPixel[1] = curImagePixel[1];
					curPixel[2] = curImagePixel[0];

					curPixel += 3;
					curImagePixel += 3;
				}
				
				// записываем одну строку
				fwrite(rowBuffer, alignedBytesPerRow, 1, bmpFile);

				reverseImage -= bytesPerRow;
			}

			delete rowBuffer;
		}
		
		fclose(bmpFile);
		
		return true;
	}
	
	return false;
}

// загружает изображение с заданным индексом
bool IMBMPImageFileFormat::loadImage() const
{
	FILE* bmpFile;
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
	bmpFile = fopen(fileName.c_str(), "rb");
	
	// если удалось, загружаем изображение
	if(bmpFile)
	{
		int j, k, imageWidth, imageHeight, bytesPerRow, alignedBytesPerRow, samplesPerPixel;
		unsigned char* image = 0;
		unsigned char* reverseImage = 0;

		// получаем указатель на массив изображения
		image = m_imageFile->image();
		if(image == 0)
		{
			return false;
		}
		imageWidth = m_imageFile->pixelsWide();
		imageHeight = m_imageFile->pixelsHigh();
		bytesPerRow = m_imageFile->bytesPerRow();
		samplesPerPixel = m_imageFile->samplesPerPixel();

		alignedBytesPerRow = samplesPerPixel * imageWidth;
		if (alignedBytesPerRow % 4 != 0)
			alignedBytesPerRow += 4 - alignedBytesPerRow % 4;

		fseek(bmpFile, m_imageOffset, SEEK_SET);
		
		// читаем изображение по строкам в обратном порядке (снизу вверх), как этого требует формат BMP
		reverseImage = image + bytesPerRow * (imageHeight - 1);
		// нецветное изображение
		if (m_imageFile->samplesPerPixel() == 1)
		{
			for (j = 0; j < imageHeight; j++)
			{
				fread(reverseImage, alignedBytesPerRow, 1, bmpFile);
				reverseImage -= bytesPerRow;
			}
		}
		// цветное изображение
		else
		{
			unsigned char* rowBuffer = new unsigned char[alignedBytesPerRow];

			for (j = 0; j < imageHeight; j++)
			{
				// читаем одну строку
				fread(rowBuffer, alignedBytesPerRow, 1, bmpFile);

				for (k = 0; k < imageWidth * 3; k += 3)
				{
					// B
					reverseImage[k + 2] = rowBuffer[k + 0];
					// G
					reverseImage[k + 1] = rowBuffer[k + 1];
					// R
					reverseImage[k + 0] = rowBuffer[k + 2];
				}
				reverseImage -= bytesPerRow;
			}

			delete rowBuffer;
		}
		
		// закрываем файл
		fclose(bmpFile);
		
		return true;
	}
		
	return false;
}