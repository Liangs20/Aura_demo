// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "MVVM_LoadScreen.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSlotSelected, bool, IsSelected);

class UMVVM_LoadSlot;
/**
 * 需引入插件：UMG ViewModel
 *
 * 视图模型类，LoadScreen类中存有对应视图模型的类类型以及对象引用
 * 加入插件后，可以在控件组件的设计器界面打开视图模型窗口新建视图模型变量，图表界面则不行
 * LoadScreen控件的蓝图中定义了一个函数通过玩家控制器找到HUD进而找到视图模型
 * 这个函数在LoadScreen总控件的视图模型界面中通过PropertyPath调用并设置（这种方式下，自定义的函数必须是const）
 *
 * 项目中，本视图模型存在唯一对象在HUD的BeginPlay中创建，在LoadScreen大控件中存在引用，在各个Slot控件中也存在引用，因此要处理所有按钮的逻辑
 */
UCLASS()
class AURA_API UMVVM_LoadScreen : public UMVVMViewModelBase
{
	GENERATED_BODY()
public:

	//初始化所有LoadSlot的VM，调用于HUD创建本类的对象后（HUD的BeginPlay中创建本对象和总Widget）
	void InitializeLoadSlots();

	//选中存档后出发的委托
	UPROPERTY(BlueprintAssignable)
	FSlotSelected SlotSelected;

	//存有存档操作相关的VM的类类型
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMVVM_LoadSlot> LoadSlotViewModelClass;

	//存档VM属于私有变量，提供访问方法
	UFUNCTION(BlueprintPure)
	UMVVM_LoadSlot* GetLoadSlotViewModelByIndex(int32 Index) const;

	//slot的VM于WBP对应，因此在按钮触发以下回调函数时只需广播Slot索引是三个存档中的哪一个即可（0,1,2）
	//确认新存档的按钮，在输入存档名后按下（WBP_EnterName）
	UFUNCTION(BlueprintCallable)
	void NewSlotButtonPressed(int32 Slot, const FString& EnteredName);
	//新建存档按钮(WBP_Vacant)
	UFUNCTION(BlueprintCallable)
	void NewGameButtonPressed(int32 Slot);
	//选择存档按钮(WBP_Taken)
	UFUNCTION(BlueprintCallable)
	void SelectSlotButtonPressed(int32 Slot);
	//三个存档下方，删除存档按钮
	UFUNCTION(BlueprintCallable)
	void DeleteButtonPressed();
	//三个存档下方，开始按钮
	UFUNCTION(BlueprintCallable)
	void PlayButtonPressed();
	//退出按钮的逻辑直接写在BP_LoadScreen中

	void LoadData();

	void SetNumLoadSlots(int32 InNumLoadSlots);

	int32 GetNumLoadSlots() const { return NumLoadSlots; }
	
private:

	UPROPERTY()
	TMap<int32, UMVVM_LoadSlot*> LoadSlots;

	//各存档对应的SlotVM
	UPROPERTY()
	TObjectPtr<UMVVM_LoadSlot> LoadSlot_0;
	UPROPERTY()
	TObjectPtr<UMVVM_LoadSlot> LoadSlot_1;
	UPROPERTY()
	TObjectPtr<UMVVM_LoadSlot> LoadSlot_2;

	UPROPERTY()
	UMVVM_LoadSlot* SelectedSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess="true"));
	int32 NumLoadSlots;
};
