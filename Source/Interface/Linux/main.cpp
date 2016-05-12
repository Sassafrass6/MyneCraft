#include <iostream>
#include <exception>

#include "Application.h"

using namespace std;

int main(int argc, char** argv)
{
	Application app;
	
	try
	{
		app.init();
		app.mRoot->startRendering();
	}
	catch (exception e)
	{
		cerr << "Uncaught exception: " << e.what() << endl;
	}
	return 0;
}