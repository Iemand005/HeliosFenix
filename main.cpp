#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <X11/Xlib.h>
#endif
#include <string>
#include <iostream>
#include "Helios.hpp"

int main() {

	std::cout << "Starting Helios" << std::endl;

	try {
		Helios game;
		game.Run();
	} catch (const std::exception& e) {
		std::cerr << "Exception caught: " << e.what() << std::endl;
		return 1;
	} catch (...) {
		std::cerr << "Unknown exception caught" << std::endl;
		return 1;
	}

	return 0;
}

#ifdef _WIN32

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR lpCmdLine, int) {
	return main();
}

#endif