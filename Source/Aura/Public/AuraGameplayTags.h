// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 *AuraGameplayTags
 *
 * Singleton containing native Gameplay Tags
 * 原生游戏标签，意思是c++本地创造的，在c++内可用，在蓝图和编辑器内也可用
 */

struct FAuraGameplayTags
{
public:
	//单例模式提供访问点
    static const FAuraGameplayTags& Get() { return GameplayTags;}
	//在资产管理类中的StartInitialLoading函数调用，在其中添加以下所有的NativeGameplayTag，同时填充本类中的TMap
    static void InitializeNativeGameplayTags();

	FGameplayTag Attributes_Primary_Strength;
	FGameplayTag Attributes_Primary_Intelligence;
	FGameplayTag Attributes_Primary_Resilience;
	FGameplayTag Attributes_Primary_Vigor;

	FGameplayTag Attributes_Secondary_Armor;
	FGameplayTag Attributes_Secondary_ArmorPenetration;
	FGameplayTag Attributes_Secondary_BlockChance;
	FGameplayTag Attributes_Secondary_CriticalHitChance;
	FGameplayTag Attributes_Secondary_CriticalHitDamage;
	FGameplayTag Attributes_Secondary_CriticalHitResistance;
	FGameplayTag Attributes_Secondary_HealthRegeneration;
	FGameplayTag Attributes_Secondary_ManaRegeneration;
	FGameplayTag Attributes_Secondary_MaxHealth;
	FGameplayTag Attributes_Secondary_MaxMana;
	
	FGameplayTag Attributes_Meta_IncomingXP;

	FGameplayTag InputTag_LMB;
	FGameplayTag InputTag_RMB;
	FGameplayTag InputTag_1;
	FGameplayTag InputTag_2;
	FGameplayTag InputTag_3;
	FGameplayTag InputTag_4;
	FGameplayTag InputTag_Passive_1;
	FGameplayTag InputTag_Passive_2;

	FGameplayTag Damage;
	FGameplayTag Damage_Fire;
	FGameplayTag Damage_Lightning;
	FGameplayTag Damage_Arcane;
	FGameplayTag Damage_Physical;

	FGameplayTag Attributes_Resistance_Fire;
	FGameplayTag Attributes_Resistance_Lightning;
	FGameplayTag Attributes_Resistance_Arcane;
	FGameplayTag Attributes_Resistance_Physical;

	FGameplayTag Debuff_Burn;
	FGameplayTag Debuff_Stun;
	FGameplayTag Debuff_Arcane;
	FGameplayTag Debuff_Physical;

	FGameplayTag Debuff_Chance;
	FGameplayTag Debuff_Damage;
	FGameplayTag Debuff_Duration;
	FGameplayTag Debuff_Frequency;

	FGameplayTag Abilities_None;
	
	FGameplayTag Abilities_Attack;
	FGameplayTag Abilities_Summon;
	
	FGameplayTag Abilities_HitReact;

	FGameplayTag Abilities_Status_Locked;
	FGameplayTag Abilities_Status_Eligible;
	FGameplayTag Abilities_Status_Unlocked;
	FGameplayTag Abilities_Status_Equipped;

	FGameplayTag Abilities_Type_Offensive;
	FGameplayTag Abilities_Type_Passive;
	FGameplayTag Abilities_Type_None;
	
	FGameplayTag Abilities_Fire_FireBolt;
	FGameplayTag Abilities_Fire_FireBlast;	
	FGameplayTag Abilities_Lightning_Electrocute;
	FGameplayTag Abilities_Arcane_ArcaneShards;


	FGameplayTag Abilities_Passive_HaloOfProtection;
	FGameplayTag Abilities_Passive_LifeSiphon;
	FGameplayTag Abilities_Passive_ManaSiphon;

	FGameplayTag Cooldown_Fire_FireBolt;

	FGameplayTag CombatSocket_Weapon;
	FGameplayTag CombatSocket_RightHand;
	FGameplayTag CombatSocket_LeftHand;
	FGameplayTag CombatSocket_Tail;

	FGameplayTag Montage_Attack_1;
	FGameplayTag Montage_Attack_2;
	FGameplayTag Montage_Attack_3;
	FGameplayTag Montage_Attack_4;

	//不同伤害和对应伤害类型抗性的标签映射组
	TMap<FGameplayTag, FGameplayTag> DamageTypesToResistances;
	//伤害和对应负面效果的映射组，如GameplayTags.Damage_Fire, GameplayTags.Debuff_Burn的映射
	TMap<FGameplayTag, FGameplayTag> DamageTypesToDebuffs;

	FGameplayTag Effects_HitReact;

	// 玩家输入阻断标签，当ASC持有这些标签时，对应的输入处理会被跳过
	// 使用场景：眩晕(Stun)Debuff期间，通过GE(服务端)或OnRep_Stunned(客户端)将这四个标签全部添加到ASC上，从而在眩晕期间完全禁用玩家的所有输入和光标追踪
	FGameplayTag Player_Block_InputPressed;  // 阻断AbilityInputTagPressed和Move：禁止技能按下触发和WASD移动
	FGameplayTag Player_Block_InputHeld;     // 阻断AbilityInputTagHeld：禁止技能持续触发(含LMB长按移动和技能激活)
	FGameplayTag Player_Block_InputReleased; // 阻断AbilityInputTagReleased：禁止技能松开触发(含LMB短按寻路点击移动)
	FGameplayTag Player_Block_CursorTrace;   // 阻断CursorTrace：禁止光标射线检测(取消当前高亮并清空目标Actor缓存)

	FGameplayTag GameplayCue_FireBlast;

private:
    static FAuraGameplayTags GameplayTags;
};
