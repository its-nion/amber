
#pragma once

#include "util/libraries.h"

#include "Window.h"
#include "Renderer.h"

class Application
{
	public:
		void create(const char* name, const int width, const int height);
		void update();
		void destroy();

	private:
		Window* _window;
		Renderer* _renderer;
};

