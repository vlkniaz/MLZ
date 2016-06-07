//
//  IMLibTypes.h
//  Билиотека IM
//
//  определения базовых типов бибилиотеки IMLib
//
//  Created by Vladimir Knyaz on 29.05.14.
//  Copyright (c) 2014 Vladimir Knyaz. All rights reserved.
//

#ifndef IMLibTypes_h
#define IMLibTypes_h

#include <iostream>
#include <string>
#include <vector>

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
    
    // оператор вывода векотра в поток
    friend std::ostream& operator<< (std::ostream &out, IMPoint &point)
    {
        out << "{" << "\"x\":\"" << point.x
        << "\",\"y\":\"" << point.y
        << "\"}";
        
        return out;
    }
    
    friend std::istream& operator>> (std::istream &in, IMPoint &point)
    {
        std::string str;
        // {
        std::getline(in, str, '"');
        // x
        std::getline(in, str, '"');
        // :
        std::getline(in, str, '"');
        // x
        std::getline(in, str, '"');
        point.x = std::stod(str);
        // ,
        std::getline(in, str, '"');
        // y
        std::getline(in, str, '"');
        // :
        std::getline(in, str, '"');
        // y
        std::getline(in, str, '"');
        point.y = std::stod(str);
        // }
        std::getline(in, str, '}');
        
        return in;
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
    
    // оператор вывода векотра в поток
    friend std::ostream& operator<< (std::ostream &out, IMSize &size)
    {
        out << "{" << "\"width\":\"" << size.width
        << "\",\"height\":\"" << size.height
        << "\"}";
        
        return out;
    }
    
    friend std::istream& operator>> (std::istream &in, IMSize &size)
    {
        std::string str;
        // {
        std::getline(in, str, '"');
        // width
        std::getline(in, str, '"');
        // :
        std::getline(in, str, '"');
        // width
        std::getline(in, str, '"');
        size.width = std::stod(str);
        // ,
        std::getline(in, str, '"');
        // height
        std::getline(in, str, '"');
        // :
        std::getline(in, str, '"');
        // height
        std::getline(in, str, '"');
        size.height = std::stod(str);
        // }
        std::getline(in, str, '}');
        
        return in;
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
    
    // оператор вывода векотра в поток
    friend std::ostream& operator<< (std::ostream &out, IMRegion &region)
    {
        out << "{" << "\"origin\":" << region.origin
        << ",\"size\":" << region.size
        << "}";
        
        return out;
    }
    
    // оператор чтения из потока
    friend std::istream& operator>> (std::istream &in, IMRegion &region)
    {
        std::string str;
        // {
        std::getline(in, str, '"');
        // origin
        std::getline(in, str, '"');
        // :
        std::getline(in, str, ':');
        // origin
        in >> region.origin;
        // ,
        std::getline(in, str, '"');
        // region
        std::getline(in, str, '"');
        // :
        std::getline(in, str, ':');
        // size
        in >> region.size;
        
        return in;
    }
};

//
struct IMMarkupObject
{
    int id;
    int objectClass;
    IMRegion region;
    
    // оператор вывода векотра в поток
    friend std::ostream& operator<< (std::ostream &out, IMMarkupObject &object)
    {
        out << "{" << "\"id\":\"" << object.id
        << "\",\"objectClass\":\"" << object.objectClass
        << "\",\"region\":" << object.region
        << "}";
        
        return out;
    }
    
    // оператор чтения из потока
    friend std::istream& operator>> (std::istream &in, IMMarkupObject &object)
    {
        std::string str;
        // {
        std::getline(in, str, '"');
        if(str.find("{") == std::string::npos)
        {
            object.id = -1;
            return in;
        }
        // id
        std::getline(in, str, '"');
        // :
        std::getline(in, str, '"');
        // id
        std::getline(in, str, '"');
        object.id = std::stoi(str);
        // ,
        std::getline(in, str, '"');
        // objectClass
        std::getline(in, str, '"');
        // :
        std::getline(in, str, '"');
        // objectClass
        std::getline(in, str, '"');
        object.objectClass = std::stoi(str);
        // ,
        std::getline(in, str, '"');
        // region
        std::getline(in, str, '"');
        // :
        std::getline(in, str, ':');
        // region
        in >> object.region;
        
        return in;
    }
};

struct IMImageMarkup
{
    std::vector<IMMarkupObject> objects;
    bool isValid;
    
    // оператор вывода IMImageMarkup в поток
    friend std::ostream& operator<< (std::ostream &out, IMImageMarkup &markup)
    {
        out << "[";
        
        std::vector<IMMarkupObject>::iterator iter;
        for(iter = markup.objects.begin(); iter != markup.objects.end(); iter++)
        {
            out << *iter;
            if(iter+1 != markup.objects.end())
            {
                out << ",";
            }
        }
        
        out << "]";
        
        return out;
    }
    
    // оператор чтения из потока
    friend std::istream& operator>> (std::istream &in, IMImageMarkup &markup)
    {
        std::string str;
        // [
        std::getline(in, str, '[');
        char comma = 0;
        IMMarkupObject object;
        do
        {
            in >> object;
            if(object.id != -1)
            {
                markup.objects.push_back(object);
            }
            comma = 0;
            while(comma != ',' && !in.eof())
            {
                in >> comma;
                if(comma == ']')
                {
                    break;
                }
            }
        } while(comma != ']');
        
        markup.isValid = true;
        return in;
    }
};

struct IMImageSeqeunceMarkup
{
    std::vector<IMImageMarkup> markup;
    
    // оператор вывода IMImageMarkup в поток
    friend std::ostream& operator<< (std::ostream &out, IMImageSeqeunceMarkup &seqeunceMarkup)
    {
        out << "[";
        
        std::vector<IMImageMarkup>::iterator iter;
        for(iter = seqeunceMarkup.markup.begin(); iter != seqeunceMarkup.markup.end(); iter++)
        {
            out << *iter;
            if(iter+1 != seqeunceMarkup.markup.end())
            {
                out << ",";
            }
        }
        
        out << "]";
        
        return out;
    }
    
    // оператор чтения из потока
    friend std::istream& operator>> (std::istream &in, IMImageSeqeunceMarkup &seqeunceMarkup)
    {
        seqeunceMarkup.markup.clear();
        
        std::string str;
        // [
        char start = 0;
        while(start != '[' && !in.eof())
        {
            in >> start;
        }
        char comma = 0;
        do
        {
            IMImageMarkup markup;
            in >> markup;
            if(markup.isValid)
            {
                seqeunceMarkup.markup.push_back(markup);
            }
            comma = 0;
            while(comma != ',' && !in.eof())
            {
                in >> comma;
                if(comma == ']')
                {
                    break;
                }
            }
        } while(comma != ']');
        
        return in;
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
