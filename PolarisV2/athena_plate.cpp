#include "athena_plate.h"
#include "globals.h"
#include "error_utils.h"
#include "sdk_utils.h"
#include "program.h"
#include <MinHook.h>
#include <vector>

PVOID(*CollectGarbageInternal)(uint32_t, bool) = nullptr;
PVOID CollectGarbageInternalHook(uint32_t KeepFlags, bool bPerformFullPurge)
{
    return NULL;
}
struct AFortAsQuickBars
{
public:
    unsigned char                                      UnknownData00[0x1A88];
    class SDK::AFortQuickBars* QuickBars;
};
struct AFortAsBuildPreview
{
public:
    unsigned char UnknownData00[0x1788];
    class SDK::ABuildingPlayerPrimitivePreview* BuildPreviewMarker;
};
struct AFortAsCurrentBuildable
{
public:
    unsigned char UnknownData00[0x1940];
    class SDK::UClass* CurrentBuildableClass;
};
SDK::FGuid pgPickaxe;
SDK::FGuid pgTrap;
SDK::FGuid pgTrap2;
SDK::FGuid pgWallBuild;
SDK::FGuid pgFloorBuild;
SDK::FGuid pgStairBuild;
SDK::FGuid pgRoofBuild;
SDK::UFortWeaponMeleeItemDefinition* pPickaxeDef;
SDK::UFortBuildingItemDefinition* pWallBuildDef;
SDK::UFortBuildingItemDefinition* pFloorBuildDef;
SDK::UFortBuildingItemDefinition* pStairBuildDef;
SDK::UFortBuildingItemDefinition* pRoofBuildDef;
SDK::UFortDecoItemDefinition* pTrapDef;
SDK::UFortTrapItemDefinition* pTrapDef2;
SDK::ABuildingActor* pTrapC;
bool bTrapDone = false;
namespace polaris
{
    namespace tables
    {
        namespace plates
        {
            void AthenaPlate::ProcessEventHook(SDK::UObject* pObject, SDK::UFunction* pFunction, PVOID pParams)
            {
                if (m_pPlayerPawn != nullptr)
                    m_pPlayerPawn->ProcessEventHook(pObject, pFunction, pParams);

                if (pFunction->GetName().find("ReadyToStartMatch") != std::string::npos && m_bIsInitialized == false)
                    Initialize();

                if (pFunction->GetName().find("ServerAttemptAircraftJump") != std::string::npos ||
                    (pFunction->GetName().find("AircraftExitedDropZone") != std::string::npos && static_cast<SDK::AFortPlayerControllerAthena*>(globals::gpPlayerController)->IsInAircraft()))
                {
                    if (pFunction->GetName().find("ServerAttemptAircraftJump") != std::string::npos)
                        m_pPlayerPawn->~AthenaPawn();

                    m_pPlayerPawn = new pawn::pawns::AthenaPawn;
                }

                if (pFunction->GetName().find("ServerLoadingScreenDropped") != std::string::npos)
                    SetupQuickbars();
                if (pFunction->GetName().find("ServerExecuteInventoryItem") != std::string::npos) {
                    //printf(" %s, %s, %s, %s\n", /*pObject->GetFullName().c_str(),*/ std::to_string(reinterpret_cast<SDK::FGuid*>(pParams)->A), std::to_string(reinterpret_cast<SDK::FGuid*>(pParams)->B), std::to_string(reinterpret_cast<SDK::FGuid*>(pParams)->C), std::to_string(reinterpret_cast<SDK::FGuid*>(pParams)->D));
                    SDK::FGuid guid;
                    SDK::FGuid* paramGuid = reinterpret_cast<SDK::FGuid*>(pParams);
                    guid.A = paramGuid->A;
                    guid.B = paramGuid->B;
                    guid.C = paramGuid->C;
                    guid.D = paramGuid->D;
                    if (AreGuidsTheSame(guid, pgPickaxe)) {
                        m_pPlayerPawn->m_pPawnActor->EquipWeaponDefinition(pPickaxeDef,guid);
                    }
                    if (AreGuidsTheSame(guid, pgWallBuild)) {
                        m_pPlayerPawn->m_pPawnActor->EquipWeaponDefinition(pWallBuildDef, guid);
                        reinterpret_cast<AFortAsCurrentBuildable*>(globals::gpPlayerController)->CurrentBuildableClass = SDK::APBWA_W1_Solid_C::StaticClass();
                        reinterpret_cast<AFortAsBuildPreview*>(globals::gpPlayerController)->BuildPreviewMarker = m_pPlayerPawn->pBuildPreviewWall;
                        m_pPlayerPawn->pBuildPreviewWall->SetActorHiddenInGame(false);
                        m_pPlayerPawn->pBuildPreviewFloor->SetActorHiddenInGame(true);
                        m_pPlayerPawn->pBuildPreviewStair->SetActorHiddenInGame(true);
                        m_pPlayerPawn->pBuildPreviewRoof->SetActorHiddenInGame(true);
                    }
                    if (AreGuidsTheSame(guid, pgFloorBuild)) {
                        m_pPlayerPawn->m_pPawnActor->EquipWeaponDefinition(pFloorBuildDef, guid);
                        reinterpret_cast<AFortAsCurrentBuildable*>(globals::gpPlayerController)->CurrentBuildableClass = SDK::APBWA_W1_Floor_C::StaticClass();
                        reinterpret_cast<AFortAsBuildPreview*>(globals::gpPlayerController)->BuildPreviewMarker = m_pPlayerPawn->pBuildPreviewFloor;
                        m_pPlayerPawn->pBuildPreviewWall->SetActorHiddenInGame(true);
                        m_pPlayerPawn->pBuildPreviewFloor->SetActorHiddenInGame(false);
                        m_pPlayerPawn->pBuildPreviewStair->SetActorHiddenInGame(true);
                        m_pPlayerPawn->pBuildPreviewRoof->SetActorHiddenInGame(true);
                    }
                    if (AreGuidsTheSame(guid, pgStairBuild)) {
                        m_pPlayerPawn->m_pPawnActor->EquipWeaponDefinition(pStairBuildDef, guid);
                        reinterpret_cast<AFortAsCurrentBuildable*>(globals::gpPlayerController)->CurrentBuildableClass = SDK::APBWA_W1_StairW_C::StaticClass();
                        reinterpret_cast<AFortAsBuildPreview*>(globals::gpPlayerController)->BuildPreviewMarker = m_pPlayerPawn->pBuildPreviewStair;
                        m_pPlayerPawn->pBuildPreviewWall->SetActorHiddenInGame(true);
                        m_pPlayerPawn->pBuildPreviewFloor->SetActorHiddenInGame(true);
                        m_pPlayerPawn->pBuildPreviewStair->SetActorHiddenInGame(false);
                        m_pPlayerPawn->pBuildPreviewRoof->SetActorHiddenInGame(true);
                    }
                    if (AreGuidsTheSame(guid, pgRoofBuild)) {
                        m_pPlayerPawn->m_pPawnActor->EquipWeaponDefinition(pRoofBuildDef, guid);
                        reinterpret_cast<AFortAsCurrentBuildable*>(globals::gpPlayerController)->CurrentBuildableClass = SDK::APBWA_W1_RoofC_C::StaticClass();
                        reinterpret_cast<AFortAsBuildPreview*>(globals::gpPlayerController)->BuildPreviewMarker = m_pPlayerPawn->pBuildPreviewRoof;
                        m_pPlayerPawn->pBuildPreviewWall->SetActorHiddenInGame(true);
                        m_pPlayerPawn->pBuildPreviewFloor->SetActorHiddenInGame(true);
                        m_pPlayerPawn->pBuildPreviewStair->SetActorHiddenInGame(true);
                        m_pPlayerPawn->pBuildPreviewRoof->SetActorHiddenInGame(false);
                    }
                    if (AreGuidsTheSame(guid, pgTrap)) {
                        if (bTrapDone == false) {
                            globals::gpPlayerController->CheatManager->Summon(TEXT("Trap_Floor_Spikes_C"));
                            pTrapC = SDK::UObject::FindObject<SDK::ABuildingActor>("Trap_Floor_Player_Jump_Free_Direction_Pad_C Athena_Terrain.Athena_Terrain.PersistentLevel.Trap_Floor_Player_Jump_Free_Direction_Pad_C_1");
                            bTrapDone = true;
                        }
                        m_pPlayerPawn->m_pPawnActor->PickUpActor(pTrapC, pTrapDef);
                        m_pPlayerPawn->m_pPawnActor->CurrentWeapon->ItemEntryGuid = pgTrap;
                    }
                    //if (AreGuidsTheSame(guid, pgTrap2)) {
                    //    if (bTrapDone == false) {
                    //        globals::gpPlayerController->CheatManager->Summon(TEXT("Trap_Floor_Spikes_C"));
                    //        pTrapC = SDK::UObject::FindObject<SDK::ABuildingActor>("Trap_Floor_Player_Jump_Free_Direction_Pad_C Athena_Terrain.Athena_Terrain.PersistentLevel.Trap_Floor_Player_Jump_Free_Direction_Pad_C_1");
                    //        bTrapDone = true;
                    //    }
                    //    m_pPlayerPawn->m_pPawnActor->PickUpActor(pTrapC, pTrapDef2);
                    //    m_pPlayerPawn->m_pPawnActor->CurrentWeapon->ItemEntryGuid = pgTrap2;
                    //}
                }
            }
            void AthenaPlate::Update()
            {
                if (m_pPlayerPawn != nullptr)
                    m_pPlayerPawn->Update();

                // TEMP: Go back to Frontend.
                if (GetAsyncKeyState(VK_OEM_PLUS) & 0x8000)
                    gpProgram->m_pMainTable->PopPlate();
            }

            void AthenaPlate::OnEnabled()
            {
                utilities::SDKUtils::InitSdk();
                utilities::SDKUtils::InitGlobals();

                globals::gpPlayerController->SwitchLevel(TEXT("Athena_Terrain"));
            }

            void AthenaPlate::Initialize()
            {
                //Initialize the SDK again.
                m_bIsInitialized = true;
                utilities::SDKUtils::InitSdk();
                utilities::SDKUtils::InitGlobals();
                utilities::SDKUtils::InitPatches();

                // Disable garbage collection.
                auto pCollectGarbageInternalAddress = utilities::SDKUtils::FindPattern("\x48\x8B\xC4\x48\x89\x58\x08\x88\x50\x10", "xxxxxxxxxx");
                if (!pCollectGarbageInternalAddress)
                    utilities::ErrorUtils::ThrowException(L"Finding pattern for CollectGarbageInternal has failed. Please relaunch Fortnite and try again!");

                MH_CreateHook(static_cast<LPVOID>(pCollectGarbageInternalAddress), CollectGarbageInternalHook, reinterpret_cast<LPVOID*>(&CollectGarbageInternal));
                MH_EnableHook(static_cast<LPVOID>(pCollectGarbageInternalAddress));

                // Spawn a Player Pawn and setup inventory.
                m_pPlayerPawn = new pawn::pawns::AthenaPawn;
                SetupInventory();

                // Tell the client that we are ready to start the match, this allows the loading screen to drop.
                static_cast<SDK::AFortPlayerController*>(globals::gpPlayerController)->ServerReadyToStartMatch();
                static_cast<SDK::AGameMode*>((*globals::gpWorld)->AuthorityGameMode)->StartMatch();
            }
            bool AthenaPlate::AreGuidsTheSame(SDK::FGuid guidA, SDK::FGuid guidB)
            {
                if (guidA.A == guidB.A && guidA.B == guidB.B && guidA.C == guidB.C && guidA.D == guidB.D) 
                    return true;
                else
                    return false;
            }
            SDK::FGuid AthenaPlate::GenerateGuid()
            {
                SDK::FGuid Guid;
                Guid.A = rand() % 100;
                Guid.B = rand() % 100;
                Guid.C = rand() % 100;
                Guid.D = rand() % 100;
                return Guid;
            }
            void AthenaPlate::SetupQuickbars()
            {
                polaris::globals::gpPlayerController->CheatManager->Summon(TEXT("FortQuickBars"));
                reinterpret_cast<AFortAsQuickBars*>(polaris::globals::gpPlayerController)->QuickBars = static_cast<SDK::AFortQuickBars*>(utilities::SDKUtils::FindActor(SDK::AFortQuickBars::StaticClass()));
                reinterpret_cast<AFortAsQuickBars*>(polaris::globals::gpPlayerController)->QuickBars->SetOwner(polaris::globals::gpPlayerController);
                auto pQuickBars = static_cast<SDK::AFortQuickBars*>(utilities::SDKUtils::FindActor(SDK::AFortQuickBars::StaticClass()));
                static_cast<SDK::AFortPlayerControllerAthena*>(polaris::globals::gpPlayerController)->Role = SDK::ENetRole::ROLE_None;
                static_cast<SDK::AFortPlayerControllerAthena*>(polaris::globals::gpPlayerController)->OnRep_QuickBar();
                static_cast<SDK::AFortPlayerControllerAthena*>(polaris::globals::gpPlayerController)->Role = SDK::ENetRole::ROLE_Authority;
                reinterpret_cast<AFortAsQuickBars*>(polaris::globals::gpPlayerController)->QuickBars->EnableSlot(SDK::EFortQuickBars::Secondary, 0);
                reinterpret_cast<AFortAsQuickBars*>(polaris::globals::gpPlayerController)->QuickBars->EnableSlot(SDK::EFortQuickBars::Secondary, 1);
                reinterpret_cast<AFortAsQuickBars*>(polaris::globals::gpPlayerController)->QuickBars->EnableSlot(SDK::EFortQuickBars::Secondary, 2);
                reinterpret_cast<AFortAsQuickBars*>(polaris::globals::gpPlayerController)->QuickBars->EnableSlot(SDK::EFortQuickBars::Secondary, 3);
                reinterpret_cast<AFortAsQuickBars*>(polaris::globals::gpPlayerController)->QuickBars->EnableSlot(SDK::EFortQuickBars::Secondary, 4);
                reinterpret_cast<AFortAsQuickBars*>(polaris::globals::gpPlayerController)->QuickBars->EnableSlot(SDK::EFortQuickBars::Secondary, 5);
                reinterpret_cast<AFortAsQuickBars*>(polaris::globals::gpPlayerController)->QuickBars->EnableSlot(SDK::EFortQuickBars::Primary, 0);
                pQuickBars->ServerAddItemInternal(pgWallBuild, SDK::EFortQuickBars::Secondary, 0);
                pQuickBars->ServerAddItemInternal(pgFloorBuild, SDK::EFortQuickBars::Secondary, 1);
                pQuickBars->ServerAddItemInternal(pgStairBuild, SDK::EFortQuickBars::Secondary, 2);
                pQuickBars->ServerAddItemInternal(pgRoofBuild, SDK::EFortQuickBars::Secondary, 3);
                pQuickBars->ServerAddItemInternal(pgTrap2, SDK::EFortQuickBars::Secondary, 4);
                pQuickBars->ServerAddItemInternal(pgPickaxe, SDK::EFortQuickBars::Primary, 0);
            }
            void AthenaPlate::SetupInventory()
            {
                //Defining Item Definitions
                pTrapDef = SDK::UObject::FindObject<SDK::UFortDecoItemDefinition>("FortTrapItemDefinition TID_Floor_Spikes_Athena_R_T03.TID_Floor_Spikes_Athena_R_T03");
                pTrapDef2 = SDK::UObject::FindObject<SDK::UFortTrapItemDefinition>("FortTrapItemDefinition TID_Wall_Electric_Athena_R_T03.TID_Wall_Electric_Athena_R_T03");
                pPickaxeDef = SDK::UObject::FindObject<SDK::UFortWeaponMeleeItemDefinition>(m_pPlayerPawn->mPickaxeAsWid[static_cast<SDK::AFortPlayerPawnAthena*>(m_pPlayerPawn->m_pPawnActor)->CustomizationLoadout.Character->GetName()].c_str());
                pWallBuildDef = SDK::UObject::FindObject<SDK::UFortBuildingItemDefinition>("FortBuildingItemDefinition BuildingItemData_Wall.BuildingItemData_Wall");
                pFloorBuildDef = SDK::UObject::FindObject<SDK::UFortBuildingItemDefinition>("FortBuildingItemDefinition BuildingItemData_Floor.BuildingItemData_Floor");
                pStairBuildDef = SDK::UObject::FindObject<SDK::UFortBuildingItemDefinition>("FortBuildingItemDefinition BuildingItemData_Stair_W.BuildingItemData_Stair_W");
                pRoofBuildDef = SDK::UObject::FindObject<SDK::UFortBuildingItemDefinition>("FortBuildingItemDefinition BuildingItemData_RoofS.BuildingItemData_RoofS");
                auto pWood = SDK::UObject::FindObject<SDK::UFortItemDefinition>("FortResourceItemDefinition WoodItemData.WoodItemData");
                auto pMetal = SDK::UObject::FindObject<SDK::UFortItemDefinition>("FortResourceItemDefinition MetalItemData.MetalItemData");
                auto pStone = SDK::UObject::FindObject<SDK::UFortItemDefinition>("FortResourceItemDefinition StoneItemData.StoneItemData");
                auto pLight = SDK::UObject::FindObject<SDK::UFortItemDefinition>("FortAmmoItemDefinition AthenaAmmoDataBulletsLight.AthenaAmmoDataBulletsLight");
                auto pMed = SDK::UObject::FindObject<SDK::UFortItemDefinition>("FortAmmoItemDefinition AthenaAmmoDataBulletsMedium.AthenaAmmoDataBulletsMedium");
                auto pShot = SDK::UObject::FindObject<SDK::UFortItemDefinition>("FortAmmoItemDefinition AthenaAmmoDataShells.AthenaAmmoDataShells");
                auto pHeavy = SDK::UObject::FindObject<SDK::UFortItemDefinition>("FortAmmoItemDefinition AthenaAmmoDataBulletsHeavy.AthenaAmmoDataBulletsHeavy");
                // funny item array
                SDK::UFortItemDefinition* aItemDefs[13] = {};
                SDK::FGuid aGuids[13] = {};
                aItemDefs[0] = pTrapDef;
                aItemDefs[1] = pPickaxeDef;
                aItemDefs[2] = pWallBuildDef;
                aItemDefs[3] = pFloorBuildDef;
                aItemDefs[4] = pStairBuildDef;
                aItemDefs[5] = pRoofBuildDef;
                aItemDefs[6] = pWood;
                aItemDefs[7] = pMetal;
                aItemDefs[8] = pStone;
                aItemDefs[9] = pLight;
                aItemDefs[10] = pMed;
                aItemDefs[11] = pShot;
                aItemDefs[12] = pHeavy;
                aItemDefs[13] = pTrapDef2;
                aGuids[0] = GenerateGuid();
                aGuids[1] = GenerateGuid();
                aGuids[2] = GenerateGuid();
                aGuids[3] = GenerateGuid();
                aGuids[4] = GenerateGuid();
                aGuids[5] = GenerateGuid();
                aGuids[6] = GenerateGuid();
                aGuids[7] = GenerateGuid();
                aGuids[8] = GenerateGuid();
                aGuids[9] = GenerateGuid();
                aGuids[10] = GenerateGuid();
                aGuids[11] = GenerateGuid();
                aGuids[12] = GenerateGuid();
                aGuids[13] = GenerateGuid();
                pgPickaxe = aGuids[1];
                pgTrap = aGuids[0];
                pgWallBuild = aGuids[2];
                pgFloorBuild = aGuids[3];
                pgStairBuild = aGuids[4];
                pgRoofBuild = aGuids[5];
                pgTrap2 = aGuids[13];
                auto pInventory1 = static_cast<SDK::AFortInventory*>(polaris::utilities::SDKUtils::FindActor(SDK::AFortInventory::StaticClass()));
                auto pInventory2 = static_cast<SDK::AFortInventory*>(polaris::utilities::SDKUtils::FindActor(SDK::AFortInventory::StaticClass(), 1));
                SDK::AFortInventory* pActualInv = nullptr;

                // joe nuts
                if (pInventory1 && pInventory1->InventoryType == SDK::EFortInventoryType::World)
                {
                    pActualInv = pInventory1;
                }

                if (pInventory2 && pInventory2->InventoryType == SDK::EFortInventoryType::World)
                {
                    pActualInv = pInventory2;
                }

                if (pActualInv)
                {
                    //gaming rgb inventory
                    auto myInventory = pActualInv;
                    SDK::FFortItemList* inv = &myInventory->Inventory;
                    SDK::TArray<class SDK::UFortWorldItem*>* pItemInsts = &inv->ItemInstances;
                    pItemInsts->Count = 14;
                    pItemInsts->Max = 14;
                    pItemInsts->Data = (class SDK::UFortWorldItem**)::malloc(pItemInsts->Count * sizeof(SDK::UFortWorldItem*));
                    //for (int i = 0; SDK::UFortItemDefinition* pItemdef = aItemDefs[i]; /*i < 13*/) {
                    for (int i = 0; i < 14; i++) {
                        auto pItemdef = aItemDefs[i];
                        printf(std::to_string(i).c_str());
                        auto pItemEntry = new SDK::FFortItemEntry;
                        switch (i) {
                        default:
                            pItemEntry->Count = 100;
                            break;
                        case 0:
                            pItemEntry->Count = 1;
                            break;
                        case 1:
                            pItemEntry->Count = 1;
                            break;
                        case 2:
                            pItemEntry->Count = 1;
                            break;
                        case 3:
                            pItemEntry->Count = 1;
                            break;
                        case 4:
                            pItemEntry->Count = 1;
                            break;
                        case 5:
                            pItemEntry->Count = 1;
                            break;
                        case 13:
                            pItemEntry->Count = 1;
                            break;
                        }
                        pItemEntry->ItemDefinition = pItemdef;
                        pItemEntry->Durability = 1000;
                        pItemEntry->Level = 1;
                        pItemEntry->ItemGuid = aGuids[i];
                        pItemEntry->bIsDirty = false;
                        auto pWorldItem = reinterpret_cast<SDK::UFortWorldItem*>(polaris::globals::StaticConstructObject_Internal(SDK::UFortWorldItem::StaticClass(), myInventory, SDK::FName("None"), 0, SDK::FUObjectItem::ObjectFlags::None, NULL, false, NULL, false));
                        pWorldItem->OwnerInventory = myInventory;
                        pWorldItem->ItemEntry = *pItemEntry;
                        pItemInsts->operator[](i) = pWorldItem;
                    }
                }
            }
        }
    }
}