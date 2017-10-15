/*
*  IMImageConverter.h
*  Билиотека IM
*
*  IMImageResizer - класс для изменения размеров и обрезки изображений
*
*  Created by Владимир Князь on 27.01.10.
*  Copyright 2010 __MyCompanyName__. All rights reserved.
*
*/

#pragma once

#include <IMLib/IMLibTypes.h>
#include <IMLib/IMImageFile.h>

class IMImageConverter
{
private:
	// исходное изображение
	IMImageFile *m_image;

public:
    static const int DITHERING_THRESHOLD;
    static const int DITHERING_RANDOM;
    
    // создаёт новое изображение с помощью дизеринга
    IMImageFile* newImageWithDithering(int mode = 1, int threshold = 128);
    
	void setImage(IMImageFile *image)
	{
		m_image = image;
	}
    
	IMImageFile *image()
	{
		return m_image;
	}
};
