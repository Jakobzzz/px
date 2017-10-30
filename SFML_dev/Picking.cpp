#include "Picking.hpp"
#include "Camera.hpp"
#include <iostream>

#define EPSILON 1.0e-25F

namespace px
{
	glm::vec3 Picking::m_direction;
	glm::vec3 Picking::m_origin;

	void Picking::PerformMousePicking(std::shared_ptr<Camera>& camera, float x, float y)
	{
		//Normalize mouse coordinates
		float normX = (2.0f * x) / (float)camera->GetWidth() - 1.f;
		float normY = (2.0f * y) / (float)camera->GetHeight() - 1.f;
		normY = -normY;

		//Translate to clip space
		glm::vec4 clipCoords = glm::vec4(normX, normY, -1.0f, 1.0f);

		//Translate to eye space
		glm::mat4 invertedProjection = glm::inverse(camera->GetProjectionMatrix());
		glm::vec4 eyeCoords = invertedProjection * clipCoords;
		glm::vec4 result = glm::vec4(eyeCoords.x, eyeCoords.y, -1.f, 0.f);

		//Translate to world space
		glm::mat4 invertedView = glm::inverse(camera->GetViewMatrix());
		glm::vec4 rayWorld = invertedView * result;

		//Store picked ray
		m_origin = camera->GetPosition();
		m_direction = glm::vec3(rayWorld.x, rayWorld.y, rayWorld.z);
		m_direction = glm::normalize(m_direction);
	}

	bool Picking::RaySphereIntersection(glm::mat4 modelMatrix, float radius)
	{
		//Now transform the ray origin from view to world space
		m_origin = glm::inverse(modelMatrix) * glm::vec4(m_origin, 1.f);

		//Now perform the the intersection test
		float a, b, c, discriminant;

		// Calculate the a, b, and c coefficients.
		a = (m_direction.x * m_direction.x) + (m_direction.y * m_direction.y) + (m_direction.z * m_direction.z);
		b = ((m_direction.x * m_origin.x) + (m_direction.y * m_origin.y) + (m_direction.z * m_origin.z)) * 2.0f;
		c = ((m_origin.x * m_origin.x) + (m_origin.y * m_origin.y) + (m_origin.z * m_origin.z)) - (radius * radius);

		// Find the discriminant.
		discriminant = (b * b) - (4 * a * c);

		// if discriminant is negative the picking ray missed the sphere, otherwise it intersected the sphere.
		if (discriminant < 0.0f)
		{
			return false;
		}

		return true;
	}

	bool Picking::RayOBBIntersection(glm::vec3 aabb_min, glm::vec3 aabb_max, glm::mat4 modelMatrix)
	{
		//Intersection based on the method described in the book Real-Time Rendering
		float tMin = -INFINITY;
		float tMax = INFINITY;

		glm::vec3 worldPos = glm::vec3(modelMatrix[3].x, modelMatrix[3].y, modelMatrix[3].z);
		glm::vec3 delta = worldPos - m_origin;

		//Test intersection with the 2 planes perpendicular to the OBB's X axis
		{
			glm::vec3 axis_x(modelMatrix[0].x, modelMatrix[0].y, modelMatrix[0].z);
			float e = glm::dot(axis_x, delta);
			float f = glm::dot(m_direction, axis_x);

			if (fabs(f) > EPSILON)
			{
				//Intersection with "left" and "right" plane
				float t1 = (e + aabb_min.x) / f;
				float t2 = (e + aabb_max.x) / f;

				//t1 and t2 now contain distances betwen ray origin and ray-plane intersections
				//We want t1 to represent the nearest intersection, thus if it's not the case then we will swap
				if (t1 > t2)
				{
					float w = t1;
					t1 = t2;
					t2 = w;
				}

				//tMax is the nearest "far" intersection (amongst the X,Y and Z planes pairs)
				if (t2 < tMax)
					tMax = t2;

				//tMin is the farthest "near" intersection (amongst the X,Y and Z planes pairs)
				if (t1 > tMin)
					tMin = t1;

				//If "far" is closer than "near", then there is NO intersection.
				if (tMax < tMin)
					return false;
			}
			else
			{
				if (-e + aabb_min.x > 0.0f || -e + aabb_max.x < 0.0f)
					return false;
			}
		}

		//Test intersection with the 2 planes perpendicular to the OBB's Y axis
		{
			glm::vec3 axis_y(modelMatrix[1].x, modelMatrix[1].y, modelMatrix[1].z);
			float e = glm::dot(axis_y, delta);
			float f = glm::dot(m_direction, axis_y);

			if (fabs(f) > EPSILON) 
			{
				float t1 = (e + aabb_min.y) / f;
				float t2 = (e + aabb_max.y) / f;

				if (t1 > t2) 
				{ 
					float w = t1; 
					t1 = t2; 
					t2 = w; 
				}

				if (t2 < tMax)
					tMax = t2;

				if (t1 > tMin)
					tMin = t1;

				if (tMin > tMax)
					return false;
			}
			else 
			{
				if (-e + aabb_min.y > 0.0f || -e + aabb_max.y < 0.0f)
					return false;
			}
		}

		//Test intersection with the 2 planes perpendicular to the OBB's Z axis
		{
			glm::vec3 axis_z(modelMatrix[2].x, modelMatrix[2].y, modelMatrix[2].z);
			float e = glm::dot(axis_z, delta);
			float f = glm::dot(m_direction, axis_z);

			if (fabs(f) > EPSILON) 
			{

				float t1 = (e + aabb_min.z) / f;
				float t2 = (e + aabb_max.z) / f;

				if (t1 > t2) 
				{ 
					float w = t1; 
					t1 = t2; 
					t2 = w; 
				}

				if (t2 < tMax)
					tMax = t2;

				if (t1 > tMin)
					tMin = t1;

				if (tMin > tMax)
					return false;
			}
			else 
			{
				if (-e + aabb_min.z > 0.0f || -e + aabb_max.z < 0.0f)
					return false;
			}
		}

		//Pass tested and we have an intersection
		return true;
	}

	glm::vec3 Picking::GetPickingRay()
	{
		return m_direction;
	}
}
