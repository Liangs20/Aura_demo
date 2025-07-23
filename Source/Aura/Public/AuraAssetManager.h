// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "AuraAssetManager.generated.h"

/**
 * 进入项目配置文件夹修改过DefaultEngine.ini
 * 于[/Script/Engine.Engine]下添加了AssetManagerClassName=/Script/Aura_demo.AuraAssetManager
 */
UCLASS()
class AURA_API UAuraAssetManager : public UAssetManager
{
	GENERATED_BODY()
public:

	static UAuraAssetManager& Get();

protected:

	virtual void StartInitialLoading() override;
};
