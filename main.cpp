#include "XRGame.hpp"
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#elif !defined(__EMSCRIPTEN__) && !defined(__ANDROID__)
#include <X11/Xlib.h>
#endif
#include <string>
#include "Helios.hpp"

#include <exception>

#include "Log.hpp"

int main() {

	fe::LogSetTag("Helios");
	fe::Log("Helios starting");

	try {
		fe::LogToFile("Creating Helios game instance...");

		Helios game;

		fe::LogToFile("Running game...");

		game.Run();

		fe::LogToFile("Game exited normally");
	} catch (const std::exception& e) {
		fe::LogToFile(std::string("Exception caught: ") + e.what());
	} catch (...) {
		fe::LogToFile("Unknown exception caught");
	}

	return 0;
}

#ifdef _WIN32

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR lpCmdLine, int) {
	return main();
}

#endif