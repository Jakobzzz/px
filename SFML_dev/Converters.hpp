#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace px
{
	namespace utils
	{
		//Converters for handling the json file format more easily
		glm::vec3 FromVec3Json(std::vector<float> values);
		std::vector<float> ToVec3Json(glm::vec3 values);
	}
}
