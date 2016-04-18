//
//  IMLibTypes.h
//  Ѕилиотека IM
//
//  определени€ базовых типов бибилиотеки IMLib
//
//  Created by Vladimir Knyaz on 29.05.14.
//  Copyright (c) 2014 Vladimir Knyaz. All rights reserved.
//

#ifndef IMLibTypes_h
#define IMLibTypes_h

struct IMPoint
{
	double x;
	double y;

	IMPoint()
	{
		x = 0;
		y = 0;
	}

	IMPoint(double xx, double yy)
	{
		x = xx;
		y = yy;
	}
};

struct IMSize
{
	double width;
	double height;

	IMSize()
	{
		width = 0;
		height = 0;
	}

	IMSize(double w, double h)
	{
		width = w;
		height = h;
	}
};

struct IMLine
{
	IMPoint start;
	IMPoint end;

	IMLine(IMPoint s, IMPoint e)
	{
		start = s;
		end = e;
	}
};

struct IMRegion
{
	IMPoint origin;
	IMSize size;

	IMRegion()
	{

	}

	IMRegion(double x, double y, double width, double height):
		origin(x, y),
		size(width, height)
	{

	}

	IMRegion(IMPoint o, IMSize s) :
		origin(o),
		size(s)
	{

	}
};

struct IMRunway
{
	IMLine leftBorderMarking;
	IMLine rightBorderMarking;
	IMLine thresholdMarking;
	IMLine touchdownMarking1;
};

extern const char* IMNetGetImageFormat;
extern const char* IMNetGetImage;

#endif
