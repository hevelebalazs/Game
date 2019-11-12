#pragma once

#include "Debug.hpp"
#include "Memory.hpp"
#include "String.hpp"

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
	BlueFlowerOfIntellectItemId,
	BlueFlowerOfHealingItemId,
	BlueFlowerOfDampeningItemId,
	RedFlowerItemId,
	RedFlowerOfStrengthItemId,
	RedFlowerOfHealthItemId,
	RedFlowerOfPoisonItemId,
	YellowFlowerItemId,
	YellowFlowerOfAntivenomItemId,
	YellowFlowerOfDexterityItemId,
	YellowFlowerOfRageItemId,
	CrystalItemId
};

struct ItemAttributes
{
	Int32 constitution;
	Int32 strength;
	Int32 intellect;
	Int32 dexterity;
};

static Bool32
func ItemGoesIntoSlot(ItemId itemId, SlotId slotId)
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
func ItemIsEquippable(ItemId itemId)
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
func GetItemAttributes(ItemId itemId)
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

static Int8 *
func GetItemName(ItemId itemId)
{
	Int8 *name = 0;
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
		case BlueFlowerOfIntellectItemId:
		{
			name = "Blue Flower of Intellect";
			break;
		}
		case BlueFlowerOfHealingItemId:
		{
			name = "Blue Flower of Healing";
			break;
		}
		case BlueFlowerOfDampeningItemId:
		{
			name = "Blue Flower of Dampening";
			break;
		}
		case RedFlowerItemId:
		{
			name = "Red Flower";
			break;
		}
		case RedFlowerOfStrengthItemId:
		{
			name = "Red Flower of Strength";
			break;
		}
		case RedFlowerOfHealthItemId:
		{
			name = "Red Flower of Health";
			break;
		}
		case RedFlowerOfPoisonItemId:
		{
			name = "Red Flower of Poison";
			break;
		}
		case YellowFlowerItemId:
		{
			name = "Yellow Flower";
			break;
		}
		case YellowFlowerOfAntivenomItemId:
		{
			name = "Yellow Flower of Antivenom";
			break;
		}
		case YellowFlowerOfDexterityItemId:
		{
			name = "Yellow Flower of Dexterity";
			break;
		}
		case YellowFlowerOfRageItemId:
		{
			name = "Yellow Flower of Rage";
			break;
		}
		case CrystalItemId:
		{
			name = "Crsytal";
			break;
		}
		default:
		{
			DebugBreak();
		}
	}
	return name;
}

static Int8 *
func GetItemSlotName(ItemId itemId)
{
	Int8 *name = 0;
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
		case BlueFlowerOfIntellectItemId:
		{
			name = "BFI";
			break;
		}
		case BlueFlowerOfHealingItemId:
		{
			name = "BFH";
			break;
		}
		case BlueFlowerOfDampeningItemId:
		{
			name = "BFD";
			break;
		}
		case RedFlowerItemId:
		{
			name = "RF";
			break;
		}
		case RedFlowerOfStrengthItemId:
		{
			name = "RFS";
			break;
		}
		case RedFlowerOfHealthItemId:
		{
			name = "RFH";
			break;
		}
		case RedFlowerOfPoisonItemId:
		{
			name = "RFP";
			break;
		}
		case YellowFlowerItemId:
		{
			name = "YF";
			break;
		}
		case YellowFlowerOfDexterityItemId:
		{
			name = "YFD";
			break;
		}
		case YellowFlowerOfAntivenomItemId:
		{
			name = "YFA";
			break;
		}
		case YellowFlowerOfRageItemId:
		{
			name = "YFR";
			break;
		}
		case CrystalItemId:
		{
			name = "CR";
			break;
		}
		default:
		{
			DebugBreak();
		}
	}
	return name;
}

static ItemId
func GetItemIdForCooldown(ItemId itemId)
{
	ItemId cooldownItemId = NoItemId;;
	switch(itemId)
	{
		case BlueFlowerOfIntellectItemId:
		case BlueFlowerOfHealingItemId:
		case BlueFlowerOfDampeningItemId:
		{
			cooldownItemId = BlueFlowerItemId;
			break;
		}
		case RedFlowerOfStrengthItemId:
		case RedFlowerOfHealthItemId:
		case RedFlowerOfPoisonItemId:
		{
			cooldownItemId = RedFlowerItemId;
			break;
		}
		case YellowFlowerOfDexterityItemId:
		case YellowFlowerOfAntivenomItemId:
		case YellowFlowerOfRageItemId:
		{
			cooldownItemId = YellowFlowerItemId;
			break;
		}
		default:
		{
			cooldownItemId = itemId;
		}
	}
	return cooldownItemId;
}

static Bool32
func ItemHasOwnCooldown(ItemId itemId)
{
	Int32 cooldownItemId = GetItemIdForCooldown(itemId);
	Bool32 hasOwnCooldown = (cooldownItemId == itemId);
	return hasOwnCooldown;
}

static Real32
func GetItemCooldownDuration(ItemId itemId)
{
	itemId = GetItemIdForCooldown(itemId);
	Assert(ItemHasOwnCooldown(itemId));

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
func GetItemTooltipText(ItemId itemId, Int8 *buffer, Int32 bufferSize)
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
		case BlueFlowerOfIntellectItemId:
		{
			AddLine(text, "Use: Gain +10 intellect and");
			AddLine(text, "-10 strength for 1 minute.");
			break;
		}
		case BlueFlowerOfHealingItemId:
		{
			AddLine(text, "Use: Heal for 5 every 3 seconds");
			AddLine(text, "for 1 minute.");
			break;
		}
		case BlueFlowerOfDampeningItemId:
		{
			AddLine(text, "Use: Decreased dealing done and taken");
			AddLine(text, "by 20% for 1 minute.");
			break;
		}
		case RedFlowerOfStrengthItemId:
		{
			AddLine(text, "Use: Gain +10 strength and");
			AddLine(text, "-10 intellect for 1 minute.");
			break;
		}
		case RedFlowerOfHealthItemId:
		{
			AddLine(text, "Use: Heal for 30.");
			break;
		}
		case RedFlowerOfPoisonItemId:
		{
			AddLine(text, "Use: Get poisoned.");
			break;
		}
		case YellowFlowerOfDexterityItemId:
		{
			AddLine(text, "Use: Gain +10 dexterity and");
			AddLine(text, "+10 constitution for 1 minute.");
			break;
		}
		case YellowFlowerOfAntivenomItemId:
		{
			AddLine(text, "Use: Gain immunity to poison");
			AddLine(text, "for 1 minute.");
			break;
		}
		case YellowFlowerOfRageItemId:
		{
			AddLine(text, "Use: Increase damage done and taken");
			AddLine(text, "by 20% for 1 minute.");
			break;
		}
		case CrystalItemId:
		{
			AddLine(text, "A piece of crystal.");
			break;
		}
		default:
		{
			DebugBreak();
		}
	}

	return text;
}


static Int8 *
func GetSlotName(SlotId slotId)
{
	Int8 *name = 0;
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
	ItemId *items;
	SlotId *slots;
};

struct InventoryItem
{
	Inventory *inventory;
	SlotId slotId;
	ItemId itemId;
	IntVec2 slot;
};

static Bool32
func InventorySlotIsValid(Inventory *inventory, IntVec2 slot)
{
	Bool32 rowIsValid = IsIntBetween(slot.row, 0, inventory->rowN - 1);
	Bool32 colIsValid = IsIntBetween(slot.col, 0, inventory->colN - 1);
	Bool32 isValid = (rowIsValid && colIsValid);
	return isValid;
}

static void
func SetInventorySlotId(Inventory *inventory, IntVec2 slot, SlotId slotId)
{
	Assert(InventorySlotIsValid(inventory, slot));
	inventory->slots[slot.row * inventory->colN + slot.col] = slotId;
}

static SlotId
func GetInventorySlotId(Inventory *inventory, IntVec2 slot)
{
	Assert(InventorySlotIsValid(inventory, slot));
	SlotId slotId = inventory->slots[slot.row * inventory->colN + slot.col];
	return slotId;
}

static void
func SetInventoryItemId(Inventory *inventory, IntVec2 slot, ItemId itemId)
{
	Assert(InventorySlotIsValid(inventory, slot));
	SlotId slotId = GetInventorySlotId(inventory, slot);
	Assert(ItemGoesIntoSlot(itemId, slotId));

	inventory->items[slot.row * inventory->colN + slot.col] = itemId;
}

static void
func ClearInventory(Inventory *inventory)
{
	for(Int32 row = 0; row < inventory->rowN; row++)
	{
		for(Int32 col = 0; col < inventory->colN; col++)
		{
			IntVec2 slot = MakeIntPoint(row, col);
			SetInventoryItemId(inventory, slot, NoItemId);
		}
	}
}

static ItemId
func GetInventoryItemId(Inventory *inventory, IntVec2 slot)
{
	Assert(InventorySlotIsValid(inventory, slot));
	ItemId itemId = inventory->items[slot.row * inventory->colN + slot.col];
	return itemId;
}

static void
func InitInventory(Inventory *inventory, MemArena *arena, Int32 rowN, Int32 colN)
{
	inventory->rowN = rowN;
	inventory->colN = colN;
	Int32 itemN = (rowN * colN);
	inventory->items = ArenaAllocArray(arena, ItemId, itemN);
	inventory->slots = ArenaAllocArray(arena, SlotId, itemN);
	for(Int32 i = 0; i < itemN; i++)
	{
		inventory->items[i] = NoItemId;
		inventory->slots[i] = AnySlotId;
	}
}

static Bool32
func InventoryItemIsValid(InventoryItem *item)
{
	Bool32 isValid = (item->inventory && InventorySlotIsValid(item->inventory, item->slot));
	return isValid;
}

static InventoryItem
func GetInventoryItem(Inventory *inventory, IntVec2 slot)
{
	Assert(InventorySlotIsValid(inventory, slot));
	InventoryItem item = {};
	item.inventory = inventory;
	item.itemId = GetInventoryItemId(inventory, slot);
	item.slotId = GetInventorySlotId(inventory, slot);
	item.slot = slot;
	return item;
}

static Bool32
func HasEmptySlot(Inventory *inventory)
{
	Bool32 hasEmptySlot = false;
	for(Int32 row = 0; row < inventory->rowN; row++)
	{
		for(Int32 col = 0; col < inventory->colN; col++)
		{
			IntVec2 slot = MakeIntPoint(row, col);
			ItemId itemId = GetInventoryItemId(inventory, slot);
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
func AddItemToInventory(Inventory *inventory, ItemId itemId)
{
	Assert(HasEmptySlot(inventory));
	Bool32 itemAdded = false;
	for(Int32 row = 0; row < inventory->rowN; row++)
	{
		for(Int32 col = 0; col < inventory->colN; col++)
		{
			IntVec2 slot = MakeIntPoint(row, col);
			ItemId currentItemId = GetInventoryItemId(inventory, slot);
			if(currentItemId == NoItemId)
			{
				SetInventoryItemId(inventory, slot, itemId);
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

static void
func MoveItemToInventory(InventoryItem item, Inventory *inventory)
{
	Assert(item.inventory != 0);
	Assert(item.inventory != inventory);
	Assert(HasEmptySlot(inventory));

	SetInventoryItemId(item.inventory, item.slot, NoItemId);
	AddItemToInventory(inventory, item.itemId);
}

static ItemId
func GetRandomFlowerItemId()
{
	ItemId flowerItemIds[] =
	{
		BlueFlowerOfIntellectItemId,
		BlueFlowerOfHealingItemId,
		BlueFlowerOfDampeningItemId,
		RedFlowerOfStrengthItemId,
		RedFlowerOfHealthItemId,
		RedFlowerOfPoisonItemId,
		YellowFlowerOfAntivenomItemId,
		YellowFlowerOfDexterityItemId,
		YellowFlowerOfRageItemId
	};
	Int32 itemCount = 9;

	Int32 indexInArray = IntRandom(0, itemCount - 1);
	ItemId itemId = flowerItemIds[indexInArray];

	return itemId;
}