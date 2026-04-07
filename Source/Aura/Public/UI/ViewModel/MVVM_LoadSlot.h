// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "Game/LoadScreenSaveGame.h"
#include "MVVM_LoadSlot.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetWidgetSwitcherIndex, int32, WidgetSwitcherIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnableSelectSlotButton, bool, bEnable);

/**
 * 本VM的作用主要在于存储存档的相关信息，而不处理UI方面具体的逻辑，具体功能调用也全由LoadScreen及其VM控制。
 */
UCLASS()
class AURA_API UMVVM_LoadSlot : public UMVVMViewModelBase
{
	GENERATED_BODY()
public:

	//当Switcher需要切换显示的WBP时触发该委托，广播要切换的WBP索引，自然于Switcher的BIW函数中绑定回调函数
	UPROPERTY(BlueprintAssignable)
	FSetWidgetSwitcherIndex SetWidgetSwitcherIndex;

	UPROPERTY(BlueprintAssignable)
	FEnableSelectSlotButton EnableSelectSlotButton;

	//根据SlotStatus切换WBP
	void InitializeSlot();

	//该Slot的索引，012取其一
	UPROPERTY()
	int32 SlotIndex;

	//记录下一次要跳转的WBP索引
	UPROPERTY()
	TEnumAsByte<ESaveSlotStatus> SlotStatus;

	 //玩家位置
	UPROPERTY()
	FName PlayerStartTag;

	//地图资产名
	UPROPERTY()
	FString MapAssetName;
	
	/** Field Notifies
	 * 当 ViewModel 中的属性值发生变化时，通知所有绑定到该属性的 UI 控件进行更新。
	 */
	void SetPlayerName(FString InPlayerName);
	void SetMapName(FString InMapName);
	void SetPlayerLevel(int32 InLevel);
	void SetLoadSlotName(FString InLoadSlotName);

	FString GetPlayerName() const { return PlayerName; }
	FString GetMapName() const { return MapName; }
	int32 GetPlayerLevel() const { return PlayerLevel; }
	FString GetLoadSlotName() const { return LoadSlotName; }

private:
	//Getter：规定“UI/绑定系统读这个属性时，走哪个函数”
	//Setter：规定“UI/绑定系统写这个属性时，走哪个函数”
	//FieldNotify：规定“值变了以后，自动通知界面刷新”
	//玩家名
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess="true"));
	FString PlayerName = "Aura";
	//地图名
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess="true"));
	FString MapName;
	//玩家等级
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess="true"));
	int32 PlayerLevel;
	//存档名（即LoadSlot_0-LoadSlot_2）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess="true"));
	FString LoadSlotName;
};
