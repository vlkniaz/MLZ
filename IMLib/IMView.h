/*
*  IMView.h
*  Билиотека IMLib
*
*  IMView - класс отрисовки заданной области в контексте OpenGL
*
*  Created by Владимир Князь on 27.01.10.
*  Copyright 2015 GosNIIAS. All rights reserved.
*
*/

#pragma once

#include <IMLib/IMLibTypes.h>
#include <vector>

using namespace std;

class IMView
{
protected:
	// вложенные области
	vector<IMView*> m_subviews;

	// координаты области отрисовки в системе координат родительской области
	IMRegion m_region;

	// координаты области отрисовки в пикселах в контексте OpenGL
	IMRegion m_pixelRegion;

	// определяет виден ли данный вид
	bool m_isVisible;

	// родительский вид
	IMView *m_parentView;

public:
	// конструктор без параметров
	IMView() : m_isVisible(true), m_parentView(0)
	{

	}

	// конструктор без параметров
	IMView(IMView* parentView) : m_isVisible(true), m_parentView(parentView)
	{

	}

	// функция отрисовки
	virtual void draw();

	IMRegion region()
	{
		return m_region;
	}

	virtual void setRegion(IMRegion region)
	{
		m_region = region;
	}

	virtual void setPixelRegion(IMRegion pixelRegion);

	void addSubview(IMView *subview)
	{
		if (subview != 0)
		{
			m_subviews.push_back(subview);
			subview->setParentView(this);
		}
	}

	vector<IMView*> subviews()
	{
		return m_subviews;
	}

	bool isVisible()
	{
		return m_isVisible;
	}

	void setIsVisible(bool isVisible)
	{
		m_isVisible = isVisible;
	}

	void setParentView(IMView* parent)
	{
		m_parentView = parent;
	}
};