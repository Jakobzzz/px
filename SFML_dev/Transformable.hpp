#pragma once
#include <memory>
#include "Transform.hpp"

namespace px
{
	struct Transformable
	{
		explicit Transformable(std::unique_ptr<Transform> & transform) : transform(std::move(transform)) {}

		std::unique_ptr<Transform> transform;
	};
}