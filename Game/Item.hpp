#pragma once

#include "Debug.hpp"

enum SlotId
{
	AnySlotId,
	HeadSlotId,
	ChestSlotId,
	HandsSlotId,
	WaistSlotId,
	LegsSlotId,
	FeetSlotId,
};

enum ItemId
{
	NoItemId,
	HealthPotionItemId,
	AntiVenomItemId,
	IntellectPotionItemId,
	TestHelmItemId,
	BlueFlowerItemId,
	RedFlowerItemId,
	YellowFlowerItemId
};

struct ItemAttributes
{
	Int32 constitution;
	Int32 strength;
	Int32 intellect;
	Int32 dexterity;
};


static Bool32
func ItemGoesIntoSlot(Int32 itemId, Int32 slotId)
{
	Bool32 goesIntoSlot = false;

	if(itemId == NoItemId)
	{
		goesIntoSlot = true;
	}
	else
	{
		switch(slotId)
		{
			case AnySlotId:
			{
				goesIntoSlot = true;
				break;
			}
			case HeadSlotId:
			{
				goesIntoSlot = (itemId == TestHelmItemId);
				break;
			}
			default:
			{
				goesIntoSlot = false;
			}
		}
	}

	return goesIntoSlot;
}


static Bool32
func ItemIsEquippable(Int32 itemId)
{
	Bool32 isEquippable = false;
	switch(itemId)
	{
		case TestHelmItemId:
		{
			isEquippable = true;
			break;
		}
		default:
		{
			isEquippable = false;
		}
	}
	return isEquippable;
}

static ItemAttributes
func GetItemAttributes(Int32 itemId)
{
	Assert(ItemIsEquippable(itemId));
	ItemAttributes attributes = {};
	switch(itemId)
	{
		case TestHelmItemId:
		{
			attributes.strength = 2;
			attributes.dexterity = 1;
			attributes.intellect = 1;
			break;
		}
		default:
		{
			DebugBreak();
		}
	}
	return attributes;
}

static Int8*
func GetItemName(Int32 itemId)
{
	Int8* name = 0;
	switch(itemId)
	{
		case HealthPotionItemId:
		{
			name = "Health Potion";
			break;
		}
		case AntiVenomItemId:
		{
			name = "Antivenom";
			break;
		}
		case IntellectPotionItemId:
		{
			name = "Intellect Potion";
			break;
		}
		case TestHelmItemId:
		{
			name = "Test Helm";
			break;
		}
		case BlueFlowerItemId:
		{
			name = "Blue Flower";
			break;
		}
		case RedFlowerItemId:
		{
			name = "Red Flower";
			break;
		}
		case YellowFlowerItemId:
		{
			name = "Yellow Flower";
			break;
		}
		default:
		{
			DebugBreak();
		}
	}
	return name;
}
static Int8*
func GetItemSlotName(Int32 itemId)
{
	Int8* name = 0;
	switch(itemId)
	{
		case HealthPotionItemId:
		{
			name = "HP";
			break;
		}
		case AntiVenomItemId:
		{
			name = "AV";
			break;
		}
		case IntellectPotionItemId:
		{
			name = "IP";
			break;
		}
		case TestHelmItemId:
		{
			name = "Helm";
			break;
		}
		case BlueFlowerItemId:
		{
			name = "BF";
			break;
		}
		case RedFlowerItemId:
		{
			name = "RF";
			break;
		}
		case YellowFlowerItemId:
		{
			name = "YF";
			break;
		}
		default:
		{
			DebugBreak();
		}
	}
	return name;
}

static Real32
func GetItemCooldownDuration(Int32 itemId)
{
	Real32 cooldown = 0.0f;
	switch(itemId)
	{
		case HealthPotionItemId:
		{
			cooldown = 30.0f;
			break;
		}
		case AntiVenomItemId:
		{
			cooldown = 10.0f;
			break;
		}
		case IntellectPotionItemId:
		{
			cooldown = 30.0f;
			break;
		}
		case BlueFlowerItemId:
		case RedFlowerItemId:
		case YellowFlowerItemId:
		{
			cooldown = 60.0f;
			break;
		}
		default:
		{
			cooldown = 0.0f;
		}
	}
	return cooldown;
}

static String
func GetItemTooltipText(Int32 itemId, Int8* buffer, Int32 bufferSize)
{
	String text = StartString(buffer, bufferSize);

	Int8* name = GetItemName(itemId);
	AddLine(text, name);

	Real32 cooldown = GetItemCooldownDuration(itemId);
	if(cooldown > 0.0f)
	{
		if(cooldown == 1.0f)
		{
			AddLine(text, "Cooldown: 1 second");
		}
		else
		{
			AddLine(text, "Cooldown: " + cooldown + " seconds");
		}
	}

	if(ItemIsEquippable(itemId))
	{
		ItemAttributes attributes = GetItemAttributes(itemId);

		if(attributes.constitution > 0)
		{
			AddLine(text, "+" + attributes.constitution + " Constitution");
		}
		if (attributes.strength > 0)
		{
			AddLine(text, "+" + attributes.strength + " Strength");
		}
		if(attributes.intellect > 0)
		{
			AddLine(text, "+" + attributes.intellect + " Intellect");
		}
		if(attributes.dexterity > 0)
		{
			AddLine(text, "+" + attributes.dexterity + " Dexterity");
		}
	}

	switch(itemId)
	{
		case HealthPotionItemId:
		{
			AddLine(text, "Use: Heal yourself for 30.");
			break;
		}
		case AntiVenomItemId:
		{
			AddLine(text, "Use: Remove poison from friendly target.");
			break;
		}
		case IntellectPotionItemId:
		{
			AddLine(text, "Use: Increase your intellect by 10");
			AddLine(text, "for 10 minutes.");
			break;
		}
		case TestHelmItemId:
		{
			AddLine(text, "Head piece for testing.");
			break;
		}
		case BlueFlowerItemId:
		case RedFlowerItemId:
		case YellowFlowerItemId:
		{
			AddLine(text, "Use: Eat the flower.");
			break;
		}
		default:
		{
			DebugBreak();
		}
	}

	return text;
}


static Int8*
func GetSlotName(Int32 slotId)
{
	Int8* name = 0;
	switch(slotId)
	{
		case HeadSlotId:
		{
			name = "Head";
			break;
		}
		case ChestSlotId:
		{
			name = "Chest";
			break;
		}
		case HandsSlotId:
		{
			name = "Hands";
			break;
		}
		case WaistSlotId:
		{
			name = "Waist";
			break;
		}
		case LegsSlotId:
		{
			name = "Legs";
			break;
		}
		case FeetSlotId:
		{
			name = "Feet";
			break;
		}
		default:
		{
			DebugBreak();
		}
	}
	return name;
}

struct Inventory
{
	Int32 left;
	Int32 top;

	Int32 rowN;
	Int32 colN;
	Int32* items;
	Int32* slots;
};

struct InventoryItem
{
	Inventory* inventory;
	Int32 slotId;
	Int32 itemId;
	IntVec2 slot;
};


static void
func SetInventorySlotId(Inventory* inventory, Int32 row, Int32 col, Int32 slotId)
{
	Assert(IsIntBetween(row, 0, inventory->rowN - 1));
	Assert(IsIntBetween(col, 0, inventory->colN - 1));
	inventory->slots[row * inventory->colN + col] = slotId;
}

static Int32
func GetInventorySlotId(Inventory* inventory, Int32 row, Int32 col)
{
	Assert(IsIntBetween(row, 0, inventory->rowN - 1));
	Assert(IsIntBetween(col, 0, inventory->colN - 1));
	Int32 slotId = inventory->slots[row * inventory->colN + col];
	return slotId;
}

static void
func SetInventoryItemId(Inventory* inventory, Int32 row, Int32 col, Int32 itemId)
{
	Assert(IsIntBetween(row, 0, inventory->rowN - 1));
	Assert(IsIntBetween(col, 0, inventory->colN - 1));
	Int32 slotId = GetInventorySlotId(inventory, row, col);
	Assert(ItemGoesIntoSlot(itemId, slotId));

	inventory->items[row * inventory->colN + col] = itemId;
}

static Int32
func GetInventoryItemId(Inventory* inventory, Int32 row, Int32 col)
{
	Assert(IsIntBetween(row, 0, inventory->rowN - 1));
	Assert(IsIntBetween(col, 0, inventory->colN - 1));
	Int32 itemId = inventory->items[row * inventory->colN + col];
	return itemId;
}

static void
func InitInventory(Inventory* inventory, MemArena* arena, Int32 rowN, Int32 colN)
{
	inventory->rowN = rowN;
	inventory->colN = colN;
	Int32 itemN = (rowN * colN);
	inventory->items = ArenaPushArray(arena, Int32, itemN);
	inventory->slots = ArenaPushArray(arena, Int32, itemN);
	for(Int32 i = 0; i < itemN; i++)
	{
		inventory->items[i] = NoItemId;
		inventory->slots[i] = AnySlotId;
	}
}

static Bool32
func InventorySlotIsValid(Inventory* inventory, IntVec2 slot)
{
	Bool32 rowIsValid = IsIntBetween(slot.row, 0, inventory->rowN - 1);
	Bool32 colIsValid = IsIntBetween(slot.col, 0, inventory->colN - 1);
	Bool32 isValid = (rowIsValid && colIsValid);
	return isValid;
}

static void
func SetSlotItemId(Inventory* inventory, IntVec2 slot, Int32 itemId)
{
	SetInventoryItemId(inventory, slot.row, slot.col, itemId);
}

static Bool32
func InventoryItemIsValid(InventoryItem* item)
{
	Bool32 isValid = (item->inventory && InventorySlotIsValid(item->inventory, item->slot));
	return isValid;
}

static Bool32
func HasEmptySlot(Inventory* inventory)
{
	Bool32 hasEmptySlot = false;
	for(Int32 row = 0; row < inventory->rowN; row++)
	{
		for(Int32 col = 0; col < inventory->colN; col++)
		{
			Int32 itemId = GetInventoryItemId(inventory, row, col);
			if(itemId == NoItemId)
			{
				hasEmptySlot = true;
				break;
			}
		}

		if(hasEmptySlot)
		{
			break;
		}
	}

	return hasEmptySlot;
}


static void
func AddItemToInventory(Inventory* inventory, Int32 itemId)
{
	Assert(HasEmptySlot(inventory));
	Bool32 itemAdded = false;
	for(Int32 row = 0; row < inventory->rowN; row++)
	{
		for(Int32 col = 0; col < inventory->colN; col++)
		{
			Int32 currentItemId = GetInventoryItemId(inventory, row, col);
			if(currentItemId == NoItemId)
			{
				SetInventoryItemId(inventory, row, col, itemId);
				itemAdded = true;
				break;
			}
		}

		if(itemAdded)
		{
			break;
		}
	}
	Assert(itemAdded);
}