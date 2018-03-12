#pragma once

#include "freetype/ftglyph.h"
#include FT_GLYPH_H
#include FT_TRUETYPE_TABLES_H
#include FT_BITMAP_H
#include FT_WINFONTS_H

#include "CELLMath.hpp"
#include "CELLShader.hpp"
using namespace CELL;
#include <assert.h>


struct  CELLUIVertex
{
    float       x,y,z;
    float       u,v,w;
    Rgba4Byte   color;
};

/**
*   描述一个字符的信息
*   在纹理上的位置，偏移距离，以及在哪一个纹理上
*/
class    Character
{
public:
    Character()
    {
        x0          =   0;
        y0          =   0;
        x1          =   0;
        y1          =   0;
        offsetX     =   0;
        offsetY     =   0;
    }
    /**
    *   存储当前字符在纹理上的坐标位置
    *   采用1024大小的纹理，可以存常用的汉字(16像素)
    */
    unsigned __int64   x0:10;
    unsigned __int64   y0:10;
    unsigned __int64   x1:10;
    unsigned __int64   y1:10;
    //! 字体的偏移x方向
    unsigned __int64   offsetX:8;
    //! 字体的偏移y方向
    unsigned __int64   offsetY:8;
    //! 纹理的所有，最大256个
    unsigned __int64   texIndex:8;
};


class CELLFont
{
public:
    /**
    *   系统字体使用的纹理2D GL_ALPHA格式
    */
    unsigned    _sysFontTexture;
    Character   _character[1<<16];
    
    FT_Library  _library;
    FT_Face     _face; 
    char*       _fontBuffer;
    //! 字体文理的宽度
    int             _textureWidth;
    //! 字体文理的高度
    int             _textureHeight;
    //! 临时变量，保存当前字体写到文理上的位置坐标
    int             _yStart;
    int             _xStart;
    //! 字体大小
    int             _fontSize;
    //! 上述字体的大小并不是字体的像素大小，所以需要用一个变量来计算字体的像素大小
    int             _fontPixelX;
    int             _fontPixelY;
    CELLUIProgram   _uiProgram;
public:

    CELLFont(void)
    {
        memset(_character,0,sizeof(_character));
        _fontPixelX     =   0;
        _fontPixelY     =   0;
        _yStart         =   0;
        _xStart         =   0;
        _textureWidth   =   1024;
        _textureHeight  =   1024;
        _fontBuffer     =   0;
        _library        =   0;
        _face           =   0;

        FT_Init_FreeType( &_library );

    }

    virtual ~CELLFont(void)
    {
		if (_fontBuffer)
		{
			delete[] _fontBuffer;
			_fontBuffer = nullptr;
		}
		//  free FreeType Lib 
		if (_library)
			FT_Done_FreeType(_library);
    }
    char*	readFile(const char* fileName,unsigned& length)
    {
        FILE*   pFile  =   fopen(fileName,"rb");
        if (pFile)
        {
            fseek( pFile, 0, SEEK_END );
            length = ftell( pFile );
            fseek(pFile, 0, SEEK_SET );
            char* buffer =   new char[length + 1];
            fread( buffer, 1, length, pFile );
            buffer[length] = 0;
            fclose( pFile );
            return  buffer;
        }
        else
        {
            char    buffer[1024];
            sprintf_s(buffer,"read %s  failed",fileName);
            assert(pFile != 0 && buffer);
        }
        return  0;

    }
    void    buildSystemFont(const char* font,int fontSize)
    {
        _uiProgram.initialize();

        unsigned    length  =   0;
        _fontBuffer  =   readFile(font,length);

        /**
        *   保存字体的大小
        */
        _fontSize   =   fontSize;
        /**
        *   已经创建了字体则销毁
        *   支持多次调用
        */
        if (_face)
        {
            FT_Done_Face(FT_Face(_face));
            _xStart         =   0;
            _yStart         =   0;
            memset(_character,0,sizeof(_character));
        }
        /**
        *   销毁字体
        */
        if (_sysFontTexture != -1)
        {
            glDeleteTextures(1,&_sysFontTexture);
        }

        glGenTextures(1,&_sysFontTexture);
        /**
        *   使用这个纹理id,或者叫绑定(关联)
        */
        glBindTexture( GL_TEXTURE_2D, _sysFontTexture );
        /**
        *   指定纹理的放大,缩小滤波，使用线性方式，即当图片放大的时候插值方式 
        */
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

        glTexImage2D( 
            GL_TEXTURE_2D,      //! 指定是二维图片
            0,                  //! 指定为第一级别，纹理可以做mipmap,即lod,离近的就采用级别大的，远则使用较小的纹理
            GL_ALPHA,           //! 纹理的使用的存储格式
            _textureWidth,
            _textureHeight,
            0,                  //! 是否的边
            GL_ALPHA,           //! 数据的格式，bmp中，windows,操作系统中存储的数据是bgr格式
            GL_UNSIGNED_BYTE,   //! 数据是8bit数据
            0
            );

        FT_Error    error   =   FT_New_Memory_Face((FT_Library)_library,(const FT_Byte *)_fontBuffer,length,0,(FT_Face*)&_face);
        if ( error != 0 ) 
        {
            return;
        }
        FT_Face     ftFace  =   (FT_Face)_face;
        
        FT_Select_Charmap( ftFace, FT_ENCODING_UNICODE );
        FT_F26Dot6 ftSize = (FT_F26Dot6)(fontSize * (1 << 6));

        FT_Set_Char_Size((FT_Face)_face, ftSize, 0, 72, 72);

        assert(_face != 0 );
    }

    Character   getCharacter( wchar_t ch )
    {

        if (_character[ch].x0 == 0 &&
            _character[ch].x0 == 0 &&
            _character[ch].x1 == 0 &&
            _character[ch].y1 == 0
            )
        {

            glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

            if (_xStart + max(_fontPixelX,_fontSize) > _textureWidth)
            {
                /**
                *   写满一行,从新开始
                */
                _xStart =   0;
                /**
                *   y开始位置要增加
                */
                _yStart +=  max(_fontPixelY,_fontSize);
            }
            FT_Load_Glyph( _face, FT_Get_Char_Index(_face, ch ), FT_LOAD_DEFAULT );
            FT_Glyph glyph;
            FT_Get_Glyph( FT_Face(_face)->glyph, &glyph );

            /**
            *   根据字体的大小决定是否使用反锯齿绘制模式
            *   当字体比较小的是说建议使用ft_render_mode_mono
            *   当字体比较大的情况下12以上，建议使用ft_render_mode_normal模式
            */
            if ( !(ch >= L'0' && ch <=L'9'))
            {
                FT_Glyph_To_Bitmap( &glyph, ft_render_mode_normal, 0, 1 );
            }
            else
            {
                FT_Glyph_To_Bitmap( &glyph, ft_render_mode_normal, 0, 1 );//使用ft_render_mode_mono会有内存泄漏？？
            }
            
            FT_BitmapGlyph  bitmap_glyph    =   (FT_BitmapGlyph)glyph;
            FT_Bitmap&      bitmap          =   bitmap_glyph->bitmap;
			FT_Bitmap       ftBitmap;
			FT_Bitmap_New(&ftBitmap);

            if(bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
            {
                if (FT_Bitmap_Convert((FT_Library)_library, &bitmap, &ftBitmap, 1) == 0)
                {
                    /**
                    *   Go through the bitmap and convert all of the nonzero values to 0xFF (white).
                    */
                    for (unsigned char* p = ftBitmap.buffer, * endP = p + ftBitmap.width * ftBitmap.rows; p != endP; ++p)
                        *p ^= -*p ^ *p;
                    bitmap  =   ftBitmap;
                }
            }
            /**
            *   如果没有数据，则不写，直接过去
            */
            if (bitmap.width == 0 || bitmap.rows == 0)
            {
                
                char    mem[1024 * 32];
                memset(mem,0,sizeof(mem));

                _character[ch].x0       =   _xStart;
                _character[ch].y0       =   _yStart;
                _character[ch].x1       =   _xStart + _fontSize/2;
                _character[ch].y1       =   _yStart + _fontSize - 1;
                _character[ch].offsetY  =   _fontSize - 1;
                _character[ch].offsetX  =   0;

                

                glBindTexture(GL_TEXTURE_2D,_sysFontTexture);

                glTexSubImage2D (
                    GL_TEXTURE_2D,
                    0,
                    _xStart,
                    _yStart,
                    _fontSize/2,
                    _fontSize,
                    GL_ALPHA,
                    GL_UNSIGNED_BYTE,
                    mem
                    );
                _xStart +=  _fontSize/2;


            }
            else
            {
                glBindTexture(GL_TEXTURE_2D,_sysFontTexture);

                _character[ch].x0       =   _xStart;
                _character[ch].y0       =   _yStart;
                _character[ch].x1       =   _xStart + bitmap.width;
                _character[ch].y1       =   _yStart + bitmap.rows;

                _character[ch].offsetY  =   bitmap_glyph->top;
                _character[ch].offsetX  =   bitmap_glyph->left;

                glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
                glTexSubImage2D (
                    GL_TEXTURE_2D,
                    0,
                    _xStart,
                    _yStart,
                    max(1,bitmap.width),
                    max(1,bitmap.rows),
                    GL_ALPHA,
                    GL_UNSIGNED_BYTE,
                    bitmap.buffer
                    );
                _xStart     +=  (bitmap.width + 1);
                _fontPixelY =   max(_fontPixelY,bitmap.rows);
                _fontPixelX =   max(_fontPixelX,bitmap.width);
            }
			FT_Done_Glyph(glyph);
           //FT_Bitmap_Done((FT_Library)_library,&ftBitmap);
        }
        return  _character[ch];
    }

    void beginText( const matrix4r& proj )
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);
        glBindTexture( GL_TEXTURE_2D, _sysFontTexture );

        _uiProgram.begin();
        /**
        *   绑定程序数据
        */
        glUniformMatrix4fv(_uiProgram.getMVPUniform(),1,false,proj.data());
        glUniform1i(_uiProgram.getTexture1Uniform(),0);
    }

    void endText()
    {
        _uiProgram.end();
    }

    float2 drawText( 
        float x,
        float y, 
        float z, 
        Rgba4Byte color,
        const wchar_t* text, 
        size_t length
        )
    {
        static      CELLUIVertex  vert[1024];

        float       texWidth    =   (float)_textureWidth;
        float       texHeight   =   (float)_textureHeight;
        float       xStart      =   (float)(int)x;
        float       yStart      =   (float)(int)y + _fontSize;
        float       zStart      =   z;
        unsigned    index       =   0;
        unsigned    size        =   length == -1 ? wcslen(text) :length;
        float2      vSize(0,0);


        if (size == 0)
        {
            return  vSize;
        }
        for (unsigned i = 0 ;i <  size; ++ i )
        {
			Character   ch  = getCharacter(text[i]);

            float       h           =   float(ch.y1 - ch.y0);
            float       w           =   float(ch.x1 - ch.x0);
            float       offsetY     =   (float(h) - float(ch.offsetY));
            /**
            *   第一个点
            */
            vert[index + 0].x       =   xStart;
            vert[index + 0].y       =   yStart - h + offsetY;
            vert[index + 0].z       =   zStart;
            vert[index + 0].u       =   ch.x0/texWidth;
            vert[index + 0].v       =   ch.y0/texHeight;
            vert[index + 0].w       =   1;
            vert[index + 0].color   =   color;
            /**
            *   第二个点
            */
            vert[index + 1].x       =   xStart + w;
            vert[index + 1].y       =   yStart - h + offsetY;
            vert[index + 1].z       =   zStart;
            vert[index + 1].u       =   ch.x1/texWidth;
            vert[index + 1].v       =   ch.y0/texHeight;
            vert[index + 1].w       =   1;
            vert[index + 1].color   =   color;
            /**
            *   第三个点
            */
            vert[index + 2].x       =   xStart + w;
            vert[index + 2].y       =   yStart  + offsetY;
            vert[index + 2].z       =   zStart;
            vert[index + 2].u       =   ch.x1/texWidth;
            vert[index + 2].v       =   ch.y1/texHeight;
            vert[index + 2].w       =   1;
            vert[index + 2].color   =   color;
            /**
            *   第一个点
            */
            vert[index + 3].x       =   xStart;
            vert[index + 3].y       =   yStart - h + offsetY;
            vert[index + 3].z       =   zStart;
            vert[index + 3].u       =   ch.x0/texWidth;
            vert[index + 3].v       =   ch.y0/texHeight;
            vert[index + 3].w       =   1;
            vert[index + 3].color   =   color;
            /**
            *   第三个点
            */
            vert[index + 4].x       =   xStart + w;
            vert[index + 4].y       =   yStart  + offsetY;
            vert[index + 4].z       =   zStart;
            vert[index + 4].u       =   ch.x1/texWidth;
            vert[index + 4].v       =   ch.y1/texHeight;
            vert[index + 4].w       =   1;
            vert[index + 4].color   =   color;
            /**
            *   第四个点
            */
            vert[index + 5].x       =   xStart;
            vert[index + 5].y       =   yStart  + offsetY;
            vert[index + 5].z       =   zStart;
            vert[index + 5].u       =   ch.x0/texWidth;
            vert[index + 5].v       =   ch.y1/texHeight;
            vert[index + 5].w       =   1;
            vert[index + 5].color   =   color;

            index   +=  6;
            xStart  +=  w + (ch.offsetX + 1);

            vSize.x +=  w + (ch.offsetX + 1);
            vSize.y =   max(h + offsetY,vSize.y);

        }
        glVertexAttribPointer(_uiProgram.getPositionAttribute(),   3,  GL_FLOAT,           GL_FALSE,   sizeof(CELLUIVertex),   vert);
        glVertexAttribPointer(_uiProgram.getUV1Attribute(),        3,  GL_FLOAT,           GL_FALSE,   sizeof(CELLUIVertex),   &vert[0].u);
        glVertexAttribPointer(_uiProgram.getColorAttribute(),      4,  GL_UNSIGNED_BYTE,   GL_TRUE,    sizeof(CELLUIVertex),   &vert[0].color);
        glDrawArrays(GL_TRIANGLES,0,index);
       
        return  vSize;
    }


};
