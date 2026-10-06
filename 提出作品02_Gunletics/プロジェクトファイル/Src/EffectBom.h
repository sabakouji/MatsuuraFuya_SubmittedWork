#pragma once
#include "EffectBase.h"
#include "Animator.h"

/// <summary>
/// Œø‰Ê@”š”­Œø‰Ê‚ÌƒNƒ‰ƒX
/// </summary>
class EffectBom : public EffectBase
{
public:
	EffectBom(CSpriteImage* image);
	~EffectBom();
	void Update() override;
	void Draw() override;
private:
	Animator* animator;

};