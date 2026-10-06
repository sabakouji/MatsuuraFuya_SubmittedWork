#pragma once
#include "Object3D.h"
#include "TextReader.h"

class Goal : public Object3D
{
public:
	Goal();
	~Goal();
	void Update() override;
	void Draw() override;

	void MakeGoal(TextReader* txt, int n);
	void SetupGoal(const VECTOR3& position, float rotationY = 0.0f, float triggerRadius = 3.0f, const std::string& tag = "Goal");

	SphereCollider Collider();
	float GetTriggerRadius() const { return m_triggerRadius; }
private:
	float m_triggerRadius;
	bool m_triggered;
};