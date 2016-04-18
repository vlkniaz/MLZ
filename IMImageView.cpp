/*
*  IMImageView.h
*  Билиотека IMLib
*
*  IMImageView - класс отрисовки изображения
*
*  Created by Владимир Князь on 27.01.10.
*  Copyright 2015 GosNIIAS. All rights reserved.
*
*/

#include "IMImageView.h"
#include <iostream>

// конструктор без параметров
IMImageView::IMImageView() : m_image(0), m_textureName(0)
{
	m_textureCoords[0][0] = 0.0f;
	m_textureCoords[0][1] = 1.0f;

	m_textureCoords[1][0] = 0.0f;
	m_textureCoords[1][1] = 0.0f;

	m_textureCoords[2][0] = 1.0f;
	m_textureCoords[2][1] = 0.0f;

	m_textureCoords[3][0] = 1.0f;
	m_textureCoords[3][1] = 1.0f;
}

// иницализация текстур
void IMImageView::initTextures()
{
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &m_textureName);
	glBindTexture(GL_TEXTURE_2D, m_textureName);

	glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
	glPixelStorei(GL_UNPACK_LSB_FIRST, GL_TRUE);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

// установка изображения
void IMImageView::setImage(IMImageFile* image)
{
	if (image != 0)
	{
		if (m_textureName == 0)
		{
			initTextures();
		}

		m_image = image;
		unsigned char *pixelData = m_image->image();
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_textureName);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_image->pixelsWide(), m_image->pixelsHigh(), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pixelData);
	}
}

void IMImageView::setPixelRegion(IMRegion region)
{
	IMView::setPixelRegion(region);
}

void IMImageView::setRegion(IMRegion region)
{
	IMView::setRegion(region);	

	m_vertices[0][0] = m_region.origin.x;
	m_vertices[0][1] = m_region.origin.y;

	m_vertices[1][0] = m_region.origin.x;
	m_vertices[1][1] = m_region.origin.y + m_region.size.height;

	m_vertices[2][0] = m_region.origin.x + m_region.size.width;
	m_vertices[2][1] = m_region.origin.y + m_region.size.height;

	m_vertices[3][0] = m_region.origin.x + m_region.size.width;
	m_vertices[3][1] = m_region.origin.y;
}

// функция отрисовки
void IMImageView::draw()
{
	//cout << "IMImageView::draw()" << endl;

	if (m_image != 0)
	{
		glEnable(GL_TEXTURE_2D);

		// настраиваем текстуры
		glBindTexture(GL_TEXTURE_2D, m_textureName);

		glVertexPointer(2, GL_FLOAT, 0, m_vertices);
		glTexCoordPointer(2, GL_FLOAT, 0, m_textureCoords);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glDrawArrays(GL_POLYGON, 0, 4);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
}