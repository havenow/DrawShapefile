#pragma once

#include "shp/shapefil.h"

#include "CELLFeature.hpp"
#include "cellfont.hpp"
#include "CELLMercator.hpp"
#include "CELLProj4.hpp"

namespace   CELL
{

    
    class   CELLShpReader
    {
    public:
        ArrayFeature    _features;
        float           _xMax;
        float           _yMax;
        float           _xMin;
        float           _yMin;

		int             _width;
		int             _height;

		int             _drawFeatures;

    public:
		virtual ~CELLShpReader()
		{
			for (std::vector<CELLFeature*>::iterator it = _features.begin(); it != _features.end(); ++it)
			{
				delete *it;
			}
		}

		wchar_t*   a2u(const char* text, size_t length = 0)
		{
			if (length == 0)
			{
				length = strlen(text);
			}
			static  wchar_t sBuffer[2048] = { 0 };
			memset(sBuffer, 0, sizeof(sBuffer));
			::MultiByteToWideChar(CP_ACP, 0, text, length, sBuffer, sizeof(sBuffer) / sizeof(wchar_t));
			return  sBuffer;
		}
		/**
		*   utf-8编码方式转化为unicode
		*/
		wchar_t* utf82u(const char* buf)
		{
			int len = ::MultiByteToWideChar(CP_UTF8, 0, buf, -1, NULL, 0);
			if (len == 0)
				return L"";

			static  wchar_t     unicode[2048] = { 0 };
			memset(unicode, 0, sizeof(unicode));
			::MultiByteToWideChar(CP_UTF8, 0, buf, -1, unicode, len);

			return unicode;
		}

        bool    read(const char* shpName, const char* dbfName)
        {
			DBFHandle   hDBF		=	DBFOpen(dbfName, "rb");
            SHPHandle   hShpFile    =   SHPOpen(shpName, "rb" );
			char*       filedName	=	"NAME";//"PERIMETER";
			int         nField		=	0;

			for (int i = 0; i < DBFGetFieldCount(hDBF); i++)
			{
				DBFFieldType	eType;
				char            szTitle[32];
				int             nWidth;
				int             nDecimals;

				eType = DBFGetFieldInfo(hDBF, i, szTitle, &nWidth, &nDecimals);
				if (strcmp(szTitle, filedName) == 0)
				{
					nField = i;
				}
			}

            if (hShpFile == 0)
            {
                return  false;
            }
            /**
            *   写入图层数据到数据库中
            */
			CELLProj4 proj4;
			double2 uv = proj4.lonLatToMeters(hShpFile->adBoundsMax[0], hShpFile->adBoundsMax[1]);
			_xMax = uv.x;
			_yMax = uv.y;
			uv = proj4.lonLatToMeters(hShpFile->adBoundsMin[0], hShpFile->adBoundsMin[1]);
			_xMin = uv.x;
			_yMin = uv.y;
            
            for(int i = 0; i < hShpFile->nRecords; i++ )
            {
                SHPObject*  psShape =   SHPReadObject(hShpFile, i);
                CELLFeature*feature =   new CELLFeature();

				uv = proj4.lonLatToMeters(psShape->dfXMin, psShape->dfYMin);
				feature->_minX = uv.x;
				feature->_minY = uv.y;
				uv = proj4.lonLatToMeters(psShape->dfXMax, psShape->dfYMax);
				feature->_maxX = uv.x;
				feature->_maxY = uv.y;

				char*   fields = "NULL";
				if (nField != -1)
				{
					fields = (char*)DBFReadStringAttribute(hDBF, i, nField);
				}
				wchar_t*    text = a2u(fields);
				wcscpy(feature->_text, text);

                feature->reserve(psShape->nVertices);

                Primative   pri;
                if (psShape->nParts == 0)
                {
                    pri._count  =   psShape->nVertices;
                    pri._start  =   0;
                    pri._type   =   GL_LINE_STRIP;
                    feature->_pris.push_back(pri);
                }
                for (int x = 0 ;x < psShape->nParts ; ++ x)
                {
                    pri._start      =   psShape->panPartStart[x];
                    //! 最后一个
                    if ( x == psShape->nParts - 1)
                    {
                        pri._count   =   psShape->nVertices - psShape->panPartStart[x];
                    }
                    else
                    {
                        pri._count   =   psShape->panPartStart[x + 1] - psShape->panPartStart[x];
                    }
                    switch (psShape->panPartType[x])
                    {
                    case SHPT_POINT:
                        {
                            pri._type   =   GL_LINE_STRIP;
                        }
                        break;
                    case SHPT_ARC:
                        {
                            pri._type   =   GL_LINE_STRIP;
                        }
                        break;
                    case SHPT_POLYGON:
                        {
                            pri._type   =   GL_LINE_STRIP;
                        }
                        break;
                    default:
                        pri._type       =   GL_LINE_STRIP;
                    }
                    feature->_pris.push_back(pri);
                }

                for ( int j = 0 ; j < psShape->nVertices ; ++ j )
                {
					double2 xy = proj4.lonLatToMeters(psShape->padfX[j], psShape->padfY[j]);
                    float   x = (float)xy.x;
                    float   y = (float)xy.y;
                    feature->push_back(float2(x,y));
                }
                SHPDestroyObject(psShape);
                _features.push_back(feature);
            }
			DBFClose(hDBF);
            SHPClose(hShpFile);
        }


        void    render(PROGRAM_P2_C4& shader)
        {
			_drawFeatures = 0;
			AABB2D  aabbWorld;
			aabbWorld.setExtents(_xMin, _yMin, _xMax, _yMax);
            glUniform4f(shader._color, 1,0,0,1);

            for (size_t i = 0 ;i < _features.size() ; ++ i)
            {
                CELLFeature*    feature =   _features[i];

				AABB2D  aabbFeature;
				aabbFeature.setExtents(feature->_minX, feature->_minY, feature->_maxX, feature->_maxY);
				if (!aabbFeature.intersects(aabbWorld))
				{
					feature->_drawFlag = 0;
					continue;
				}
				feature->_drawFlag = 1;
				++_drawFeatures;

                float2* vertex  =   (float2*)&feature->front();

                
                
                glVertexAttribPointer(shader._position,2,GL_FLOAT,  false,  sizeof(float2),vertex);

                for (size_t j = 0 ; j < feature->_pris.size(); ++ j)
                {
                    Primative   pri =   feature->_pris[j];
                    glDrawArrays(pri._type,pri._start,pri._count);
                }
            }
        }

		/**
		*   绘制文字
		*/
		void    renderText(CELLFont& font)
		{
			for (size_t i = 0; i < _features.size(); ++i)
			{
				CELLFeature*    feature = _features[i];
				if (feature->_drawFlag != 1)
				{
					continue;
				}

				float           centerX = (feature->_minX + feature->_maxX) * 0.5f;
				float           centerY = (feature->_minY + feature->_maxY) * 0.5f;

				float2          screen = longLatToScreen(float2(centerX, centerY));

				font.drawText(screen.x, screen.y, 0, Rgba4Byte(), feature->_text, wcslen(feature->_text));
			}

			wchar_t     text[128];
			swprintf(text, L"feature draw %d", _drawFeatures);
			font.drawText(100, 100, 0, Rgba4Byte(255, 0, 0), text, wcslen(text));
		}

		/**
		*   坐标转化
		*/
		float2 longLatToScreen(const float2& longLat)
		{

			float   longSize = _xMax - _xMin;
			float   latSize = _yMax - _yMin;
			float   xPixel = longSize / float(_width);
			float   yPixel = latSize / float(_height);

			return  float2((longLat.x - _xMin) / xPixel, (_yMax - longLat.y) / yPixel);
		}

		void    setOrth(double left, double right, double top, double bottom)
		{
			/**
			*   这里要计算关于经度纬度与屏幕坐标的比例
			*   保证比例，不变形
			*/
			double    width = abs(right - left);
			double    height = abs(top - bottom);

			double    fXScale = width / real(_width);
			double    fYScale = height / real(_height);
			if (fXScale > fYScale)
			{
				height = fXScale * _height;
			}
			else
			{
				width = fYScale * _width;
			}
			double    centerLong = (left + right) * 0.5f;
			double    centerLat = (bottom + top) * 0.5f;

			/**
			*   根据新的宽度与高度计算出来bottom 与top
			*/
			_xMin = centerLong - width * 0.5f;
			_xMax = centerLong + width * 0.5f;
			_yMax = centerLat + height * 0.5f;
			_yMin = centerLat - height * 0.5f;
		}

		void    resize()
		{
			setOrth(_xMin, _xMax, _yMax, _yMin);
		}
    };

}