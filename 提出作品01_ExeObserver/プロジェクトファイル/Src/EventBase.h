#pragma once
#include "Object3D.h"
#include "Animator.h"

class EventBase : public Object3D
{
public:
	EventBase();
	virtual ~EventBase();

	virtual void Update() override;
	virtual void UpdateEvent() {};
	virtual void UpdateOnce();
	virtual void DeleteEvent();
	virtual void Draw() override;
	virtual void DrawPushEnter();

	virtual SphereCollider Collider() override;
	virtual bool HitCheck();

protected:
	std::string tag;
	bool needEnter;
	bool onceOn;

private:
	enum eventCycle 
	{
		ecStandby = 0,
		ecWait,
		ecEnd
	};
	eventCycle eventcycle;
	int countTime;

};