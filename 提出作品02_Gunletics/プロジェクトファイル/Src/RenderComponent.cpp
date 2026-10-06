#include "RenderComponent.h"

RenderComponent::RenderComponent(CSpriteImage* image, const VECTOR4& srcPattern)
{
	animator = new Animator(this);
	imageheigth = srcPattern.z;
	imagewidth = srcPattern.w;
	CreateSprite(image, srcPattern);

	wait = 0;
}

RenderComponent::~RenderComponent()
{
	SAFE_DELETE(animator);
}

void RenderComponent::Update()
{
	wait--;
	if (wait <= 0) wait = 0;
	if (wait > 0)
	{
		animator->SetFlash();
	}
	else
	{
		animator->ResetFlash();
	}
	animator->Update();
}

void RenderComponent::Draw()
{
	Object2D::Draw();
}

void RenderComponent::sidechange(bool side)
{
	if (!side)
	{
		animator->SetAnimationRange(0, 1, 0, false);
	}
	else
	{
		animator->SetAnimationRange(1, 1, 0, false);
	}
}

void RenderComponent::Flash(int flashtime)
{
	wait = flashtime;
}