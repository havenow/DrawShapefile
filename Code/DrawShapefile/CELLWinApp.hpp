#pragma once

#include <Windows.h>
#include <tchar.h>
#include <math.h>

#include <EGL/egl.h>
#include <gles2/gl2.h>


#include "CELLMath.hpp"
#include "CELLShader.hpp"
#include "CELLShpReader.hpp"
#include "CELLFont.hpp"

namespace   CELL
{
    class   CELLWinApp
    {
    public:
        //! 实例句柄
        HINSTANCE   _hInstance;
        //! 窗口句柄
        HWND        _hWnd;
        //! 窗口的高度
        int         _width;
        //! 窗口的宽度
        int         _height;
        /// for gles2.0
        EGLConfig   _config;
        EGLSurface  _surface;
        EGLContext  _context;
        EGLDisplay  _display;
        //! 增加shader
        PROGRAM_P2_AC4  _shader;
		PROGRAM_P2_C4   _shaderShp;
		CELLShpReader   _shpReader;
		PROGRAM_P2_UV2  _shaderTex;
		CELLFont        _font;
		int2            _mouseDown;
		bool            _isDown;
    public:
        CELLWinApp(HINSTANCE hInstance)
            :_hInstance(hInstance)
        {
            WNDCLASSEX  winClass;
            winClass.lpszClassName  =   _T("CELLWinApp");
            winClass.cbSize         =   sizeof(winClass);
            winClass.style          =   CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
            winClass.lpfnWndProc    =   wndProc;
            winClass.hInstance      =   hInstance;
            winClass.hIcon	        =   0;
            winClass.hIconSm	    =   0;
            winClass.hCursor        =   LoadCursor(hInstance, IDC_ARROW);
            winClass.hbrBackground  =   (HBRUSH)GetStockObject(BLACK_BRUSH);
            winClass.lpszMenuName   =   NULL;
            winClass.cbClsExtra     =   0;
            winClass.cbWndExtra     =   0;
            RegisterClassEx(&winClass);

			_isDown = false;
        }
        virtual ~CELLWinApp()
        {
            UnregisterClass(_T("CELLWinApp"),_hInstance);
        }

        /**
        *   初始化 OpenGLES2.0
        */
        bool    initOpenGLES20()
        {
            const EGLint attribs[] =
            {
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_BLUE_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_RED_SIZE, 8,
                EGL_DEPTH_SIZE,24,
                EGL_NONE
            };
            EGLint 	format(0);
            EGLint	numConfigs(0);
            EGLint  major;
            EGLint  minor;

            //! 1
            _display	    =	eglGetDisplay(EGL_DEFAULT_DISPLAY);

            //! 2init
            eglInitialize(_display, &major, &minor);

            //! 3
            eglChooseConfig(_display, attribs, &_config, 1, &numConfigs);

            eglGetConfigAttrib(_display, _config, EGL_NATIVE_VISUAL_ID, &format);
            //! 4 
            _surface	    = 	eglCreateWindowSurface(_display, _config, _hWnd, NULL);

            //! 5
            EGLint attr[]   =   { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };
            _context 	    = 	eglCreateContext(_display, _config, 0, attr);
            //! 6
            if (eglMakeCurrent(_display, _surface, _surface, _context) == EGL_FALSE)
            {
                return false;
            }

            eglQuerySurface(_display, _surface, EGL_WIDTH,  &_width);
            eglQuerySurface(_display, _surface, EGL_HEIGHT, &_height);

            return  true;

        }
        /**
        *   销毁OpenGLES2.0
        */
        void    destroyOpenGLES20()
        {
            if (_display != EGL_NO_DISPLAY)
            {
                eglMakeCurrent(_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
                if (_context != EGL_NO_CONTEXT) 
                {
                    eglDestroyContext(_display, _context);
                }
                if (_surface != EGL_NO_SURFACE) 
                {
                    eglDestroySurface(_display, _surface);
                }
                eglTerminate(_display);
            }
            _display    =   EGL_NO_DISPLAY;
            _context    =   EGL_NO_CONTEXT;
            _surface    =   EGL_NO_SURFACE;
        }
    protected:
        static  LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            
            CELLWinApp*  pThis   =   (CELLWinApp*)GetWindowLong(hWnd,GWL_USERDATA);
            if (pThis)
            {
                return  pThis->onEvent(hWnd,msg,wParam,lParam);
            }
            if (WM_CREATE == msg)
            {
                CREATESTRUCT*   pCreate =   (CREATESTRUCT*)lParam;
                SetWindowLong(hWnd,GWL_USERDATA,(DWORD_PTR)pCreate->lpCreateParams);
            }
            return  DefWindowProc( hWnd, msg, wParam, lParam );
        }
    public:
        /**
        *   事件函数
        */
        virtual LRESULT onEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            switch (msg)
            {
            case WM_CLOSE:
            case WM_DESTROY:
                {
                    ::PostQuitMessage(0);
                }
                break;
			case WM_MOUSEWHEEL:
			{
				short   zDelta = HIWORD(wParam);
				zoom(zDelta);
			}
			break;
			case WM_LBUTTONDOWN:
			{
				_mouseDown = int2(LOWORD(lParam), HIWORD(lParam));
				_isDown = true;
			}
			break;
			case WM_LBUTTONUP:
			{
				_isDown = false;
			}
			break;
			case WM_MOUSEMOVE:
			{
				if (_isDown)
				{
					int2    pt = int2(LOWORD(lParam), HIWORD(lParam));



					float   longSize = _shpReader._xMax - _shpReader._xMin;
					float   latSize = _shpReader._yMax - _shpReader._yMin;
					float   xPixel = longSize / float(_width);
					float   yPixel = latSize / float(_height);

					int     xOffset = pt.x - _mouseDown.x;
					int     yOffset = pt.y - _mouseDown.y;

					_shpReader._xMax -= xPixel * xOffset;
					_shpReader._xMin -= xPixel * xOffset;

					_shpReader._yMax += yPixel * yOffset;
					_shpReader._yMin += yPixel * yOffset;

					_mouseDown = pt;
				}
			}
			break;
			case WM_SIZE:
			{
				if (::IsWindow(hWnd))
				{
					RECT    rt;
					GetClientRect(hWnd, &rt);
					_width = rt.right - rt.left;
					_height = rt.bottom - rt.top;

					_shpReader._width = _width;
					_shpReader._height = _height;

					_shpReader.resize();
					
				}
			}
			break;
            default:
                return  DefWindowProc( hWnd, msg, wParam, lParam ); 
            }
            return  S_OK;
            
        }

		void    zoom(short delta)
		{
			float   scalar = delta > 0 ? 1.2f : 0.7f;
			float   centerX = (_shpReader._xMax + _shpReader._xMin) * 0.5f;//采用中心点进行缩放
			float   centerY = (_shpReader._yMax + _shpReader._yMin) * 0.5f;

			float   width = (_shpReader._xMax - _shpReader._xMin) * scalar * 0.5f;
			float   height = (_shpReader._yMax - _shpReader._yMin) * scalar * 0.5f;

			//_shpReader._xMin = centerX - width;
			//_shpReader._xMax = centerX + width;
			//_shpReader._yMax = centerY + height;
			//_shpReader._yMin = centerY - height;

			_shpReader.setOrth(centerX - width, centerX + width, centerY + height, centerY - height);
		}

        virtual void    render()
        {

			struct  Vertex
			{
				float   x, y, u, v;
			};

			//! 视口，在Windows窗口指定的位置和大小上绘制OpenGL内容
			glViewport(0, 0, _width, _height);

			CELL::matrix4   screenProj = CELL::ortho<float>(0, float(_width), float(_height), 0, -100.0f, 100);

			if (_shpReader._needDraw)
			{
				_shpReader._needDraw = false;
				_shpReader._frameBuffer.begin(_shpReader._frameTextrue);
				//! 清空缓冲区
				glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
				//! 创建一个投影矩阵



				CELL::matrix4   longLatPrj = CELL::ortho<float>(_shpReader._xMin, _shpReader._xMax, _shpReader._yMin, _shpReader._yMax, -100.0f, 100);
				_shaderShp.begin();

				_shpReader._width = _width;
				_shpReader._height = _height;
				glUniformMatrix4fv(_shaderShp._MVP, 1, false, longLatPrj.data());


				_shpReader.render(_shaderShp);

				_shaderShp.end();

				_font.beginText(screenProj);

				_shpReader.renderText(_font);
				_font.endText();

				_shpReader._frameBuffer.end();
			}



			Vertex  vertex[] =
			{
				{ 0,0,       0,1 },
				{ _width,0,  1,1 },
				{ 0,_height, 0,0 },
				{ _width,_height, 1,0 },
			};

			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

			glBindTexture(GL_TEXTURE_2D, _shpReader._frameTextrue);
			_shaderTex.begin();
			glUniformMatrix4fv(_shaderTex._MVP, 1, false, screenProj.data());
			glUniform1i(_shaderTex._texture, 0);

			glVertexAttribPointer(_shaderTex._position, 2, GL_FLOAT, false, sizeof(Vertex), vertex);
			glVertexAttribPointer(_shaderTex._uv, 2, GL_FLOAT, false, sizeof(Vertex), &vertex[0].u);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			_shaderShp.end();

        }

		char*   getPathName()
		{
			static  char szResource[1024] = { 0 };
			char    szBuf[1024];
			GetModuleFileNameA(0, szBuf, sizeof(szBuf));

			std::string     strPathName = szBuf;
			size_t          pos = strPathName.rfind("\\");
			std::string     strPath = strPathName.substr(0, pos);
			pos = strPath.rfind("\\");
			strPathName = strPath.substr(0, pos);
			strcpy(szResource, strPathName.c_str());

			return  szResource;
		}

        /**
        *   主函数
        */
        int     main(int width,int height)
        {
            _hWnd   =   CreateWindowEx( NULL,
                                        _T("CELLWinApp"),
                                        _T("CELLWinApp"),
                                        WS_OVERLAPPEDWINDOW,
                                        CW_USEDEFAULT,
                                        CW_USEDEFAULT,
                                        width,
                                        height, 
                                        NULL, 
                                        NULL,
                                        _hInstance, 
                                        this
                                        );

            if (_hWnd == 0)
            {
                return  -1;
            }
            UpdateWindow(_hWnd);

            ShowWindow(_hWnd,SW_SHOW);

            if (!initOpenGLES20())
            {
                return  false;
            }
            _shader.initialize();
			_shaderShp.initialize();
			_shaderTex.initialize();
			char    filePath[1024];
			char    filePathDbf[1024];
			sprintf(filePath, "%s/data/china_province.shp", getPathName());
			sprintf(filePathDbf, "%s/data/china_province.dbf", getPathName());
			_shpReader.read(filePath, filePathDbf);
			_shpReader.init(_width, _height);

			sprintf(filePath, "%s/data/simsun.ttc", getPathName());
			_font.buildSystemFont(filePath, 20);
            MSG msg =   {0};
            while(msg.message != WM_QUIT)
            {
                if (msg.message == WM_DESTROY || 
                    msg.message == WM_CLOSE)
                {
                    break;
                }
                /**
                *   有消息，处理消息，无消息，则进行渲染绘制
                */
                if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
                { 
                    TranslateMessage( &msg );
                    DispatchMessage( &msg );
                }
                else
                {
                    render();
                    eglSwapBuffers(_display,_surface);
                }
            }
            /**
            *   销毁OpenGLES20
            */
            destroyOpenGLES20();

            return  0;
        }
    };
}
