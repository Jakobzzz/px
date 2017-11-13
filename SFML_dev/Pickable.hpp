#pragma once
#include <memory>
#include "PickingBody.hpp"

namespace px
{
	struct Pickable
	{
		explicit Pickable(std::unique_ptr<PickingBody> & object) : object(std::move(object)) {}

		std::unique_ptr<PickingBody> object;
	};
}