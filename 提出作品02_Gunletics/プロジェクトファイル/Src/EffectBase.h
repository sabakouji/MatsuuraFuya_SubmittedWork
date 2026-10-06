#pragma once
#include "Object2D.h"

/// <summary>
/// Œø‰Ê‚ÌŠî’êƒNƒ‰ƒX
/// </summary>
class EffectBase : public Object2D
{
public:
	virtual void SetPos(VECTOR2 pos){ transform.position = pos;	}
	EffectBase() {};
	virtual ~EffectBase() {};
};