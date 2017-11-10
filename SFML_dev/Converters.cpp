#include "Converters.hpp"

namespace px
{
	namespace utils
	{
		//Converters for handling the json file format more easily
		glm::vec3 FromVec3Json(std::vector<float> values)
		{
			glm::vec3 result = { values[0], values[1], values[2] };
			return result;
		}

		std::vector<float> ToVec3Json(glm::vec3 values)
		{
			std::vector<float> result = { values[0], values[1], values[2] };
			return result;
		}
	}
}
