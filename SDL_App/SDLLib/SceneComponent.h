/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/
#pragma once

#include "Common.h"
#include "DownloadableComponent.h"

namespace Library
{
	class Scene;
	class SceneTime;

	class SceneComponent : public DownloadableComponent
	{
		RTTI_DECLARATIONS(SceneComponent, DownloadableComponent)

	public:
		SceneComponent();
		SceneComponent(Scene& game);
		virtual ~SceneComponent();

		Scene* GetGame();
		void SetGame(Scene& game);
		bool Enabled() const;
		void SetEnabled(bool enabled);

		virtual void Initialize();
		virtual void Update(const SceneTime& gameTime);

	protected:
		Scene* mGame;
		bool mEnabled;

	private:
		SceneComponent(const SceneComponent& rhs);
		SceneComponent& operator=(const SceneComponent& rhs);
	};
}
