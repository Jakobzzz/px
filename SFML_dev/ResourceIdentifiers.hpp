#pragma once

namespace px
{
	namespace Models
	{
		enum ID
		{
			Cube,
			Sphere,
			Cylinder,
			Capsule
		};
	}

	//Forward declaration and a few type definitions
	template <typename Identifier>
	class Model;

	typedef std::shared_ptr<Model<Models::ID>> ModelHolder;
}
