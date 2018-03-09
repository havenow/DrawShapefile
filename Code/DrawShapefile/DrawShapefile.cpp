// DrawShapefile.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "DrawShapefile.h"
#include "CELLWinApp.hpp"
#ifdef _DEBUG
#include "vld/vld.h"
#endif

int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

#ifdef _DEBUG
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
#endif
	
	CELL::CELLWinApp  app(hInstance);
	app.main(800, 600);

#ifdef _DEBUG
	FreeConsole();
#endif
	return 0;
}
