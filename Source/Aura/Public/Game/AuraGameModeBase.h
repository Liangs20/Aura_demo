// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AuraGameModeBase.generated.h"

class ULootTiers;
class ULoadScreenSaveGame;
class USaveGame;
class UMVVM_LoadSlot;
class UAbilityInfo;
class UCharacterClassInfo;
/**
 * 
 */
UCLASS()
class AURA_API AAuraGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	//存储敌人三种不同类型及其对应的初始化GE
	UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
	TObjectPtr<UCharacterClassInfo> CharacterClassInfo;

	//存储Aura的各种技能信息的数据资产
	UPROPERTY(EditDefaultsOnly, Category = "Ability Info")
	TObjectPtr<UAbilityInfo> AbilityInfo;

	// 存储怪物掉落物品概率与数量配置的数据资产
	UPROPERTY(EditDefaultsOnly, Category = "Loot Tiers")
	TObjectPtr<ULootTiers> LootTiers;

	//基于UGameplayStatics进行存档的保存，获取，删除
	void SaveSlotData(UMVVM_LoadSlot* LoadSlot, int32 SlotIndex);
	ULoadScreenSaveGame* GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const;
	static void DeleteSlot(const FString& SlotName, int32 SlotIndex);
	
	// 根据当前游戏实例中记录的槽位名和槽位索引，读取并返回正在游玩的存档的SaveGame数据
	ULoadScreenSaveGame* RetrieveInGameSaveData();
	// 将游戏内最新的进度数据写回当前正在使用的存档槽位
	void SaveInGameProgressData(ULoadScreenSaveGame* SaveObject);

	// 将当前世界中实现了SaveInterface的Actor状态序列化到当前使用的存档中。
	// 如果传入DestinationMapAssetName，还会同步更新存档记录的目标地图信息。
	void SaveWorldState(UWorld* World, const FString& DestinationMapAssetName = FString("")) const;
	// 从当前使用的存档中读取当前世界的状态，
	// 并恢复本地图内实现了SaveInterface的Actor序列化数据。
	void LoadWorldState(UWorld* World) const;

	//
	void TravelToMap(UMVVM_LoadSlot* Slot);

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<USaveGame> LoadScreenSaveGameClass;
	
	UPROPERTY(EditDefaultsOnly)
	FString DefaultMapName;
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UWorld> DefaultMap;

	UPROPERTY(EditDefaultsOnly)
	FName DefaultPlayerStartTag;

	//默认地图名和默认关卡会在BeginPlay的时候加入Maps映射，其余地图映射在蓝图中手动添加
	//注意，这里的字符串键并不是地图资产名，而是类似于“地牢一层”这种自定义名
	UPROPERTY(EditDefaultsOnly)
	TMap<FString, TSoftObjectPtr<UWorld>> Maps;

	FString GetMapNameFromMapAssetName(const FString& MapAssetName) const;

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	void PlayerDied(ACharacter* DeadCharacter);
protected:
	virtual void BeginPlay() override;
	
};
