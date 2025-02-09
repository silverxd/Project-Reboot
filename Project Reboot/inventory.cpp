#include "inventory.h"
#include "helper.h"

void LoopReplicatedEntries(UObject* Controller, std::function<bool(__int64*)> func)
{
	auto ReplicatedEntries = Inventory::GetReplicatedEntries(Controller);

	for (int i = 0; i < ReplicatedEntries->Num(); i++)
	{
		auto CurrentReplicatedEntry = ReplicatedEntries->AtPtr(i, FFortItemEntry::GetStructSize());

		if (CurrentReplicatedEntry)
		{
			if (func(CurrentReplicatedEntry))
				return;
		}
	}
}

void FFortItemEntry::SetLoadedAmmo(__int64* Entry, UObject* Controller, int NewLoadedAmmo)
{
	*GetLoadedAmmo(Entry) = NewLoadedAmmo;

	auto Inventory = Inventory::GetInventory(Controller);

	FastTArray::MarkItemDirty(Inventory, (FFastArraySerializerItem*)Entry);

	auto EntryGuid = *FFortItemEntry::GetGuid(Entry);

	auto ahah = [&Inventory, &EntryGuid, &NewLoadedAmmo](__int64* currentEntry) -> bool {
		if (*FFortItemEntry::GetGuid(currentEntry) == EntryGuid)
		{
			// std::cout << "Found!\n";
			*GetLoadedAmmo(currentEntry) = NewLoadedAmmo;
			FastTArray::MarkItemDirty(Inventory, (FFastArraySerializerItem*)currentEntry);
			return true;
		}

		return false;
	};

	LoopReplicatedEntries(Controller, ahah);
}

int FFortItemEntry::GetStructSize()
{
	static auto Size = Helper::GetSizeOfClass(ItemEntryStruct);

	return Size;
}

UObject* Inventory::GetWorldInventory(UObject* Controller)
{
	static auto WorldInventoryOffset = Controller->GetOffset("WorldInventory");
	auto WorldInventoryP = Get<UObject*>(Controller, WorldInventoryOffset);

	return *WorldInventoryP;
}

__int64* Inventory::GetInventory(UObject* Controller)
{
	auto WorldInventory = GetWorldInventory(Controller);

	if (WorldInventory)
	{
		static auto InventoryOffset = WorldInventory->GetOffset("Inventory");
		auto Inventory = Get<__int64>(WorldInventory, InventoryOffset);

		return Inventory;
	}

	return nullptr;
}

TArray<UObject*>* Inventory::GetItemInstances(UObject* Controller)
{
	static auto ItemInstancesOffset = FindOffsetStruct2("ScriptStruct /Script/FortniteGame.FortItemList", "ItemInstances");
	auto Inventory = GetInventory(Controller);
	
	return Inventory ? (TArray<UObject*>*)(__int64(Inventory) + ItemInstancesOffset) : nullptr;
}

template <typename EntryStruct>
TArray<EntryStruct>* Inventory::GetReplicatedEntries(UObject* Controller)
{
	static auto ReplicatedEntriesOffset = FindOffsetStruct2("ScriptStruct /Script/FortniteGame.FortItemList", "ReplicatedEntries");
	auto Inventory = GetInventory(Controller);

	return (TArray<EntryStruct>*)(__int64(Inventory) + ReplicatedEntriesOffset);
}

__int64* Inventory::GetEntryFromWeapon(UObject* Controller, UObject* Weapon)
{
	auto instance = Inventory::FindItemInInventory(Controller, Inventory::GetWeaponGuid(Weapon)); // ahhhhhhhhhhhhhh
	return UFortItem::GetItemEntry(instance);
}

UObject* GetQuickBars(UObject* Controller)
{
	static auto QuickBarsOffset = Controller->GetOffset("QuickBars");

	return *(UObject**)(__int64(Controller) + QuickBarsOffset);
}

void Inventory::Update(UObject* Controller, bool bAddOrRemove, FFastArraySerializerItem* ModifiedItem) // TODO: add eFortquickbars bars and thenm update accoridng to that
{
	auto Inventory = GetInventory(Controller);

	if (ModifiedItem)
	{
		// *FFortItemEntry::GetIsDirty((__int64*)ModifiedItem) = true;
		FastTArray::MarkItemDirty(Inventory, ModifiedItem);
	}
	else if (bAddOrRemove)
	{
		FastTArray::MarkArrayDirty(Inventory);
	}

	auto WorldInventory = GetWorldInventory(Controller);

	static auto WorldHandleInvUpdate = FindObject<UFunction>("/Script/FortniteGame.FortInventory.HandleInventoryLocalUpdate");
	WorldInventory->ProcessEvent(WorldHandleInvUpdate); // This like sets the OwnerInventory probs idfk

	if (Fortnite_Version < 7.4)
	{
		const auto QuickBars = GetQuickBars(Controller);

		if (QuickBars)
		{
			static auto OnRep_PrimaryQuickBar = FindObject<UFunction>("/Script/FortniteGame.FortQuickBars.OnRep_PrimaryQuickBar");
			QuickBars->ProcessEvent(OnRep_PrimaryQuickBar);

			static auto OnRep_SecondaryQuickBar = FindObject<UFunction>("/Script/FortniteGame.FortQuickBars.OnRep_SecondaryQuickBar");
			QuickBars->ProcessEvent(OnRep_SecondaryQuickBar);

			static auto OnRep_QuickBar = FindObject<UFunction>("/Script/FortniteGame.FortPlayerController.OnRep_QuickBar");
			Controller->ProcessEvent(OnRep_QuickBar);
		}
	}
	else
	{
		static auto ClientForceUpdateQuickbar = FindObject<UFunction>("/Script/FortniteGame.FortPlayerController.ClientForceUpdateQuickbar");

		auto PrimaryQuickbar = EFortQuickBars::Primary;
		Controller->ProcessEvent(ClientForceUpdateQuickbar, &PrimaryQuickbar);

		auto SecondaryQuickbar = EFortQuickBars::Secondary;
		Controller->ProcessEvent(ClientForceUpdateQuickbar, &SecondaryQuickbar);
	}
}

static float GetMaxStackSize(UObject* ItemDefinition)
{
	static auto MaxStackSizeOffset = ItemDefinition->GetOffset("MaxStackSize");

	bool bIsScalableFloat = Engine_Version >= 424; // prob wrong

	return bIsScalableFloat ? ((FScalableFloat*)(__int64(ItemDefinition) + MaxStackSizeOffset))->Value :
		*(int*)(__int64(ItemDefinition) + MaxStackSizeOffset);
}

bool ModifyItemCount(UObject* Controller, UObject* Instance, int IncreaseBy, bool bDecrease = false)
{
	if (Controller && Instance)
	{
		auto Inventory = Inventory::GetInventory(Controller);
		auto ItemEntry = UFortItem::GetItemEntry(Instance);

		auto ItemEntryGuid = *FFortItemEntry::GetGuid(ItemEntry);
		auto CurrentCount = FFortItemEntry::GetCount(ItemEntry);

		auto NewCount = bDecrease  ? *CurrentCount - IncreaseBy : *CurrentCount + IncreaseBy;
		auto OldCount = *CurrentCount;
		*CurrentCount = NewCount;

		auto ChangeCount = [&Inventory, &Controller, &ItemEntryGuid, &NewCount](__int64* Entry) -> bool {
			if (ItemEntryGuid == *FFortItemEntry::GetGuid(Entry))
			{
				*FFortItemEntry::GetCount(Entry) = NewCount;
				FastTArray::MarkItemDirty(Inventory, (FFastArraySerializerItem*)Entry);

				return true;
			}

			return false;
		};

		LoopReplicatedEntries(Controller, ChangeCount);

		FastTArray::MarkItemDirty(Inventory, (FFastArraySerializerItem*)ItemEntry);

		return true;
	}

	return false;
}

bool DecreaseItemCount(UObject* Controller, UObject* Instance, int DecreaseBy)
{
	return ModifyItemCount(Controller, Instance, DecreaseBy, true);
}

UObject* CreateItemInstance(UObject* Controller, UObject* Definition, int Count = 1)
{
	if (!Definition || !Controller)
		return nullptr;

	if (true) // Fortnite_Season < 19)
	{
		struct { int count; int level; UObject* instance; } Params{ Count, 1 };

		static auto CTIIFn = FindObject<UFunction>("/Script/FortniteGame.FortItemDefinition.CreateTemporaryItemInstanceBP");
		Definition->ProcessEvent(CTIIFn, &Params);

		UObject* itemInstance = Params.instance;

		if (itemInstance)
		{
			static auto SOCFTIFn = FindObject<UFunction>("/Script/FortniteGame.FortItem.SetOwningControllerForTemporaryItem");

			itemInstance->ProcessEvent(SOCFTIFn, &Controller);

			auto Entry = UFortItem::GetItemEntry(itemInstance);
			// *FFortItemEntry::GetIsReplicatedCopy(Entry) = true;

			return itemInstance;
		}
	}
	else
	{
		static auto FortWorldItemClass = FindObjectSlow("Class /Script/FortniteGame.FortWorldItem", false);
		static auto SizeOfItemEntryStruct = Helper::GetSizeOfClass(FFortItemEntry::ItemEntryStruct);

		return Helper::Easy::SpawnObject(FortWorldItemClass, Helper::GetTransientPackage());
	}

	return nullptr;
}

UObject* Inventory::GiveItem(UObject* Controller, UObject* ItemDefinition, EFortQuickBars Bars, int Slot, int Count, bool bUpdate)
{
	if (!ItemDefinition)
		return nullptr;

	static auto SizeOfItemEntryStruct = Helper::GetSizeOfClass(FFortItemEntry::ItemEntryStruct);

	auto ItemInstance = CreateItemInstance(Controller, ItemDefinition, Count);

	if (ItemInstance)
	{
		bool bDontCreateNewStack = false;
		bool bShouldStack = false;

		static auto FortResourceItemDefinition = FindObject("/Script/FortniteGame.FortResourceItemDefinition");

		if (ItemDefinition->IsA(FortResourceItemDefinition))
			bDontCreateNewStack = true;

		auto ItemEntry = UFortItem::GetItemEntry(ItemInstance);
		auto ItemInstances = Inventory::GetItemInstances(Controller);

		// start bad code

		__int64* StackingItemEntry = nullptr;
		UObject* StackingItemInstance = nullptr;

		int OverStack = 0;

		std::vector<UObject*> InstancesOfItem;

		auto MaxStackCount = GetMaxStackSize(ItemDefinition);

		std::cout << "MaxStackCount: " << MaxStackCount << '\n';

		if (MaxStackCount > 1)
		{
			for (int i = 0; i < ItemInstances->Num(); i++)
			{
				auto CurrentItemInstance = ItemInstances->At(i);

				if (CurrentItemInstance && *UFortItem::GetDefinition(CurrentItemInstance) == ItemDefinition)
				{
					InstancesOfItem.push_back(CurrentItemInstance);
				}
			}

			if (InstancesOfItem.size() > 0)
			{
				// We need this skunked thing because if they have 2 full stacks and half a stack then we want to find the lowest stack and stack to there.
				for (auto InstanceOfItem : InstancesOfItem)
				{
					if (InstanceOfItem)
					{
						auto CurrentItemEntry = UFortItem::GetItemEntry(InstanceOfItem);

						if (ItemEntry)
						{
							auto currentCount = FFortItemEntry::GetCount(ItemEntry);

							if (currentCount && *currentCount < MaxStackCount)
							{
								StackingItemInstance = InstanceOfItem;
								StackingItemEntry = CurrentItemEntry;
								break;
							}
						}
					}
				}
			}
		}

		if (StackingItemEntry)
		{
			auto currentCount = FFortItemEntry::GetCount(StackingItemEntry);

			if (currentCount)
			{
				//			      3	      +   2   -      6       =   -1
				OverStack = *currentCount + Count - GetMaxStackSize(ItemDefinition);

				// checks if it is going to overstack, if it is then we subtract the incoming count by the overstack, but its not then we just use the incoming count.
				int AmountToStack = OverStack > 0 ? Count - OverStack : Count;

				ModifyItemCount(Controller, StackingItemInstance, AmountToStack);

				if (OverStack <= 0) // there is no overstack, we can now return peacefully.
					return StackingItemInstance;
			}
		}

		if (bDontCreateNewStack && StackingItemInstance) 
			return ItemInstance;

		// to here

		auto CreateAndAddItem = [&](int countForItem) {
			*FFortItemEntry::GetItemDefinition(ItemEntry) = ItemDefinition;
			*FFortItemEntry::GetCount(ItemEntry) = countForItem;

			GetItemInstances(Controller)->Add(ItemInstance);
			GetReplicatedEntries(Controller)->Add(*ItemEntry, SizeOfItemEntryStruct);

			if (Fortnite_Version < 7.4)
			{
				static auto ServerAddItemInternal = FindObject<UFunction>("/Script/FortniteGame.FortQuickBars.ServerAddItemInternal");
				auto QuickBars = GetQuickBars(Controller);

				struct { FGuid Item; EFortQuickBars Quickbar; int Slot; } SAIIParams{ *FFortItemEntry::GetGuid(ItemEntry), Bars, Slot };

				QuickBars->ProcessEvent(ServerAddItemInternal, &SAIIParams);
			}

			if (bUpdate)
				Update(Controller, true);
		};

		if (OverStack > 0)
			CreateAndAddItem(OverStack);
		else
			CreateAndAddItem(Count);
	}

	return ItemInstance;
}

UObject* Inventory::EquipWeapon(UObject* Controller, const FGuid& Guid, UObject* ItemDefinition, int Ammo)
{
	// if (Helper::IsInAircraft(Controller)) // The pawn check does this ig
		// return nullptr;

	auto Pawn = Helper::GetPawnFromController(Controller);

	if (!Pawn)
		return nullptr;

	static auto FortTrapItemDefinitionClass = FindObject("Class /Script/FortniteGame.FortTrapItemDefinition");
	static auto FortDecoItemDefinitionClass = FindObject("/Script/FortniteGame.FortDecoItemDefinition");
	static auto AthenaGadgetItemDefinitionClass = FindObject("/Script/FortniteGame.FortGadgetItemDefinition");
	static auto FortContextTrapItemDefinitionClass = FindObject("/Script/FortniteGame.FortContextTrapItemDefinition");

	UObject* Wep = nullptr;

	if (ItemDefinition->IsA(AthenaGadgetItemDefinitionClass))
	{
		static auto GetWeaponItemDefinition = FindObject<UFunction>("/Script/FortniteGame.FortGadgetItemDefinition.GetWeaponItemDefinition");

		UObject* GadgetDefinition = nullptr;

		if (GetWeaponItemDefinition)
		{
			ItemDefinition->ProcessEvent(GetWeaponItemDefinition, &GadgetDefinition);
		}
		else
		{
			static auto GetDecoItemDefinition = FindObject<UFunction>("/Script/FortniteGame.FortGadgetItemDefinition.GetDecoItemDefinition");
			ItemDefinition->ProcessEvent(GetDecoItemDefinition, &GadgetDefinition);
		}

		std::cout << "GadgetDefinition: " << GadgetDefinition << '\n';

		auto CharacterPartsOffset = ItemDefinition->GetOffset("CharacterParts");
		auto CharacterParts = Get<TArray<UObject*>>(ItemDefinition, CharacterPartsOffset);

		for (int i = 0; i < CharacterParts->size(); i++)
		{
			auto CharacterPart = CharacterParts->At(i);

			Helper::ChoosePart(Pawn, (EFortCustomPartType)i, CharacterPart);
		}

		if (!GadgetDefinition)
			return nullptr;

		EquipWeapon(Controller, Guid, GadgetDefinition, Ammo);
	}
		
	else if (false) // (ItemDefinition->IsA(FortDecoItemDefinitionClass) && Engine_Version >= 424) || ItemDefinition->IsA(FortTrapItemDefinitionClass)) // IDK
	{
		UObject* WeaponClass = nullptr;
		static auto GetWeaponActorClass = FindObject<UFunction>("/Script/FortniteGame.FortWeaponItemDefinition.GetWeaponActorClass");
		ItemDefinition->ProcessEvent(GetWeaponActorClass, &WeaponClass);

		auto CurrentWeapon = Helper::GetCurrentWeapon(Pawn);

		Wep = CurrentWeapon;

		std::cout << "ItemDefinition: " << ItemDefinition->GetFullName() << '\n';
		std::cout << "WeaponClass: " << WeaponClass->GetFullName() << '\n';
		std::cout << "Wep Before: " << Wep->GetFullName() << '\n';

		auto PawnLoc = Helper::GetActorLocation(Pawn);
		auto newtool = Helper::Easy::SpawnActor(WeaponClass, Helper::GetActorLocation(Pawn));

		static auto PickUpActor = FindObject<UFunction>("/Script/FortniteGame.FortPawn.PickUpActor");
		struct { UObject* PickupActor; UObject* PlacementDecoItemDefinition; } parms{ newtool, ItemDefinition };

		Pawn->ProcessEvent(PickUpActor, &parms); // TODO NOT DO THIS

		std::cout << "Wep After: " << Wep->GetFullName() << '\n';

		static UObject* FortContextTrapTool = FindObject("/Script/FortniteGame.FortDecoTool_ContextTrap");

		// IDK

		if (Wep->IsA(FortContextTrapTool))
		{
			static auto ContextTrapItemDefinitionOffset = Wep->GetOffset("ContextTrapItemDefinition");
			auto ContextTrapItemDefinition = Get<UObject*>(Wep, ContextTrapItemDefinitionOffset);

			*ContextTrapItemDefinition = ItemDefinition->IsA(FortContextTrapItemDefinitionClass) ? ItemDefinition : nullptr;
		}

		// std::cout << "CurrentWeapon: " << CurrentWeapon->GetFullName() << '\n';

		/* static auto FortDecoHelperClass = FindObject("/Script/FortniteGame.FortDecoHelper");
		auto NewFortDecoHelper = Helper::Easy::SpawnActor(FortDecoHelperClass, PawnLoc);

		static auto DecoHelperOffset = Wep->GetOffset("DecoHelper");
		auto DecoHelper = Get<UObject*>(Wep, DecoHelperOffset);

		std::cout << "DecoHelper: " << *DecoHelper << '\n';

		*DecoHelper = NewFortDecoHelper;

		static auto TrapPickerDecoHelperOffset = Controller->GetOffset("TrapPickerDecoHelper");
		auto TrapPickerDecoHelper = Get<UObject*>(Controller, TrapPickerDecoHelperOffset);

		std::cout << "TrapPickerDecoHelper: " << *TrapPickerDecoHelper << '\n';

		*TrapPickerDecoHelper = *DecoHelper;

		if (!TrapPickerDecoHelper)
			return nullptr;

		static auto DecoItemDefinitionOffset = (*TrapPickerDecoHelper)->GetOffset("DecoItemDefinition");
		auto DecoItemDefinition = Get<UObject*>(*TrapPickerDecoHelper, DecoItemDefinitionOffset);

		*DecoItemDefinition = ItemDefinition;

		std::cout << "Wep: " << Wep->GetFullName() << '\n';

		// *DecoHelper = TrapPickerDecoHelper;

		static auto ItemDefinitionOffset = Wep->GetOffset("ItemDefinition");
		*Get<UObject*>(Wep, ItemDefinitionOffset) = ItemDefinition;

		*/

		// PC-TrapPickerDecoHelper
	}

	else
	{
		auto TrackerGuid = FGuid();

		struct { UObject* Def; FGuid Guid; UObject* Wep; } params{ ItemDefinition, Guid };
		struct { UObject* Def; FGuid Guid; FGuid TrackerGuid; UObject* Wep; } S16_params{ ItemDefinition, Guid, TrackerGuid };
		struct { UObject* Def; FGuid Guid; FGuid TrackerGuid; bool bDisableEquipAnimation; UObject* Wep; } S17_params{ ItemDefinition, Guid, TrackerGuid, false };

		static auto EquipWeaponDefinition = FindObject<UFunction>("/Script/FortniteGame.FortPawn.EquipWeaponDefinition");

		// bro what

		if (Fortnite_Season < 16)
		{
			Pawn->ProcessEvent(EquipWeaponDefinition, &params);
			Wep = params.Wep;
		}

		else if (Fortnite_Season == 16)
		{
			Pawn->ProcessEvent(EquipWeaponDefinition, &S16_params);
			Wep = S16_params.Wep;
		}

		else
		{
			Pawn->ProcessEvent(EquipWeaponDefinition, &S17_params);
			Wep = S17_params.Wep;
		}

		if (Wep)
		{
			{
				static auto AmmoCountOffset = Wep->GetOffset("AmmoCount");
				// *Get<int>(Wep, AmmoCountOffset) = Ammo;
			}
		}
	}

	return Wep;
}

UObject* Inventory::EquipWeapon(UObject* Controller, UObject* Instance, int Ammo)
{
	auto Def = UFortItem::GetDefinition(Instance);
	auto Guid = UFortItem::GetGuid(Instance);

	return Def && Guid ? EquipWeapon(Controller, *Guid, *Def, Ammo) : nullptr;
}

EFortQuickBars Inventory::WhatQuickBars(UObject* Definition)
{
	static auto FortWeaponItemDefinitionClass = FindObject("/Script/FortniteGame.FortWeaponItemDefinition");
	static auto FortTrapItemDefinitionClass = FindObject("/Script/FortniteGame.FortTrapItemDefinition"); // FindObject("/Script/FortniteGame.FortDecoItemDefinition");

	if (Definition->IsA(FortWeaponItemDefinitionClass) && !Definition->IsA(FortTrapItemDefinitionClass))
		return EFortQuickBars::Primary;
	else
		return EFortQuickBars::Secondary;
}

template <typename EntryStruct>
bool RemoveGuidFromReplicatedEntries(UObject* Controller, const FGuid& Guid)
{
	auto ReplicatedEntries = Inventory::GetReplicatedEntries<EntryStruct>(Controller);

	bool bSuccessful = false;

	for (int x = 0; x < ReplicatedEntries->Num(); x++)
	{
		auto ItemEntry = ReplicatedEntries->At(x);

		if (*FFortItemEntry::GetGuid((__int64*)&ItemEntry) == Guid)
		{
			ReplicatedEntries->RemoveAt(x);
			bSuccessful = true;
			break;
		}
	}

	if (!bSuccessful)
		std::cout << ("Failed to find ItemGuid in Inventory!\n");

	return bSuccessful;
}

UObject* Inventory::TakeItem(UObject* Controller, const FGuid& Guid, int Count, bool bForceRemove)
{
	auto Instance = Inventory::FindItemInInventory(Controller, Guid);

	if (!Instance)
		return nullptr;

	auto Def = *UFortItem::GetDefinition(Instance);

	if (!bForceRemove)
		DecreaseItemCount(Controller, Instance, Count);

	auto InstanceEntry = UFortItem::GetItemEntry(Instance);

	auto count = *FFortItemEntry::GetCount(InstanceEntry);

	std::cout << "Count: " << count << '\n';

	if (bForceRemove || count <= 0)
	{
		std::cout << "ugh!\n";

		bool bSuccessful = false;

		if (Fortnite_Season == 2)
		{
			struct ItemEntrySize { unsigned char Unk00[0xB8]; };
			bSuccessful = RemoveGuidFromReplicatedEntries<ItemEntrySize>(Controller, Guid);
		}
		else if (Fortnite_Season == 3)
		{
			struct ItemEntrySize { unsigned char Unk00[0xC8]; };
			bSuccessful = RemoveGuidFromReplicatedEntries<ItemEntrySize>(Controller, Guid);
		}
		else if (Fortnite_Season > 3 && Fortnite_Version < 7.30)
		{
			struct ItemEntrySize { unsigned char Unk00[0xD0]; };
			bSuccessful = RemoveGuidFromReplicatedEntries<ItemEntrySize>(Controller, Guid);
		}
		else if (Fortnite_Version >= 7.30 && Engine_Version <= 424)
		{
			struct ItemEntrySize { unsigned char Unk00[0x120]; };
			bSuccessful = RemoveGuidFromReplicatedEntries<ItemEntrySize>(Controller, Guid);
		}
		else if (Engine_Version == 425)
		{
			struct ItemEntrySize { unsigned char Unk00[0x150]; };
			bSuccessful = RemoveGuidFromReplicatedEntries<ItemEntrySize>(Controller, Guid);
		}
		else if (Engine_Version >= 426 && Fortnite_Version < 14.60) // idk if right
		{
			struct ItemEntrySize { unsigned char Unk00[0x160]; };
			bSuccessful = RemoveGuidFromReplicatedEntries<ItemEntrySize>(Controller, Guid);
		}
		else if (Fortnite_Version == 14.60) // idk when they did it
		{
			struct ItemEntrySize { unsigned char Unk00[0x180]; };
			bSuccessful = RemoveGuidFromReplicatedEntries<ItemEntrySize>(Controller, Guid);
		}
		else if (Fortnite_Version >= 15 && Fortnite_Version < 18)
		{
			struct ItemEntrySize { unsigned char Unk00[0x190]; };
			bSuccessful = RemoveGuidFromReplicatedEntries<ItemEntrySize>(Controller, Guid);
		}
		else if (Fortnite_Version >= 18)
		{
			struct ItemEntrySize { unsigned char Unk00[0x1A0]; };
			bSuccessful = RemoveGuidFromReplicatedEntries<ItemEntrySize>(Controller, Guid);
		}

		auto ItemInstances = Inventory::GetItemInstances(Controller);

		for (int i = 0; i < ItemInstances->Num(); i++)
		{
			auto ItemInstance = ItemInstances->At(i);

			if (*UFortItem::GetGuid(ItemInstance) == Guid)
			{
				ItemInstances->RemoveAt(i);
			}
		}
	}
	
	Inventory::Update(Controller, true);
	
	return Def;
}

void Inventory::WipeInventory(UObject* Controller, bool bTakePickaxe)
{
	auto PickaxeDef = Helper::GetPickaxeDef(Controller);

	auto ItemInstances = Inventory::GetItemInstances(Controller);

	if (ItemInstances)
	{
		auto NumItemInstances = ItemInstances->Num();
		std::cout << "ItemInstances->Num(): " << NumItemInstances << '\n';

		int Start = bTakePickaxe ? 5 : 6; // 0
		int loopedThrough = Start;

		for (int i = loopedThrough; i < NumItemInstances; i++)
		{
			auto CurrentItemInstance = ItemInstances->At(i);

			if (!CurrentItemInstance)
				continue;

			auto CurrentItemDefinition = *UFortItem::GetDefinition(CurrentItemInstance);

			if (!CurrentItemDefinition)
				continue;

			Inventory::TakeItem(Controller, *UFortItem::GetGuid(CurrentItemInstance), 1, true);
		}
	}
}

UObject* Inventory::FindItemInInventory(UObject* Controller, const FGuid& Guid)
{
	auto ItemInstances = GetItemInstances(Controller);

	if (ItemInstances)
	{
		for (int i = 0; i < ItemInstances->Num(); i++)
		{
			auto ItemInstance = ItemInstances->At(i);

			if (!IsBadReadPtr(ItemInstance))
			{
				auto ItemGuid = UFortItem::GetGuid(ItemInstance);

				if (!IsBadReadPtr(ItemGuid) && *ItemGuid == Guid)
					return ItemInstance;
			}
		}
	}

	return nullptr;
}

UObject* Inventory::FindItemInInventory(UObject* Controller, UObject* Definition)
{
	auto ItemInstances = GetItemInstances(Controller);

	if (ItemInstances)
	{
		for (int i = 0; i < ItemInstances->Num(); i++)
		{
			auto ItemInstance = ItemInstances->At(i);

			if (ItemInstance)
			{
				auto definition = UFortItem::GetDefinition(ItemInstance);

				if (*definition == Definition)
					return ItemInstance;
			}
		}
	}

	return nullptr;
}

bool Inventory::ServerExecuteInventoryItem(UObject* Controller, UFunction* Function, void* Parameters)
{
	if (!Parameters)
		return false;

	auto Guid = *(FGuid*)Parameters;

	auto Instance = FindItemInInventory(Controller, Guid);

	if (Instance)
	{
		auto Definition = UFortItem::GetDefinition(Instance);
		auto LoadedAmmo = FFortItemEntry::GetLoadedAmmo(UFortItem::GetItemEntry(Instance));
		std::cout << "LoadedAmmo: " << *LoadedAmmo << '\n';
		auto NewWeapon = EquipWeapon(Controller, Guid, Definition ? *Definition : nullptr, *LoadedAmmo);
	}
	else
		std::cout << "Unable to find Guid in Inventory for ServerExecuteInventoryItem!\n";

	return false;
}

bool Inventory::ServerAttemptInventoryDrop(UObject* Controller, UFunction*, void* Parameters)
{
	if (!Parameters)
		return false;

	struct AFortPlayerController_ServerAttemptInventoryDrop_Params { FGuid ItemGuid; int Count; };

	auto Params = (AFortPlayerController_ServerAttemptInventoryDrop_Params*)Parameters;

	auto Pawn = Helper::GetPawnFromController(Controller);

	if (!Pawn)
		return false;

	auto Instance = Inventory::FindItemInInventory(Controller, Params->ItemGuid);

	auto newDef = TakeItem(Controller, Params->ItemGuid, Params->Count);

	Helper::SummonPickup(Pawn, newDef, Helper::GetActorLocation(Pawn), EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, Params->Count, false, 
		*FFortItemEntry::GetLoadedAmmo(UFortItem::GetItemEntry(Instance)));

	// TODO: Remove from Pawn->CurrentWeaponList

	return false;
}

bool Inventory::ServerExecuteInventoryWeapon(UObject* Controller, UFunction*, void* Parameters)
{
	auto Weapon = Parameters ? *(UObject**)Parameters : nullptr;

	if (!Weapon)
		return false;

	auto Instance = FindItemInInventory(Controller, Inventory::GetWeaponGuid(Weapon));

	if (!Instance)
		return false;

	auto LoadedAmmo = *FFortItemEntry::GetLoadedAmmo(UFortItem::GetItemEntry(Instance));

	EquipWeapon(Controller, Instance, LoadedAmmo);

	return false;
}

bool Inventory::ServerHandlePickup(UObject* Pawn, UFunction*, void* Parameters)
{
	UObject* Pickup = Parameters ? *(UObject**)Parameters : nullptr;

	if (!Pickup)
		return false;

	static auto bPickedUpOffset = Pickup->GetOffset("bPickedUp");
	auto bPickedUp = Get<bool>(Pickup, bPickedUpOffset);

	if (*bPickedUp)
		return false;

	auto PickupEntry = Helper::GetEntryFromPickup(Pickup);

	auto Definition = FFortItemEntry::GetItemDefinition(PickupEntry);
	auto Count = FFortItemEntry::GetCount(PickupEntry);

	auto Controller = Helper::GetControllerFromPawn(Pawn);

	auto CurrentWeapon = Helper::GetCurrentWeapon(Pawn);
	auto CurrentWeaponDef = Helper::GetWeaponData(CurrentWeapon);

	auto ItemInstances = GetItemInstances(Controller);

	bool bShouldSwap = false;

	if (Inventory::WhatQuickBars(*Definition) == EFortQuickBars::Primary)
	{
		int PrimaryQuickBarSlotsFilled = 0;

		for (int i = 0; i < ItemInstances->Num(); i++)
		{
			auto ItemInstance = ItemInstances->At(i);

			if (!ItemInstance)
				break;

			auto Definition = *UFortItem::GetDefinition(ItemInstance);

			if (Inventory::WhatQuickBars(Definition) == EFortQuickBars::Primary)
			{
				PrimaryQuickBarSlotsFilled++;
				bShouldSwap = (PrimaryQuickBarSlotsFilled - 6) >= 5;

				if (bShouldSwap)
					break;
			}
		}

		std::cout << "PrimaryQuickBarSlotsFilled: " << PrimaryQuickBarSlotsFilled << '\n';

		if (CurrentWeaponDef == Helper::GetPickaxeDef(Controller) && bShouldSwap)
			return false;
	}

	int NextSlot = 1;

	struct FFortPickupLocationData
	{
		UObject* PickupTarget;                                             // 0x0000(0x0008) (ZeroConstructor, IsPlainOldData)
		UObject* CombineTarget;  // AFortPickup                                           // 0x0008(0x0008) (ZeroConstructor, IsPlainOldData)
		UObject* ItemOwner;   // APawn                                              // 0x0010(0x0008) (ZeroConstructor, IsPlainOldData)
		FVector                      LootInitialPosition;                                      // 0x0018(0x000C)
		FVector                      LootFinalPosition;                                        // 0x0024(0x000C)
		float                                              FlyTime;                                                  // 0x0030(0x0004) (ZeroConstructor, IsPlainOldData)
		FVector                   StartDirection;                                           // 0x0034(0x000C)
		FVector FinalTossRestLocation;                                    // 0x0040(0x000C)
		uint8_t                               TossState;    // EFortPickupTossState                                             // 0x004C(0x0001) (ZeroConstructor, IsPlainOldData)
		bool                                               bPlayPickupSound;                                         // 0x004D(0x0001) (ZeroConstructor, IsPlainOldData)
		unsigned char                                      UnknownData00[0x2];                                       // 0x004E(0x0002) MISSED OFFSET
		struct FGuid                                       PickupGuid;                                               // 0x0050(0x0010) (IsPlainOldData, RepSkip, RepNotify, Interp, NonTransactional, EditorOnly, NoDestructor, AutoWeak, ContainsInstancedReference, AssetRegistrySearchable, SimpleDisplay, AdvancedDisplay, Protected, BlueprintCallable, BlueprintAuthorityOnly, TextExportTransient, NonPIEDuplicateTransient, ExposeOnSpawn, PersistentInstance, UObjectWrapper, HasGetValueTypeHash, NativeAccessSpecifierPublic, NativeAccessSpecifierProtected, NativeAccessSpecifierPrivate)
	};

	static auto PickupLocationDataOffset = Pickup->GetOffset("PickupLocationData");
	auto PickupLocationData = (FFortPickupLocationData*)(__int64(Pickup) + PickupLocationDataOffset);

	PickupLocationData->PickupTarget = Pawn;
	PickupLocationData->ItemOwner = Pawn;
	// PickupLocationData->LootInitialPosition = Helper::GetActorLocation(Pickup);
	PickupLocationData->FlyTime = 0.40f;
	// PickupLocationData->StartDirection = StartDirection;

	static auto OnRep_PickupLocationData = FindObject<UFunction>("/Script/FortniteGame.FortPickup.OnRep_PickupLocationData");
	Pickup->ProcessEvent(OnRep_PickupLocationData);

	if (bShouldSwap)
	{
		auto CurrentWeaponGuid = Inventory::GetWeaponGuid(CurrentWeapon);
		auto CurrentWeaponInstance = Inventory::FindItemInInventory(Controller, CurrentWeaponDef); // ugh

		if (!CurrentWeaponInstance)
		{
			std::cout << std::format("[{}] Unable to find current weapon instance!\n", __FUNCTION__);
			return false;
		}

		struct { FGuid ItemGuid; int Count; } drop_parms{ CurrentWeaponGuid, *UFortItem::GetCount(CurrentWeaponInstance) };

		Inventory::ServerAttemptInventoryDrop(Controller, nullptr, &drop_parms);
	}

	auto Instance = GiveItem(Controller, *Definition, WhatQuickBars(*Definition), NextSlot, *Count);

	if (!Instance)
		return false;

	*bPickedUp = true;

	static auto OnRep_bPickedUp = FindObject<UFunction>("/Script/FortniteGame.FortPickup.OnRep_bPickedUp");
	Pickup->ProcessEvent(OnRep_bPickedUp);

	auto NewEntry = UFortItem::GetItemEntry(Instance);
	
	auto PickupLoadedAmmo = *FFortItemEntry::GetLoadedAmmo(PickupEntry);
	std::cout << "PickupLoadedAmmo: " << PickupLoadedAmmo << '\n';

	FFortItemEntry::SetLoadedAmmo(NewEntry, Controller, PickupLoadedAmmo);

	// Helper::DestroyActor(Pickup);

	return false;
}

void Inventory::HandleReloadCost(UObject* Weapon, int AmountToRemove)
{
	// std::cout << "pls help!\n";

	// Defines::HandleReloadCost(Weapon, AmountToRemove);

	static auto AmmoCountOffset = Weapon->GetOffset("AmmoCount");

	auto Pawn = Helper::GetOwner(Weapon);
	auto Controller = Helper::GetControllerFromPawn(Pawn);

	auto entry = Inventory::GetEntryFromWeapon(Controller, Weapon);

	if (IsBadReadPtr(entry))
		return;

	auto WepAmmoCount = *Get<int>(Weapon, AmmoCountOffset);

	std::cout << "WepAmmoCount: " << WepAmmoCount << '\n';

	FFortItemEntry::SetLoadedAmmo(entry, Controller, WepAmmoCount);

	if (Defines::bIsPlayground || Defines::bInfiniteAmmo)
		return;

	auto WeaponData = Helper::GetWeaponData(Weapon);
	auto AmmoDef = Helper::GetAmmoForDefinition(WeaponData).first;

	static auto FortTrapItemDefinitionClass = FindObject("/Script/FortniteGame.FortTrapItemDefinition");

	// std::cout << "WeaponData Name: " << WeaponData->GetFullName() << '\n';

	if (!AmmoDef || WeaponData->IsA(FortTrapItemDefinitionClass))
		AmmoDef = WeaponData;

	// std::cout << "ammoDefName: " << AmmoDef->GetFullName() << '\n';

	auto AmmoInstance = Inventory::FindItemInInventory(Controller, AmmoDef);

	Inventory::TakeItem(Controller, *UFortItem::GetGuid(AmmoInstance), AmountToRemove);
}