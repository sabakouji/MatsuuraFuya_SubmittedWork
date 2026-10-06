#include "SceneFactory.h"
#include <windows.h>
#include <assert.h>
#include "TitleScene.h"
#include "PlayScene.h"
#include "OverScene.h"
#include "ClearScene.h"
#include "StageClearScene.h"
#include "DataCarrier.h"

SceneBase* SceneFactory::CreateFirst()
{
	SingleInstantiate <DataCarrier>();	// DataCarrierは全ての最初に作成される。一つしか作らない。NoDestroy。

	return new TitleScene();
}

SceneBase * SceneFactory::Create(const std::string & name)
{
	if (name == "TitleScene") {
		return new TitleScene();
	}
	if (name == "PlayScene") {
		return new PlayScene();
	}
	if (name == "OverScene") {
		return new OverScene();
	}
	if (name == "ClearScene") {
		return new ClearScene();
	}
	if (name == "StageClearScene") {
		return new StageClearScene();
	}
	assert(false);
	return nullptr;
}
