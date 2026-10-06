#pragma once
#include "Object3D.h"
#include "Animator.h"
#include "TextReader.h"

class Item : public Object3D
{
public:
	Item();
	virtual ~Item();

	void Update() override;
	void Draw() override;

	void MakeItem(TextReader* txt, int n);
	float Radius() { return radius; }

private:
	float radius;
	VECTOR3 rotUp;
};