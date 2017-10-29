#include "RenderSystem.hpp"
#include "Renderable.hpp"
#include "Transformable.hpp"

namespace px
{
	RenderSystem::RenderSystem()
	{
	}

	RenderSystem::~RenderSystem()
	{
	}

	void RenderSystem::update(EntityManager & es, EventManager & events, TimeDelta dt)
	{
		ComponentHandle<Transformable> transform;
		ComponentHandle<Renderable> renderable;

		for (Entity entity : es.entities_with_components(transform, renderable))
		{
			Shader::SetMatrix4x4(renderable->object->GetShader(), "model", transform->transform->GetTransform());
			renderable->object->Draw();
			transform->transform->SetIdentity();
		}
	}
}
