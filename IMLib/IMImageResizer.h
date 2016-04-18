/*
*  IMImageFile.h
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

class IMImageResizer
{
private:
	// исходное изображение
	IMImageFile *m_image;

protected:
	// создаёт новое изображение из заданной области исходного изображения
	IMImageFile* newImageFromRegionWithPadding(IMRegion region);

public:
	// создаёт новое изображение из заданной области исходного изображения
	IMImageFile* newImageFromRegion(IMRegion region);

	void setImage(IMImageFile *image)
	{
		m_image = image;
	}
	IMImageFile *image()
	{
		return m_image;
	}
};