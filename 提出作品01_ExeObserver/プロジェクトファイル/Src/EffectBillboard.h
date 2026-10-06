#pragma once
#include "EffectBase.h"
#include <string>
#include <vector>

class EffectManager;
class EffectBillboard : public EffectBase
{
public:
	EffectBillboard();
	virtual ~EffectBillboard();
	void Update() override;
	void Draw() override;
	void SetEffectName(std::string name);

private:
	BILLBOARDBASE* billB;
	VECTOR2        uvOffset;
	float          frame;

};