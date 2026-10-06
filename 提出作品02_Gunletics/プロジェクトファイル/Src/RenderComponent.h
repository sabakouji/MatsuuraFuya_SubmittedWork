#pragma once
#include "Object2D.h"
#include "Animator.h"

class Player;
class RenderComponent : public Object2D
{
public:
	RenderComponent(CSpriteImage* image, const VECTOR4& srcPattern);
	~RenderComponent();

	void Update() override;
	void Draw() override;
	void sidechange(bool side);
	void Flash(int flashtime);

	void SetAngle(float angle)
	{
		transform.rotation = angle;
	}
	VECTOR2 GetCenter()
	{
		return transform.center;
	}
	float GetHeight()
	{
		return imageheigth;
	}
	float GetWidth()
	{
		return imagewidth;
	}
	float SetAngle()
	{
		return transform.rotation;
	}

private:
	Animator* animator;
	float imageheigth;
	float imagewidth;
	int wait;
};