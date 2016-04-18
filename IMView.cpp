/*
*  IMView.cpp
*  ��������� IMLib
*
*  IMView - ����� ��������� �������� ������� � ��������� OpenGL
*
*  Created by �������� ����� on 27.01.10.
*  Copyright 2015 __MyCompanyName__. All rights reserved.
*
*/

#include "IMView.h"
#include <iostream>
#include <Windows.h>
#include <gl/GL.h>

// ������� ���������
void IMView::draw()
{
	//cout << "IMView::draw()" << endl;

	// ������������ ��� ����������
	vector<IMView*>::iterator iter;
	for (iter = m_subviews.begin(); iter != m_subviews.end(); iter++)
	{
		if ((*iter)->isVisible())
		{
			(*iter)->draw();
		}
	}
}

void IMView::setPixelRegion(IMRegion pixelRegion)
{
	m_pixelRegion = pixelRegion;

	// ������ ���������� ���� �����������
	vector<IMView*>::iterator iter;
	for (iter = m_subviews.begin(); iter != m_subviews.end(); iter++)
	{
		IMRegion region = (*iter)->region();
		region.origin.x *= m_pixelRegion.size.width;
		region.origin.y *= m_pixelRegion.size.height;

		region.size.width *= m_pixelRegion.size.width;
		region.size.height *= m_pixelRegion.size.height;
		
		(*iter)->setPixelRegion(region);
	}
}