#include <iostream>
#include "Application.h"

/// <summary>
/// Runs & exits the application.
/// </summary>

int main()
{
	Application app;

	try
	{
		app.Run();
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}