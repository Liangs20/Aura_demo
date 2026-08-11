// Copyright Druid Mechanics


#include "AbilitySystem/Abilities/ArcaneShards.h"

FString UArcaneShards::GetDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	if (Level == 1)
	{
		return FString::Printf(TEXT("<Title>奥术碎片</>\n\n<Small>等级：</><Level>%d</>\n<Small>法力消耗：</><ManaCost>%.1f</>\n<Small>冷却时间：</><Cooldown>%.1f</>\n\n<Default>召唤 1 枚奥术碎片，在碎片生成处造成 </><Damage>%d</><Default> 点范围奥术伤害。</>"), Level, ManaCost, Cooldown, ScaledDamage);
	}
	else
	{
		return FString::Printf(TEXT("<Title>奥术碎片</>\n\n<Small>等级：</><Level>%d</>\n<Small>法力消耗：</><ManaCost>%.1f</>\n<Small>冷却时间：</><Cooldown>%.1f</>\n\n<Default>召唤 %d 枚奥术碎片，在碎片生成处各自造成 </><Damage>%d</><Default> 点范围奥术伤害。</>"), Level, ManaCost, Cooldown, FMath::Min(Level, MaxNumShards), ScaledDamage);		
	}
}

FString UArcaneShards::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	
	return FString::Printf(TEXT("<Title>下一级</>\n\n<Small>等级：</><Level>%d</>\n<Small>法力消耗：</><ManaCost>%.1f</>\n<Small>冷却时间：</><Cooldown>%.1f</>\n\n<Default>召唤 %d 枚奥术碎片，在碎片生成处各自造成 </><Damage>%d</><Default> 点范围奥术伤害。</>"), Level, ManaCost, Cooldown, FMath::Min(Level, MaxNumShards), ScaledDamage);	
}
