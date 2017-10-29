#pragma once

#include <entityx\entityx.h>

using namespace entityx;

namespace px
{
	class RenderSystem : public System<RenderSystem>
	{
	public:
		explicit RenderSystem();
		~RenderSystem();

	public:
		void update(EntityManager &es, EventManager &events, TimeDelta dt) override;
	};
}
