#pragma once
#include "Object2D.h"

/// <summary>
/// アイテムの基底クラス
/// </summary>
class ItemBase : public Object2D
{
public:
	const enum ItemKind {
		itNone = 0,
		itRescue,
		itDoor,
		grenade,
		penetrate
	};

	void StopItem(bool StopOrder)
	{
		Itemupdate = StopOrder;
	}

	bool IsStopItem()const
	{
		return Itemupdate;
	}

public:
	ItemBase() {};
	virtual ~ItemBase() {};
	bool Itemupdate = true;
};