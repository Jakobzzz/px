#pragma once
#include <memory>
#include "Render.hpp"

namespace px
{
	struct Renderable
	{
		explicit Renderable(std::unique_ptr<Render> & object) : object(std::move(object)) {}

		std::unique_ptr<Render> object;
	};
}