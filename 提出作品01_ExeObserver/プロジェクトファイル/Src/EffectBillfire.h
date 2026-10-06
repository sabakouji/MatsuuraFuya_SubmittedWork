#pragma once
#include "EffectBase.h"
#include <string>
#include <vector>

class EffectManager;
class EffectBillfire : public EffectBase
{
public:
	EffectBillfire();
	~EffectBillfire();
	void Update() override;
	void Draw() override;
	void SetEffectName(std::string name);

private:
	BILLBOARDBASE* billB;
	VECTOR2        uvOffset;
	float          frame;

};