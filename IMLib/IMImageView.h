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

#pragma once

#include <IMLib/IMLibTypes.h>
#include <IMLib/IMImageFile.h>
#include <IMLib/IMView.h>
#include <vector>

#include <Windows.h>
#include <gl/GL.h>

using namespace std;

class IMImageView : public IMView
{
private:
	// изображение для отрисовки
	IMImageFile *m_image;

	// индентификатор текстуры для отрисовки изображения
	GLuint m_textureName;

	GLfloat m_vertices[4][2];
	GLfloat m_textureCoords[4][2];

public:
	static const int IM_FIT_MODE = 0;

private:
	// иницализация текстур
	void initTextures();

public:
	// конструктор без параметров
	IMImageView();

	// функция отрисовки
	virtual void draw();

	virtual void setRegion(IMRegion region);

	virtual void setPixelRegion(IMRegion region);

	// установка изображения
	void setImage(IMImageFile* image);
};