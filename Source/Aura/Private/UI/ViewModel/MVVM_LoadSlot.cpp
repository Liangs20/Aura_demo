// Copyright Druid Mechanics


#include "UI/ViewModel/MVVM_LoadSlot.h"

void UMVVM_LoadSlot::InitializeSlot()
{
	const int32 WidgetSwitcherIndex = SlotStatus.GetValue();
	SetWidgetSwitcherIndex.Broadcast(WidgetSwitcherIndex);
}

void UMVVM_LoadSlot::SetPlayerName(FString InPlayerName)
{
	//MVVM视图模型基类自定义宏，用于设置属性值并自动通知变更
	//触发的广播名为BroadcastFieldValueChanged(FieldId)，其中FieldId默认为ThisClass::FFieldNotificationClassDescriptor::MemberName
	//由于视图模型插件的存在，在控件蓝图的下方会出现视图绑定，在那里绑定的数据如果触发以上广播则会自动动态修改UI显示的值
	UE_MVVM_SET_PROPERTY_VALUE(PlayerName, InPlayerName);
}

void UMVVM_LoadSlot::SetMapName(FString InMapName)
{
	UE_MVVM_SET_PROPERTY_VALUE(MapName, InMapName);
}

void UMVVM_LoadSlot::SetPlayerLevel(int32 InLevel)
{
	UE_MVVM_SET_PROPERTY_VALUE(PlayerLevel, InLevel);
}

void UMVVM_LoadSlot::SetLoadSlotName(FString InLoadSlotName)
{
	UE_MVVM_SET_PROPERTY_VALUE(LoadSlotName, InLoadSlotName);
}
