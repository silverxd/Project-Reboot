#pragma once

// TODO: Update ImGUI

#pragma comment(lib, "d3d9.lib")

#include <Windows.h>
#include <dxgi.h>
#include <d3d11.h>
#include <d3d9.h>

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_dx9.h>

#include <string>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_stdlib.h>
#include "fontawesome.h"
#include <vector>
#include <format>
#include <imgui/imgui_internal.h>
#include <set>
#include "definitions.h"
#include "loot.h"
#include "events.h"
#include "helper.h"
#include "calendar.h"

#define GAME_TAB 1
#define PLAYERS_TAB 2
#define GAMEMODE_TAB 3
#define THANOS_TAB 4
#define EVENT_TAB 5
#define LATEGAME_TAB 6
#define DUMP_TAB 7
#define SETTINGS_TAB 8
#define CREDITS_TAB 9

// THE BASE CODE IS FROM IMGUI GITHUB

static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

std::string wstring_to_utf8(const std::wstring& str)
{
	if (str.empty()) return {};
	const auto size_needed = WideCharToMultiByte(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), nullptr, 0, nullptr, nullptr);
	std::string str_to(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), &str_to[0], size_needed, nullptr, nullptr);
	return str_to;
}

void InitStyle()
{
	ImGui::GetIO().Fonts->AddFontFromFileTTF("Reboot Resources/fonts/ruda-bold.ttf", 17);
	ImGui::GetStyle().FrameRounding = 4.0f;
	ImGui::GetStyle().GrabRounding = 4.0f;

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.12f, 0.14f, 0.65f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.09f, 0.21f, 0.31f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

class Playera
{
public:
	std::string Name;
	int Kills = 0;

	Playera(const std::string& _Name, int _Kills) : Name(_Name), Kills(_Kills) {}

	Playera() {}
};

void TextCentered(std::string text, bool bNewLine = true) {
	if (bNewLine)
		ImGui::NewLine();

	float win_width = ImGui::GetWindowSize().x;
	float text_width = ImGui::CalcTextSize(text.c_str()).x;

	// calculate the indentation that centers the text on one line, relative
	// to window left, regardless of the `ImGuiStyleVar_WindowPadding` value
	float text_indentation = (win_width - text_width) * 0.5f;

	// if text is too long to be drawn on one line, `text_indentation` can
	// become too small or even negative, so we check a minimum indentation
	float min_indentation = 20.0f;
	if (text_indentation <= min_indentation) {
		text_indentation = min_indentation;
	}

	ImGui::SameLine(text_indentation);
	ImGui::PushTextWrapPos(win_width - text_indentation);
	ImGui::TextWrapped(text.c_str());
	ImGui::PopTextWrapPos();
}

bool ButtonCentered(std::string text, bool bNewLine = true) {
	if (bNewLine)
		ImGui::NewLine();

	float win_width = ImGui::GetWindowSize().x;
	float text_width = ImGui::CalcTextSize(text.c_str()).x;

	// calculate the indentation that centers the text on one line, relative
	// to window left, regardless of the `ImGuiStyleVar_WindowPadding` value
	float text_indentation = (win_width - text_width) * 0.5f;

	// if text is too long to be drawn on one line, `text_indentation` can
	// become too small or even negative, so we check a minimum indentation
	float min_indentation = 20.0f;
	if (text_indentation <= min_indentation) {
		text_indentation = min_indentation;
	}

	ImGui::SameLine(text_indentation);
	ImGui::PushTextWrapPos(win_width - text_indentation);
	auto res = ImGui::Button(text.c_str());
	ImGui::PopTextWrapPos();
	return res;
}

static int Width = 640;
static int Height = 480;

void MainUI()
{
	static int Tab = 1;
	static int PlayerTab = 2435892;
	static bool bIsEditingInventory = false;
	static bool bInformationTab = false;

	auto bLoaded = Server::BeaconHost && !Defines::bIsRestarting; // Looting::bInitialized;

	if (ImGui::BeginTabBar(""))
	{
		if (ImGui::BeginTabItem(ICON_FA_GAMEPAD " Game"))
		{
			Tab = GAME_TAB;
			PlayerTab = -1;
			bInformationTab = false;
			ImGui::EndTabItem();
		}

		// if (serverStatus == EServerStatus::Up)
		{
			if (ImGui::BeginTabItem(ICON_FA_PEOPLE_CARRY " Players"))
			{
				Tab = PLAYERS_TAB;
				ImGui::EndTabItem();
			}
		}

		if (false && ImGui::BeginTabItem(("Gamemode")))
		{
			Tab = GAMEMODE_TAB;
			PlayerTab = -1;
			bInformationTab = false;
			ImGui::EndTabItem();
		}

		if (false && (std::floor(Fortnite_Version) == 8 || Engine_Version >= 424 || Fortnite_Version == 4.1))
		{
			if (ImGui::BeginTabItem(("Thanos")))
			{
				Tab = THANOS_TAB;
				PlayerTab = -1;
				bInformationTab = false;
				ImGui::EndTabItem();
			}
		}

		// if (Events::HasEvent())
		if (Defines::bIsGoingToPlayMainEvent)
		{
			if (ImGui::BeginTabItem(("Event")))
			{
				Tab = EVENT_TAB;
				PlayerTab = -1;
				bInformationTab = false;
				ImGui::EndTabItem();
			}
		}

		if (false && Defines::bIsLateGame && ImGui::BeginTabItem(("Lategame")))
		{
			Tab = LATEGAME_TAB;
			PlayerTab = -1;
			bInformationTab = false;
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Dump"))
		{
			Tab = DUMP_TAB;
			PlayerTab = -1;
			bInformationTab = false;
			ImGui::EndTabItem();
		}

		if (false && ImGui::BeginTabItem(("Settings")))
		{
			Tab = SETTINGS_TAB;
			PlayerTab = -1;
			bInformationTab = false;
			ImGui::EndTabItem();
		}

		// maybe a Replication Stats for >3.3?

		if (false && ImGui::BeginTabItem(("Credits")))
		{
			Tab = CREDITS_TAB;
			PlayerTab = -1;
			bInformationTab = false;
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	std::vector<UObject*> AllControllers;

	float* WarmupCountdownEndTime = nullptr;
	UObject* GameState = nullptr;
	float TimeSeconds = 0;

	if (bLoaded)
	{
		GameState = Helper::GetGameState();
		static auto WarmupCountdownEndTimeOffset = GameState->GetOffset("WarmupCountdownEndTime");
		WarmupCountdownEndTime = Get<float>(GameState, WarmupCountdownEndTimeOffset);

		TimeSeconds = Helper::GetTimeSeconds();

		if (*WarmupCountdownEndTime - 10 >= TimeSeconds && *WarmupCountdownEndTime != -1) // IDK
			*WarmupCountdownEndTime = TimeSeconds + 40;	
	}

	if (PlayerTab == -1)
	{
		if (Tab == GAME_TAB)
		{
			if (bLoaded)
			{
				ImGui::Checkbox("Log ProcessEvent", &Defines::bLogProcessEvent);
				ImGui::Checkbox("Infinite Mats", &Defines::bInfiniteMats);
				ImGui::Checkbox("Infinite Ammo", &Defines::bInfiniteAmmo);

				static std::string ConsoleCommand;

				ImGui::InputText("Console command", &ConsoleCommand);

				if (ImGui::Button("Execute console command"))
				{
					auto wstr = std::wstring(ConsoleCommand.begin(), ConsoleCommand.end());

					auto aa = wstr.c_str();
					FString cmd = aa;

					std::cout << "boi: " << cmd.ToString() << '\n';

					Helper::ExecuteConsoleCommand(cmd);
				}

				if (Defines::bIsCreative)
					ImGui::InputText("URL", &Defines::urlForPortal);

				if (Fortnite_Version >= 14.60 && ImGui::Button("Summon Vehicles"))
				{
					Defines::bShouldSpawnVehicles = true;
				}

				if (ImGui::Button("Restart"))
				{
					Server::Restart();
				}

				if (Fortnite_Season == 19)
				{
					static int SnowIndex = 0;
					ImGui::SliderInt("SnowIndex", &SnowIndex, 0, 6);

					if (ImGui::Button("Set Snow Phase"))
					{
						Helper::SetSnowIndex(SnowIndex);
					}
				}

				if (ImGui::Button("Fill all vending machines"))
				{
					static auto BuildingItemCollectorClass = FindObject("/Script/FortniteGame.BuildingItemCollectorActor");

					std::cout << "BuildingItemCollectorClass: " << BuildingItemCollectorClass << '\n';

					auto BuildingItemCollectorActorActors = Helper::GetAllActorsOfClass(BuildingItemCollectorClass);

					std::cout << "Skid: " << BuildingItemCollectorActorActors.size() << '\n';

					auto GameData = Helper::GetGameData();

					if (GameData)
					{
						// for (auto BuildingItemCollectorActor : BuildingItemCollectorActorActors)
						for (int i = 0; i < BuildingItemCollectorActorActors.size(); i++)
						{
							auto BuildingItemCollectorActor = BuildingItemCollectorActorActors.At(i);

							std::cout << "A!\n";

							static auto CollectorUnitInfoClassName = FindObject("/Script/FortniteGame.CollectorUnitInfo") ? "/Script/FortniteGame.CollectorUnitInfo" :
								"/Script/FortniteGame.ColletorUnitInfo";

							static auto CollectorUnitInfoClass = FindObject(CollectorUnitInfoClassName);

							static auto CollectorUnitInfoClassSize = Helper::GetSizeOfClass(CollectorUnitInfoClass);

							static auto ItemCollectionsOffset = BuildingItemCollectorActor->GetOffset("ItemCollections");

							TArray<__int64>* ItemCollections = Get<TArray<__int64>>(BuildingItemCollectorActor, ItemCollectionsOffset); // CollectorUnitInfo

							static auto OutputItemOffset = FindOffsetStruct2(CollectorUnitInfoClassName, "OutputItem", false, true);

							std::cout << "OutputItemOffset: " << OutputItemOffset << '\n';

							int rand = GetRandomInt(3, 5);

							ERarity Rarity = (ERarity)rand;

							std::cout << "Rarity: " << (int)Rarity << '\n';

							auto Def0 = Looting::GetRandomItem(ItemType::Weapon, Looting::LootItems, Rarity).Definition;
							auto Def1 = Looting::GetRandomItem(ItemType::Weapon, Looting::LootItems, Rarity).Definition;
							auto Def2 = Looting::GetRandomItem(ItemType::Weapon, Looting::LootItems, Rarity).Definition;

							std::cout << "Def0: " << Def0 << '\n';
							std::cout << "Def1: " << Def1 << '\n';
							std::cout << "Def2: " << Def2 << '\n';

							static auto SetRarityColors = FindObject<UFunction>("/Game/Athena/Items/Gameplay/VendingMachine/B_Athena_VendingMachine.B_Athena_VendingMachine_C.SetRarityColors");

							struct FFortRarityItemData
							{
								char Name[0x18];
								FLinearColor                          Color1;                                            // 0x18(0x10)(Edit, BlueprintVisible, BlueprintReadOnly, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
								FLinearColor                          Color2;                                            // 0x28(0x10)(Edit, BlueprintVisible, BlueprintReadOnly, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
								FLinearColor                          Color3;                                            // 0x38(0x10)(Edit, BlueprintVisible, BlueprintReadOnly, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
								FLinearColor                          Color4;                                            // 0x48(0x10)(Edit, BlueprintVisible, BlueprintReadOnly, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
								FLinearColor                          Color5;
							};

							FLinearColor Color{};

							constexpr bool bGetColorFromRarityData = false;

							if (bGetColorFromRarityData)
							{
								bool bIsSoftObjectPtr = Engine_Version > 420; // Idk when

								auto RarityDataOffset = GameData->GetOffset("RarityData");
								auto RarityData = bIsSoftObjectPtr ? Get<TSoftObjectPtr>(GameData, RarityDataOffset)->Get(nullptr, true) : *Get<UObject*>(GameData, RarityDataOffset);

								// std::cout << "RarityData: " << RarityData << '\n';

								static auto RarityCollectionOffset = RarityData->GetOffset("RarityCollection");
								// FFortRarityItemData* RarityCollections[0x8] = Get<FFortRarityItemData[0x8]>(RarityData, RarityCollectionOffset); // 0x8-0xA

								FFortRarityItemData* aa = *Get<FFortRarityItemData*>(RarityData, RarityCollectionOffset); // [(int)Rarity - 1] ;// RarityCollections[(int)Rarity - 1];

								static auto SizeOfFFortRarityItemData = Helper::GetSizeOfClass(FindObject("/Script/FortniteGame.FortRarityItemData"));
								FFortRarityItemData RarityCollection = *(FFortRarityItemData*)(__int64(aa) + (SizeOfFFortRarityItemData * ((int)Rarity - 1)));

								// FLinearColor Color = Rarity == ERarity::Rare ? RarityCollection->Color3 : 
									// Rarity == ERarity::Epic ? RarityCollection->Color4 : RarityCollection->Color5;

								FLinearColor Color = RarityCollection.Color1;

								std::cout << "Color Before: " << Color.Describe() << '\n';

								/* static auto GetRarityData = FindObject<UFunction>("/Script/FortniteGame.FortRarityData.BPGetRarityData");
								struct { UObject* ItemDef; FFortRarityItemData RarityItemData; } GetRarityDataparms{Def0};
								RarityData->ProcessEvent(GetRarityData, &GetRarityDataparms);

								Color = GetRarityDataparms.RarityItemData.Color1; */
							}
							else
							{
								// got from 3.5 raritydata

								// Color = Rarity == ERarity::Rare ? FLinearColor(0, 255, 245.99493, 255) :
									// Rarity == ERarity::Epic ? FLinearColor(213.350085, 5.1, 255, 255) : FLinearColor(245.995185, 138.98724, 31.979295, 255);

								Color = Rarity == ERarity::Rare ? FLinearColor(0, 1, 0.964686, 1) :
									Rarity == ERarity::Epic ? FLinearColor(0.836667, 0.02, 1, 1) : FLinearColor(0.964687, 0.545048, 0.125409, 1);
							}

							std::cout << "Color After: " << Color.Describe() << '\n';

							*Get<UObject*>(ItemCollections->AtPtr(0, CollectorUnitInfoClassSize), OutputItemOffset) = Def0;
							*Get<UObject*>(ItemCollections->AtPtr(1, CollectorUnitInfoClassSize), OutputItemOffset) = Def1;
							*Get<UObject*>(ItemCollections->AtPtr(2, CollectorUnitInfoClassSize), OutputItemOffset) = Def2;

							BuildingItemCollectorActor->ProcessEvent(SetRarityColors, &Color);
						}
					}

					BuildingItemCollectorActorActors.Free();
				}

				/* if (ImGui::Button("test looting"))
				{
					auto LootDrops = Looting::PickLootDrops("Loot_AthenaFloorLoot_Warmup");
					std::cout << "LootDrops Num: " << LootDrops.size() << '\n';

					for (int i = 0; i < LootDrops.size(); i++)
					{
						auto& LootDrop = LootDrops.at(i);

						std::cout << std::format("[{}] {} {}\n", i, LootDrop.first->GetFullName(), LootDrop.second);
					}
				} */

				if (Defines::bIsCreative && ImGui::Button("Apply"))
				{
					auto aa = std::wstring(Defines::urlForPortal.begin(), Defines::urlForPortal.end());
					const wchar_t* url = aa.c_str();

					static auto ImageURLOffset = Defines::Portal->GetOffset("ImageURL");
					*Get<FString>(Defines::Portal, ImageURLOffset) = url;

					static auto OnThumbnailTextureReady = FindObject<UFunction>("/Script/FortniteGame.FortAthenaCreativePortal.OnThumbnailTextureReady");
					Defines::Portal->ProcessEvent(OnThumbnailTextureReady);

					static auto OnRep_ImageURLChanged = FindObject<UFunction>("/Script/FortniteGame.FortAthenaCreativePortal.OnRep_ImageURLChanged");
					Defines::Portal->ProcessEvent(OnRep_ImageURLChanged);
				}

				if (Fortnite_Version == 14.60 && ImGui::Button("show funny flying aircfaft"))
				{
					auto foundation = FindObject("/Game/Athena/Apollo/Maps/Apollo_POI_Foundations.Apollo_POI_Foundations.PersistentLevel.Lobby_Foundation3");
					std::cout << "foundation: " << foundation << '\n';

					ShowFoundation(foundation);
				}

				ImGui::Text(("Game has been going on for " + std::to_string(TimeSeconds)).c_str());
				ImGui::SliderFloat("Warmup end", WarmupCountdownEndTime, TimeSeconds + 9, TimeSeconds + 1000);
			}
		}

		else if (Tab == PLAYERS_TAB)
		{
			if (bLoaded)
			{
				auto World = Helper::GetWorld();

				if (World)
				{
					static auto NetDriverOffset = World->GetOffset("NetDriver");
					auto NetDriver = *(UObject**)(__int64(World) + NetDriverOffset);

					if (NetDriver)
					{
						static auto ClientConnectionsOffset = NetDriver->GetOffset("ClientConnections");
						auto ClientConnections = (TArray<UObject*>*)(__int64(NetDriver) + ClientConnectionsOffset);

						if (ClientConnections)
						{
							for (int i = 0; i < ClientConnections->Num(); i++)
							{
								auto Connection = ClientConnections->At(i);

								if (!Connection)
									continue;

								static auto Connection_PlayerControllerOffset = Connection->GetOffset("PlayerController");
								auto CurrentController = *(UObject**)(__int64(Connection) + Connection_PlayerControllerOffset);

								if (CurrentController)
								{
									AllControllers.push_back(CurrentController);
								}
							}

							for (int i = 0; i < AllControllers.size(); i++)
							{
								auto CurrentController = AllControllers.at(i);
								auto CurrentPlayerState = Helper::GetPlayerStateFromController(CurrentController);

								if (!CurrentPlayerState)
								{
									std::cout << "tf!\n";
									continue;
								}

								FString NameFStr;

								static auto GetPlayerName = FindObject<UFunction>("/Script/Engine.PlayerState.GetPlayerName");
								CurrentPlayerState->ProcessEvent(GetPlayerName, &NameFStr);

								const wchar_t* NameWCStr = NameFStr.Data.Data;
								std::wstring NameWStr = std::wstring(NameWCStr);
								std::string Name = std::string(NameWStr.begin(), NameWStr.end());

								auto NameCStr = Name.c_str();

								if (ImGui::Button(NameCStr))
								{
									std::cout << "wtf! " << i << '\n';
									PlayerTab = i;
								}
							}
						}
					}
				}
			}
		}

		else if (Tab == EVENT_TAB)
		{
			if (ImGui::Button("loadevent"))
			{
				Events::LoadEvent();
			}

			if (ImGui::Button("startevent"))
			{
				Events::StartEvent();
			}

			if ((Fortnite_Version == 7.10 || Fortnite_Version == 11.31 || Fortnite_Version == 15.10) && ImGui::Button("Start New Years Event"))
			{
				Events::StartNewYears();
			}

			if (Fortnite_Version == 14.60 && ImGui::Button("Show carrier"))
			{
				struct { UObject* GameState; UObject* Playlist; FGameplayTagContainer PlaylistContextTags; } bbparms{ Helper::GetGameState(), *Helper::GetPlaylist(),
					FGameplayTagContainer() };

				auto fnc = FindObject<UFunction>("/Junior/Blueprints/BP_CarrierLoader.BP_CarrierLoader_C.OnReady_13D45E9346036B11F782F9922BC368EC");
				auto loader = FindObject("/Junior/Levels/Junior_Map.Junior_Map.PersistentLevel.BP_CarrierLoader_2");

				loader->ProcessEvent(fnc, &bbparms);
			}

			if (Fortnite_Version == 8.51)
			{
				std::string unvaultedItemName;

				ImGui::InputText("Unvaulted Item", &unvaultedItemName);

				if (ImGui::Button("Unvault"))
				{
					std::wstring unvaultedItemNameWStr = std::wstring(unvaultedItemName.begin(), unvaultedItemName.end());
					const wchar_t* unvaultedItemNameCWStr = unvaultedItemNameWStr.c_str();
					FString unvaultedItemNameFStr = unvaultedItemNameCWStr;
					FName unvaultedItemNameFName = Helper::Conversion::StringToName(unvaultedItemNameFStr);

					Events::Unvault(unvaultedItemNameFName);
				}
			}
		}

		else if (Tab == DUMP_TAB)
		{
			static std::string ahh = std::format("Fortnite Version {}\n\n", std::to_string(Fortnite_Version));

			if (ImGui::Button("Dump Objects"))
			{
				auto ObjectNum = OldObjects ? OldObjects->Num() : NewObjects->Num();

				std::ofstream obj("ObjectsDump.txt");

				obj << ahh;

				for (int i = 0; i < ObjectNum; i++)
				{
					auto CurrentObject = GetObjectByIndex(i);

					if (!CurrentObject)
						continue;

					obj << CurrentObject->GetFullName() << '\n';
				}
			}

			if (ImGui::Button("Dump Skins (Skins.txt)"))
			{
				std::ofstream SkinsFile("Skins.txt");

				if (SkinsFile.is_open())
				{
					SkinsFile << ahh;

					static auto CIDClass = FindObjectSlow("Class /Script/FortniteGame.AthenaCharacterItemDefinition", false);

					auto AllObjects = Helper::GetAllObjectsOfClass(CIDClass);

					for (int i = 0; i < AllObjects.size(); i++)
					{
						auto CurrentCID = AllObjects.at(i);

						static auto DisplayNameOffset = CurrentCID->GetOffset("DisplayName");
						auto DisplayNameFText = (FText*)(__int64(CurrentCID) + DisplayNameOffset);

						FString DisplayNameFStr = Helper::Conversion::TextToString(*DisplayNameFText);

						if (!DisplayNameFStr.Data.Data)
							continue;

						SkinsFile << std::format("[{}] {}\n", DisplayNameFStr.ToString(), CurrentCID->GetPathName());
					}
				}
			}

			if (ImGui::Button("Dump Playlists (Playlists.txt)"))
			{
				std::ofstream PlaylistsFile("Playlists.txt");

				if (PlaylistsFile.is_open())
				{
					PlaylistsFile << ahh;
					static auto FortPlaylistClass = FindObjectSlow("Class /Script/FortniteGame.FortPlaylist", false);
					// static auto FortPlaylistClass = FindObject("Class /Script/FortniteGame.FortPlaylistAthena");

					auto AllObjects = Helper::GetAllObjectsOfClass(FortPlaylistClass);

					for (int i = 0; i < AllObjects.size(); i++)
					{
						auto Object = AllObjects.at(i);

						// std::string PlaylistName = Object->Member<FName>("PlaylistName")->ToString(); // Short name basically
						static auto UIDisplayNameOffset = Object->GetOffset("UIDisplayName");
						FString PlaylistNameFStr = Helper::Conversion::TextToString(*Get<FText>(Object, UIDisplayNameOffset));

						if (!PlaylistNameFStr.Data.Data)
							continue;

						std::string PlaylistName = PlaylistNameFStr.ToString();

						PlaylistsFile << std::format("[{}] {}\n", PlaylistName, Object->GetPathName());
					}
				}
				else
					std::cout << "Failed to open playlist file!\n";
			}

			if (ImGui::Button("Dump Weapons (Weapons.txt)"))
			{
				std::ofstream WeaponsFile("Weapons.txt");

				if (WeaponsFile.is_open())
				{
					WeaponsFile << ahh;
					static auto FortWeaponItemDefinitionClass = FindObjectSlow("Class /Script/FortniteGame.FortWeaponItemDefinition", false);

					auto AllObjects = Helper::GetAllObjectsOfClass(FortWeaponItemDefinitionClass);

					for (int i = 0; i < AllObjects.size(); i++)
					{
						auto Object = AllObjects.at(i);

						// std::string PlaylistName = Object->Member<FName>("PlaylistName")->ToString(); // Short name basically
						static auto DisplayNameOffset = Object->GetOffset("DisplayName");
						FString ItemDefinitionFStr = Helper::Conversion::TextToString(*Get<FText>(Object, DisplayNameOffset));

						if (!ItemDefinitionFStr.Data.Data)
							continue;

						std::string ItemDefinitionName = ItemDefinitionFStr.ToString();

						// check if it contains gallery or playset?

						WeaponsFile << std::format("[{}] {}\n", ItemDefinitionName, Object->GetPathName());
					}
				}
				else
					std::cout << "Failed to open playlist file!\n";
			}
		}
	}

	else if (PlayerTab != 2435892 && bLoaded)
	{
		auto World = Helper::GetWorld();

		if (World) // Ima die
		{
			static auto NetDriverOffset = World->GetOffset("NetDriver");
			auto NetDriver = *(UObject**)(__int64(World) + NetDriverOffset);

			if (NetDriver)
			{
				static auto ClientConnectionsOffset = NetDriver->GetOffset("ClientConnections");
				auto ClientConnections = (TArray<UObject*>*)(__int64(NetDriver) + ClientConnectionsOffset);

				if (ClientConnections)
				{
					for (int i = 0; i < ClientConnections->Num(); i++)
					{
						auto Connection = ClientConnections->At(i);

						if (!Connection)
							continue;

						static auto Connection_PlayerControllerOffset = Connection->GetOffset("PlayerController");
						auto CurrentController = *(UObject**)(__int64(Connection) + Connection_PlayerControllerOffset);

						if (CurrentController)
						{
							AllControllers.push_back(CurrentController);
						}
					}
				}
			}
		}

		if (PlayerTab < AllControllers.size())
		{
			auto CurrentController = AllControllers.at(PlayerTab);
			auto CurrentPawn = Helper::GetPawnFromController(CurrentController);
			auto CurrentPlayerState = Helper::GetPlayerStateFromController(CurrentController);
	
			if (!bIsEditingInventory)
			{
				FString NameFStr;

				static auto GetPlayerName = FindObject<UFunction>("/Script/Engine.PlayerState.GetPlayerName");
				CurrentPlayerState->ProcessEvent(GetPlayerName, &NameFStr);

				const wchar_t* NameWCStr = NameFStr.Data.Data;
				std::wstring NameWStr = std::wstring(NameWCStr);
				std::string Name = std::string(NameWStr.begin(), NameWStr.end());

				ImGui::Text(("Viewing " + Name).c_str());

				static std::string WID;
				static std::string KickReason = "You have been kicked!";
				static int stud = 0;

				ImGui::InputText("WID To Give", &WID);

				if (CurrentPawn)
				{
					auto CurrentWeapon = Helper::GetCurrentWeapon(CurrentPawn);
					static auto AmmoCountOffset = FindOffsetStruct("Class /Script/FortniteGame.FortWeapon", "AmmoCount");

					auto AmmoCountPtr = (int*)(__int64(CurrentWeapon) + AmmoCountOffset);

					if (ImGui::InputInt("Ammo Count of CurrentWeapon", CurrentWeapon ? AmmoCountPtr : &stud))
					{
						/* if (CurrentWeapon)
						{
							FFortItemEntry::SetLoadedAmmo(Inventory::GetEntryFromWeapon(CurrentController, CurrentWeapon), CurrentController, *AmmoCountPtr);
						} */
					}
				}

				if (ImGui::Button("Spawn Llama"))
				{
					if (CurrentPawn)
					{
						static auto LlamaClass = FindObject("/Game/Athena/SupplyDrops/Llama/AthenaSupplyDrop_Llama.AthenaSupplyDrop_Llama_C");

						std::cout << "LlamaClass: " << LlamaClass << '\n';

						if (LlamaClass)
						{
							auto Llama = Helper::Easy::SpawnActor(LlamaClass, Helper::GetActorLocation(CurrentPawn));
						}
					}
				}

				if (ImGui::Button("Give Item"))
				{
					if (!WID.empty())
					{
						std::string cpywid;

						if (WID.find(".") == std::string::npos)
							cpywid = std::format("{}.{}", WID, WID);

						auto wid = FindObjectSlow(cpywid);

						if (wid)
							Inventory::GiveItem(CurrentController, wid, Inventory::WhatQuickBars(wid), 1);
						else
							std::cout << "Unable to find WID!\n";
					}
				}

				ImGui::InputText("Kick Reason", &KickReason);

				if (ImGui::Button("Kick"))
				{
					std::wstring wstr = std::wstring(KickReason.begin(), KickReason.end());
					FString Reason;
					Reason.Set(wstr.c_str());

					static auto ClientReturnToMainMenu = FindObject<UFunction>("/Script/Engine.PlayerController.ClientReturnToMainMenu");
					CurrentController->ProcessEvent(ClientReturnToMainMenu, &Reason);
				}

				if (ImGui::Button("Edit Inventory"))
				{
					bIsEditingInventory = true;
				}
			}
			else
			{
				if (false)
				{
					std::vector<UObject*> PrimaryQuickbarInstances;

					auto ItemInstances = Inventory::GetItemInstances(CurrentController);

					for (int i = 0; i < ItemInstances->Num(); i++)
					{
						auto ItemInstance = ItemInstances->At(i);

						if (!ItemInstance)
							continue;

						auto Def = *UFortItem::GetDefinition(ItemInstance);

						if (Def && Inventory::WhatQuickBars(Def) == EFortQuickBars::Primary)
						{
							PrimaryQuickbarInstances.push_back(ItemInstance);

							static auto DisplayNameOffset = Def->GetOffset("DisplayName");
							auto DisplayNameFText = Get<FText>(Def, DisplayNameOffset);
							auto DisplayNameFStr = Helper::Conversion::TextToString(*DisplayNameFText);

							if (!DisplayNameFStr.Data.Data)
								continue;

							auto DisplayNameStr = DisplayNameFStr.ToString();

							auto Count = *UFortItem::GetCount(ItemInstance);

							TextCentered(std::format("{} {}", Count, DisplayNameStr));
						}
					}
				}

				if (ImGui::Button("Back"))
				{
					bIsEditingInventory = false;
				}
			}
		}
	}
}

void PregameUI()
{
	ImGui::Text(std::format("You will only be able to set these for around {} more second{}!", Defines::SecondsUntilTravel + 6, Defines::SecondsUntilTravel + 6 <= 1 ? "" : "s").c_str());

	ImGui::NewLine();

	if (!Defines::bTraveled)
		ImGui::SliderInt("Seconds until load into game", &Defines::SecondsUntilTravel, 1, 30);

	ImGui::InputText("Map Name", &Defines::MapName);

	if (!Defines::bIsPlayground && !Defines::bIsGoingToPlayMainEvent)
		ImGui::InputText("Playlist", &Defines::Playlist);

	if (ImGui::Checkbox("Going to play event", &Defines::bIsGoingToPlayMainEvent))
	{
		Defines::Playlist = Defines::bIsGoingToPlayMainEvent ? Events::GetEventPlaylistName() :
			Defines::bIsPlayground ? "/Game/Athena/Playlists/Playground/Playlist_Playground.Playlist_Playground"
			: "/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo";
	}

	if (ImGui::Checkbox("Playground", &Defines::bIsPlayground))
	{
		// Defines::bWipeInventoryOnAircraft = Defines::bIsPlayground; // even if its playground it still clears
		Defines::Playlist = Defines::bIsPlayground ? "/Game/Athena/Playlists/Playground/Playlist_Playground.Playlist_Playground"
			: "/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo";
	}
}

DWORD WINAPI GuiThread(LPVOID)
{
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"RebootClass", NULL };
	::RegisterClassEx(&wc);
	HWND hwnd = ::CreateWindowExW(0L, wc.lpszClassName, L"Project Reboot", (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX), 100, 100, Width, Height, NULL, NULL, wc.hInstance, NULL);

	HANDLE hIcon = LoadImageW(wc.hInstance, L"Reboot Resources/images/reboot.ico", IMAGE_ICON, 48, 48, LR_LOADFROMFILE);
	SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

	// SetWindowLongPtrW(hwnd, GWL_STYLE, WS_POPUP); // Disables windows title bar at the cost of dragging and some quality

	// Initialize Direct3D
	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		::UnregisterClass(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	io.IniFilename = NULL; // Disable imgui.ini generation.
	io.DisplaySize = ImGui::GetMainViewport()->Size;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	// io.Fonts->AddFontFromFileTTF("../vendor/fonts/Aller_Bd.ttf", 17);

	// Setup Dear ImGui style
	InitStyle();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX9_Init(g_pd3dDevice);

	// Our state
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	ImFontConfig config;
	config.MergeMode = true;
	config.GlyphMinAdvanceX = 13.0f; // Use if you want to make the icon monospaced
	static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	io.Fonts->AddFontFromFileTTF("Reboot Resources/fonts/fontawesome-webfont.ttf", 13.0f, &config, icon_ranges);

	// Main loop
	bool done = false;
	while (!done)
	{
		MSG msg;
		while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
			{
				done = true;
				break;
			}
		}

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		auto WindowSize = ImGui::GetMainViewport()->Size;
		// ImGui::SetNextWindowPos(ImVec2(WindowSize.x * 0.5f, WindowSize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f)); // Center
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);

		tagRECT rect;

		if (GetWindowRect(hwnd, &rect))
		{
			int width = rect.right - rect.left;
			int height = rect.bottom - rect.top;
			ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
		}

		if (!ImGui::IsWindowCollapsed())
		{
			ImGui::Begin(("Project Reboot"), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

			!Defines::bReadyForStartMatch ? MainUI() : PregameUI();

			ImGui::End();
		}

		// Rendering
		ImGui::EndFrame();
		g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f), (int)(clear_color.y * clear_color.w * 255.0f), (int)(clear_color.z * clear_color.w * 255.0f), (int)(clear_color.w * 255.0f));
		g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);

		if (g_pd3dDevice->BeginScene() >= 0)
		{
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			g_pd3dDevice->EndScene();
		}

		HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

		// Handle loss of D3D9 device
		if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
			ResetDevice();
	}

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	::DestroyWindow(hwnd);
	::UnregisterClass(wc.lpszClassName, wc.hInstance);

	return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
	if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
		return false;

	// Create the D3DDevice
	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
	g_d3dpp.Windowed = TRUE;
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
	g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
	//g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
	if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
		return false;

	return true;
}

void CleanupDeviceD3D()
{
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
	if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void ResetDevice()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
	if (hr == D3DERR_INVALIDCALL)
		IM_ASSERT(0);
	ImGui_ImplDX9_CreateDeviceObjects();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// my implementation of window dragging..
	/* {
		static int dababy = 0;
		if (dababy > 100) // wait until gui is initialized ig?
		{
			if (ImGui::IsMouseDragging(ImGuiMouseButton(0)))
			{
				// if (LOWORD(lParam) > 255 && HIWORD(lParam) > 255)
				{
					POINT p;
					GetCursorPos(&p);

					SetWindowPos(hWnd, nullptr, p.x, p.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				}
			}
		}
		dababy++;
	} */

	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			g_d3dpp.BackBufferWidth = LOWORD(lParam);
			g_d3dpp.BackBufferHeight = HIWORD(lParam);
			ResetDevice();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}