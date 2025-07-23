// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "AuraAbilitySystemGlobals.generated.h"

/*
  这里调整了DefaultGame.ini文件，添加了
	[/Script/GameAbilities.AbilitySystemGlobals]
	+AbilitySystemGlobalsClassName="/Script/Aura.AuraAbilitySystemGlobals"
	如此调整过后，项目默认使用自定义的AuraAbilitySystemGlobals，进而使用AuraGameplayEffectContext
 */
UCLASS()
class AURA_API UAuraAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
