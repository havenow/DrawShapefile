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
        ArrayPrimative  _pris;
    };

    typedef std::vector<CELLFeature*>   ArrayFeature;
}