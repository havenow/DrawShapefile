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
*   ����һ���ַ�����Ϣ
*   �������ϵ�λ�ã�ƫ�ƾ��룬�Լ�����һ��������
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
    *   �洢��ǰ�ַ��������ϵ�����λ��
    *   ����1024��С���������Դ泣�õĺ���(16����)
    */
    unsigned __int64   x0:10;
    unsigned __int64   y0:10;
    unsigned __int64   x1:10;
    unsigned __int64   y1:10;
    //! �����ƫ��x����
    unsigned __int64   offsetX:8;
    //! �����ƫ��y����
    unsigned __int64   offsetY:8;
    //! ��������У����256��
    unsigned __int64   texIndex:8;
};


class CELLFont
{
public:
    /**
    *   ϵͳ����ʹ�õ�����2D GL_ALPHA��ʽ
    */
    unsigned    _sysFontTexture;
    Character   _character[1<<16];
    
    FT_Library  _library;
    FT_Face     _face; 
    char*       _fontBuffer;
    //! ��������Ŀ��
    int             _textureWidth;
    //! ��������ĸ߶�
    int             _textureHeight;
    //! ��ʱ���������浱ǰ����д�������ϵ�λ������
    int             _yStart;
    int             _xStart;
    //! �����С
    int             _fontSize;
    //! ��������Ĵ�С��������������ش�С��������Ҫ��һ��������������������ش�С
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
        *   ��������Ĵ�С
        */
        _fontSize   =   fontSize;
        /**
        *   �Ѿ�����������������
        *   ֧�ֶ�ε���
        */
        if (_face)
        {
            FT_Done_Face(FT_Face(_face));
            _xStart         =   0;
            _yStart         =   0;
            memset(_character,0,sizeof(_character));
        }
        /**
        *   ��������
        */
        if (_sysFontTexture != -1)
        {
            glDeleteTextures(1,&_sysFontTexture);
        }

        glGenTextures(1,&_sysFontTexture);
        /**
        *   ʹ���������id,���߽а�(����)
        */
        glBindTexture( GL_TEXTURE_2D, _sysFontTexture );
        /**
        *   ָ������ķŴ�,��С�˲���ʹ�����Է�ʽ������ͼƬ�Ŵ��ʱ���ֵ��ʽ 
        */
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

        glTexImage2D( 
            GL_TEXTURE_2D,      //! ָ���Ƕ�άͼƬ
            0,                  //! ָ��Ϊ��һ�������������mipmap,��lod,����ľͲ��ü����ģ�Զ��ʹ�ý�С������
            GL_ALPHA,           //! �����ʹ�õĴ洢��ʽ
            _textureWidth,
            _textureHeight,
            0,                  //! �Ƿ�ı�
            GL_ALPHA,           //! ���ݵĸ�ʽ��bmp�У�windows,����ϵͳ�д洢��������bgr��ʽ
            GL_UNSIGNED_BYTE,   //! ������8bit����
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
                *   д��һ��,���¿�ʼ
                */
                _xStart =   0;
                /**
                *   y��ʼλ��Ҫ����
                */
                _yStart +=  max(_fontPixelY,_fontSize);
            }
            FT_Load_Glyph( _face, FT_Get_Char_Index(_face, ch ), FT_LOAD_DEFAULT );
            FT_Glyph glyph;
            FT_Get_Glyph( FT_Face(_face)->glyph, &glyph );

            /**
            *   ��������Ĵ�С�����Ƿ�ʹ�÷���ݻ���ģʽ
            *   ������Ƚ�С����˵����ʹ��ft_render_mode_mono
            *   ������Ƚϴ�������12���ϣ�����ʹ��ft_render_mode_normalģʽ
            */
            if ( !(ch >= L'0' && ch <=L'9'))
            {
                FT_Glyph_To_Bitmap( &glyph, ft_render_mode_normal, 0, 1 );
            }
            else
            {
                FT_Glyph_To_Bitmap( &glyph, ft_render_mode_normal, 0, 1 );//ʹ��ft_render_mode_mono�����ڴ�й©����
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
            *   ���û�����ݣ���д��ֱ�ӹ�ȥ
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
        *   �󶨳�������
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
            *   ��һ����
            */
            vert[index + 0].x       =   xStart;
            vert[index + 0].y       =   yStart - h + offsetY;
            vert[index + 0].z       =   zStart;
            vert[index + 0].u       =   ch.x0/texWidth;
            vert[index + 0].v       =   ch.y0/texHeight;
            vert[index + 0].w       =   1;
            vert[index + 0].color   =   color;
            /**
            *   �ڶ�����
            */
            vert[index + 1].x       =   xStart + w;
            vert[index + 1].y       =   yStart - h + offsetY;
            vert[index + 1].z       =   zStart;
            vert[index + 1].u       =   ch.x1/texWidth;
            vert[index + 1].v       =   ch.y0/texHeight;
            vert[index + 1].w       =   1;
            vert[index + 1].color   =   color;
            /**
            *   ��������
            */
            vert[index + 2].x       =   xStart + w;
            vert[index + 2].y       =   yStart  + offsetY;
            vert[index + 2].z       =   zStart;
            vert[index + 2].u       =   ch.x1/texWidth;
            vert[index + 2].v       =   ch.y1/texHeight;
            vert[index + 2].w       =   1;
            vert[index + 2].color   =   color;
            /**
            *   ��һ����
            */
            vert[index + 3].x       =   xStart;
            vert[index + 3].y       =   yStart - h + offsetY;
            vert[index + 3].z       =   zStart;
            vert[index + 3].u       =   ch.x0/texWidth;
            vert[index + 3].v       =   ch.y0/texHeight;
            vert[index + 3].w       =   1;
            vert[index + 3].color   =   color;
            /**
            *   ��������
            */
            vert[index + 4].x       =   xStart + w;
            vert[index + 4].y       =   yStart  + offsetY;
            vert[index + 4].z       =   zStart;
            vert[index + 4].u       =   ch.x1/texWidth;
            vert[index + 4].v       =   ch.y1/texHeight;
            vert[index + 4].w       =   1;
            vert[index + 4].color   =   color;
            /**
            *   ���ĸ���
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
