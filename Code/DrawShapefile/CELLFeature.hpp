#pragma once

#include "CELLMath.hpp"
#include <vector>

namespace   CELL
{
    struct  Primative
    {
        int     _start;
        int     _count;
        int     _type;
    };

    typedef std::vector<Primative>      ArrayPrimative;
    class   CELLFeature :public std::vector<float2>
    {
    public:
		CELLFeature()
		{
			memset(_text, 0, sizeof(_text));
			_minX = 0;
			_minY = 0;
			_maxX = 0;
			_maxY = 0;
		}
		ArrayPrimative  _pris;
		wchar_t         _text[32];
		float           _minX;
		float           _minY;
		float           _maxX;
		float           _maxY;
		int             _drawFlag;
    };

    typedef std::vector<CELLFeature*>   ArrayFeature;
}