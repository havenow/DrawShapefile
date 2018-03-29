#pragma once

#include    <gles2/gl2.h>
namespace   CELL
{


    class CELLFrameBuffer
    {
    public:
        unsigned    _width;
        unsigned    _height;
        unsigned    _FBOID;
        unsigned    _RBOID;
    public:
        CELLFrameBuffer()
        {
            _width  =   0;
            _height =   0;
            _FBOID  =   0;
            _RBOID  =   0;
        }
    public:
        void    createFrameBuffer( int width,int height)
        {
            _width  =   width;
            _height =   height;

            glGenFramebuffers(1, &_FBOID);
            glBindFramebuffer(GL_FRAMEBUFFER, _FBOID);

            glGenRenderbuffers(1, &_RBOID);
            glBindRenderbuffer(GL_RENDERBUFFER, _RBOID);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _width, _height);

            glBindRenderbuffer(GL_RENDERBUFFER, 0);

            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _RBOID);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        void    begin(unsigned textureId )
        {
            glBindFramebuffer(GL_FRAMEBUFFER, _FBOID);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
            glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _RBOID );
        }

        void    end()
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        bool    resize(int width,int height )
        {
            if (width != _width || _height != height)
            {
                _width  =   width;
                _height =   height;
                glBindRenderbuffer(GL_RENDERBUFFER, _RBOID);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
                return  true;

            }
            return  false;
           
        }

    };
}