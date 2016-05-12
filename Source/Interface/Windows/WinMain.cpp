#include <exception>
#include <windows.h>

#include "Application.h"

using namespace std;

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Application app;

	try
	{
		app.init();
		app.mRoot->startRendering();
	}
	catch (exception e)
	{
		MessageBoxA(nullptr, e.what(), "Uncaught exception", MB_ICONERROR);
	}
	return 0;
}