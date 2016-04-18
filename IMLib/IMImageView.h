/*
*  IMImageView.h
*  ��������� IMLib
*
*  IMImageView - ����� ��������� �����������
*
*  Created by �������� ����� on 27.01.10.
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
	// ����������� ��� ���������
	IMImageFile *m_image;

	// �������������� �������� ��� ��������� �����������
	GLuint m_textureName;

	GLfloat m_vertices[4][2];
	GLfloat m_textureCoords[4][2];

public:
	static const int IM_FIT_MODE = 0;

private:
	// ������������ �������
	void initTextures();

public:
	// ����������� ��� ����������
	IMImageView();

	// ������� ���������
	virtual void draw();

	virtual void setRegion(IMRegion region);

	virtual void setPixelRegion(IMRegion region);

	// ��������� �����������
	void setImage(IMImageFile* image);
};