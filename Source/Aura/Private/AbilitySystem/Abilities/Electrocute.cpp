// Copyright Druid Mechanics


#include "AbilitySystem/Abilities/Electrocute.h"

FString UElectrocute::GetDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	if (Level == 1)
	{
		return FString::Printf(TEXT("<Title>电刑</>\n\n<Small>等级：</><Level>%d</>\n<Small>法力消耗：</><ManaCost>%.1f</>\n<Small>冷却时间：</><Cooldown>%.1f</>\n\n<Default>释放一道闪电束连接目标，持续造成 </><Damage>%d</><Default> 点闪电伤害，并有概率造成眩晕。</>"), Level, ManaCost, Cooldown, ScaledDamage);
	}
	else
	{
		return FString::Printf(TEXT("<Title>电刑</>\n\n<Small>等级：</><Level>%d</>\n<Small>法力消耗：</><ManaCost>%.1f</>\n<Small>冷却时间：</><Cooldown>%.1f</>\n\n<Default>释放一道闪电束连接目标，并额外传导至附近 %d 个目标，造成 </><Damage>%d</><Default> 点闪电伤害，并有概率造成眩晕。</>"), Level, ManaCost, Cooldown, FMath::Min(Level, MaxNumShockTargets - 1), ScaledDamage);		
	}
}

FString UElectrocute::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT("<Title>下一级</>\n\n<Small>等级：</><Level>%d</>\n<Small>法力消耗：</><ManaCost>%.1f</>\n<Small>冷却时间：</><Cooldown>%.1f</>\n\n<Default>释放一道闪电束连接目标，并额外传导至附近 %d 个目标，造成 </><Damage>%d</><Default> 点闪电伤害，并有概率造成眩晕。</>"), Level, ManaCost, Cooldown, FMath::Min(Level, MaxNumShockTargets - 1), ScaledDamage);	
}
