#pragma once

#include "Debug.hpp"
#include "String.hpp"

enum ItemId
{
	NoItemId = 0,
	TestItemId,
	TestHelmId,
	TestChestId,
	TestPantsId,
	TestBootsId,
	TestGlovesId,
	TestPotionId
};

String func GetItemShortName(I32 itemId, I8* buffer, I32 bufferSize)
{
	String string = StartString(buffer, bufferSize);
	switch(itemId)
	{
		case TestItemId:
		{
			AddText(&string, "Test");
			break;
		}
		case TestHelmId:
		{
			AddText(&string, "Helm");
			break;
		}
		case TestChestId:
		{
			AddText(&string, "Chest");
			break;
		}
		case TestPantsId:
		{
			AddText(&string, "Pants");
			break;
		}
		case TestBootsId:
		{
			AddText(&string, "Boots");
			break;
		}
		case TestGlovesId:
		{
			AddText(&string, "Gloves");
			break;
		}
		case TestPotionId:
		{
			AddText(&string, "Potion");
			break;
		}
		default:
		{
			DebugBreak();
		}
	}
	return string;
}

String func GetItemName(I32 itemId, I8* buffer, I32 bufferSize)
{
	String string = StartString(buffer, bufferSize);
	switch(itemId)
	{
		case TestItemId:
		{
			AddText(&string, "Test Item");
			break;
		}
		case TestHelmId:
		{
			AddText(&string, "Test Helm");
			break;
		}
		case TestChestId:
		{
			AddText(&string, "Test Chest");
			break;
		}
		case TestPantsId:
		{
			AddText(&string, "Test Pants");
			break;
		}
		case TestBootsId:
		{
			AddText(&string, "Test Boots");
			break;
		}
		case TestGlovesId:
		{
			AddText(&string, "Test Gloves");
			break;
		}
		case TestPotionId:
		{
			AddText(&string, "Test Potion");
			break;
		}
		default:
		{
			DebugBreak();
		}
	}
	return string;
}

struct ItemAttributes
{
	I32 constitution;
	I32 strength;
	I32 intellect;
	I32 dexterity;
};

ItemAttributes GetItemAttributes(I32 itemId)
{
	ItemAttributes attributes = {};
	switch(itemId)
	{
		case TestPotionId:
		case TestItemId:
		{
			break;
		}
		case TestHelmId:
		{
			attributes.strength  = 2;
			attributes.dexterity = 1;
			break;
		}
		case TestChestId:
		{
			attributes.constitution = 1;
			attributes.strength     = 3;
			attributes.dexterity    = 2;
			break;
		}
		case TestPantsId:
		{

			attributes.constitution = 1;
			attributes.strength     = 1;
			attributes.dexterity    = 1;
			break;
		}
		case TestBootsId:
		{
			attributes.strength  = 1;
			attributes.dexterity = 1;
			break;
		}
		case TestGlovesId:
		{
			attributes.strength  = 1;
			attributes.dexterity = 1;
			break;
		}
		default:
		{
			DebugBreak();
		}
	}
	return attributes;
}

String func GetItemTooltipText(I32 itemId, I8* buffer, I32 bufferSize)
{
	String string = StartString(buffer, bufferSize);

	I8 itemName[32];
	GetItemName(itemId, itemName, 32);
	AddLine(string, itemName);

	ItemAttributes attributes = GetItemAttributes(itemId);

	if(attributes.constitution > 0)
	{
		AddLine(string, "+" + attributes.constitution + " Constitution");
	}
	if(attributes.strength > 0)
	{
		AddLine(string, "+" + attributes.strength + " Strength");
	}
	if(attributes.intellect > 0)
	{
		AddLine(string, "+" + attributes.intellect + " Intellect");
	}
	if(attributes.dexterity > 0)
	{
		AddLine(string, "+" + attributes.dexterity + " Dexterity");
	}

	if(itemId == TestPotionId)
	{
		AddLine(string, "Consume: Heal yourself for 30.");
	}

	return string;
}

enum SlotId
{
	AnySlotId,
	HeadSlotId,
	ChestSlotId,
	ArmsSlotId,
	LegsSlotId,
	FeetSlotId,
	HandSlotId,
	WaistSlotId
};

static B32 func IsValidSlotId(I32 id)
{
	B32 result = (id >= AnySlotId) && (id <= WaistSlotId);
	return result;
}

static String func GetSlotName(I32 id, I8* buffer, I32 bufferSize)
{
	String string = StartString(buffer, bufferSize);
	switch(id)
	{
		case HeadSlotId:
		{
			AddText(&string, "Head");
			break;
		}
		case ChestSlotId:
		{
			AddText(&string, "Chest");
			break;
		}
		case ArmsSlotId:
		{
			AddText(&string, "Arms");
			break;
		}
		case LegsSlotId:
		{
			AddText(&string, "Legs");
			break;
		}
		case FeetSlotId:
		{
			AddText(&string, "Feet");
			break;
		}
		case HandSlotId:
		{
			AddText(&string, "Hand");
			break;
		}
		case WaistSlotId:
		{
			AddText(&string, "Waist");
			break;
		}
		default:
		{
			DebugBreak();
		}
	}
	return string;
}

B32 ItemGoesIntoSlot(I32 itemId, I32 slotId)
{
	Assert(IsValidSlotId(slotId));

	B32 result = false;

	if(itemId == NoItemId)
	{
		result = true;
	}
	else
	{
		switch(slotId)
		{
			case AnySlotId:
			{
				result = true;
				break;
			}
			case HeadSlotId:
			{
				result = (itemId == TestHelmId);
				break;
			}
			case ChestSlotId:
			{
				result = (itemId == TestChestId);
				break;
			}
			case ArmsSlotId:
			{
				result = (itemId == TestGlovesId);
				break;
			}
			case LegsSlotId:
			{
				result = (itemId == TestPantsId);
				break;
			}
			case FeetSlotId:
			{
				result = (itemId == TestBootsId);
				break;
			}
			case HandSlotId:
			{
				result = false;
				break;
			}
			case WaistSlotId:
			{
				result = false;
				break;
			}
			default:
			{
				DebugBreak();
			}
		}
	}

	return result;
}