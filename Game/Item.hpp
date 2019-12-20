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
	I32 constitution;
	I32 strength;
	I32 intellect;
	I32 dexterity;
};

static B32
func ItemGoesIntoSlot(ItemId item_id, SlotId slot_id)
{
	B32 goes_into_slot = false;

	if(item_id == NoItemId)
	{
		goes_into_slot = true;
	}
	else
	{
		switch(slot_id)
		{
			case AnySlotId:
			{
				goes_into_slot = true;
				break;
			}
			case HeadSlotId:
			{
				goes_into_slot = (item_id == TestHelmItemId);
				break;
			}
			default:
			{
				goes_into_slot = false;
			}
		}
	}

	return goes_into_slot;
}

static B32
func ItemIsEquippable(ItemId item_id)
{
	B32 is_equippable = false;
	switch(item_id)
	{
		case TestHelmItemId:
		{
			is_equippable = true;
			break;
		}
		default:
		{
			is_equippable = false;
		}
	}
	return is_equippable;
}

static ItemAttributes
func GetItemAttributes(ItemId item_id)
{
	Assert(ItemIsEquippable(item_id));
	ItemAttributes attributes = {};
	switch(item_id)
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

static I8 *
func GetItemName(ItemId item_id)
{
	I8 *name = 0;
	switch(item_id)
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

static I8 *
func GetItemSlotName(ItemId item_id)
{
	I8 *name = 0;
	switch(item_id)
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
func GetItemIdForCooldown(ItemId item_id)
{
	ItemId cooldown_item_id = NoItemId;
	switch(item_id)
	{
		case BlueFlowerOfIntellectItemId:
		case BlueFlowerOfHealingItemId:
		case BlueFlowerOfDampeningItemId:
		{
			cooldown_item_id = BlueFlowerItemId;
			break;
		}
		case RedFlowerOfStrengthItemId:
		case RedFlowerOfHealthItemId:
		case RedFlowerOfPoisonItemId:
		{
			cooldown_item_id = RedFlowerItemId;
			break;
		}
		case YellowFlowerOfDexterityItemId:
		case YellowFlowerOfAntivenomItemId:
		case YellowFlowerOfRageItemId:
		{
			cooldown_item_id = YellowFlowerItemId;
			break;
		}
		default:
		{
			cooldown_item_id = item_id;
		}
	}
	return cooldown_item_id;
}

static B32
func ItemHasOwnCooldown(ItemId item_id)
{
	I32 cooldown_item_id = GetItemIdForCooldown(item_id);
	B32 has_own_cooldown = (cooldown_item_id == item_id);
	return has_own_cooldown;
}

static R32
func GetItemCooldownDuration(ItemId item_id)
{
	item_id = GetItemIdForCooldown(item_id);
	Assert(ItemHasOwnCooldown(item_id));

	R32 cooldown = 0.0f;
	switch(item_id)
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
func GetItemTooltipText(ItemId item_id, I8 *buffer, I32 buffer_size)
{
	String text = StartString(buffer, buffer_size);

	I8 *name = GetItemName(item_id);
	AddLine(text, name);

	R32 cooldown = GetItemCooldownDuration(item_id);
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

	if(ItemIsEquippable(item_id))
	{
		ItemAttributes attributes = GetItemAttributes(item_id);

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

	switch(item_id)
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


static I8 *
func GetSlotName(SlotId slot_id)
{
	I8 *name = 0;
	switch(slot_id)
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
	I32 left;
	I32 top;

	I32 row_n;
	I32 col_n;
	ItemId *items;
	SlotId *slots;
};

struct InventoryItem
{
	Inventory *inventory;
	SlotId slot_id;
	ItemId item_id;
	IV2 slot;
};

static B32
func InventorySlotIsValid(Inventory *inventory, IV2 slot)
{
	B32 row_is_valid = IsIntBetween(slot.row, 0, inventory->row_n - 1);
	B32 col_is_valid = IsIntBetween(slot.col, 0, inventory->col_n - 1);
	B32 is_valid = (row_is_valid && col_is_valid);
	return is_valid;
}

static void
func SetInventorySlotId(Inventory *inventory, IV2 slot, SlotId slot_id)
{
	Assert(InventorySlotIsValid(inventory, slot));
	inventory->slots[slot.row * inventory->col_n + slot.col] = slot_id;
}

static SlotId
func GetInventorySlotId(Inventory *inventory, IV2 slot)
{
	Assert(InventorySlotIsValid(inventory, slot));
	SlotId slot_id = inventory->slots[slot.row * inventory->col_n + slot.col];
	return slot_id;
}

static void
func SetInventoryItemId(Inventory *inventory, IV2 slot, ItemId item_id)
{
	Assert(InventorySlotIsValid(inventory, slot));
	SlotId slot_id = GetInventorySlotId(inventory, slot);
	Assert(ItemGoesIntoSlot(item_id, slot_id));

	inventory->items[slot.row * inventory->col_n + slot.col] = item_id;
}

static void
func ClearInventory(Inventory *inventory)
{
	for(I32 row = 0; row < inventory->row_n; row++)
	{
		for(I32 col = 0; col < inventory->col_n; col++)
		{
			IV2 slot = MakeIntPoint(row, col);
			SetInventoryItemId(inventory, slot, NoItemId);
		}
	}
}

static ItemId
func GetInventoryItemId(Inventory *inventory, IV2 slot)
{
	Assert(InventorySlotIsValid(inventory, slot));
	ItemId itemId = inventory->items[slot.row * inventory->col_n + slot.col];
	return itemId;
}

static void
func InitInventory(Inventory *inventory, MemArena *arena, I32 row_n, I32 col_n)
{
	inventory->row_n = row_n;
	inventory->col_n = col_n;
	I32 item_n = (row_n * col_n);
	inventory->items = ArenaAllocArray(arena, ItemId, item_n);
	inventory->slots = ArenaAllocArray(arena, SlotId, item_n);
	for(I32 i = 0; i < item_n; i++)
	{
		inventory->items[i] = NoItemId;
		inventory->slots[i] = AnySlotId;
	}
}

static B32
func InventoryItemIsValid(InventoryItem *item)
{
	B32 isValid = (item->inventory && InventorySlotIsValid(item->inventory, item->slot));
	return isValid;
}

static InventoryItem
func GetInventoryItem(Inventory *inventory, IV2 slot)
{
	Assert(InventorySlotIsValid(inventory, slot));
	InventoryItem item = {};
	item.inventory = inventory;
	item.item_id = GetInventoryItemId(inventory, slot);
	item.slot_id = GetInventorySlotId(inventory, slot);
	item.slot = slot;
	return item;
}

static B32
func HasEmptySlot(Inventory *inventory)
{
	B32 has_empty_slot = false;
	for(I32 row = 0; row < inventory->row_n; row++)
	{
		for(I32 col = 0; col < inventory->col_n; col++)
		{
			IV2 slot = MakeIntPoint(row, col);
			ItemId item_id = GetInventoryItemId(inventory, slot);
			if(item_id == NoItemId)
			{
				has_empty_slot = true;
				break;
			}
		}

		if(has_empty_slot)
		{
			break;
		}
	}

	return has_empty_slot;
}


static void
func AddItemToInventory(Inventory *inventory, ItemId item_id)
{
	Assert(HasEmptySlot(inventory));
	B32 item_added = false;
	for(I32 row = 0; row < inventory->row_n; row++)
	{
		for(I32 col = 0; col < inventory->col_n; col++)
		{
			IV2 slot = MakeIntPoint(row, col);
			ItemId current_item_id = GetInventoryItemId(inventory, slot);
			if(current_item_id == NoItemId)
			{
				SetInventoryItemId(inventory, slot, item_id);
				item_added = true;
				break;
			}
		}

		if(item_added)
		{
			break;
		}
	}
	Assert(item_added);
}

static void
func MoveItemToInventory(InventoryItem item, Inventory *inventory)
{
	Assert(item.inventory != 0);
	Assert(item.inventory != inventory);
	Assert(HasEmptySlot(inventory));

	SetInventoryItemId(item.inventory, item.slot, NoItemId);
	AddItemToInventory(inventory, item.item_id);
}

static ItemId
func GetRandomFlowerItemId()
{
	ItemId flower_item_ids[] =
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
	I32 item_count = 9;

	I32 index_in_array = IntRandom(0, item_count - 1);
	ItemId item_id = flower_item_ids[index_in_array];

	return item_id;
}