/*
*  IMImageConverter.h
*  Билиотека IM
*
*  IMImageConverter - класс для изменения глубины цвета изображений
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
    // пороговый дизеринг
    static const int DITHERING_THRESHOLD;
    // дизеринг со случайным порогом
    static const int DITHERING_RANDOM;
    // упорядовченный дизеринг
    static const int DITHERING_ORDERD;
    // дизеринг с диффузией ошибки вперёд
    static const int DITHERING_DIFFUSION_FORWARD;
    // дизеринг с диффузией ошибки по алгоритму Флойда-Штейнберга
    static const int DITHERING_FLOYD;
    // дизеринг с диффузией ошибки вперёд и назаж
    static const int DITHERING_DIFFUSION_FORWARD_BACKWARD;
    
    
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
