// Copyright Druid Mechanics


#include "AbilitySystem/Abilities/AuraFireBlast.h"

#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Actor/AuraFireBall.h"

FString UAuraFireBlast::GetDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT("<Title>烈焰冲击</>\n\n<Small>等级：</><Level>%d</>\n<Small>法力消耗：</><ManaCost>%.1f</>\n<Small>冷却时间：</><Cooldown>%.1f</>\n\n<Default>向四周发射 %d 个火球，每个火球都会返回并在返回时爆炸，造成 </><Damage>%d</><Default> 点范围火焰伤害，并有概率施加燃烧。</>"), Level, ManaCost, Cooldown, NumFireBalls, ScaledDamage);
}

FString UAuraFireBlast::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT("<Title>下一级</>\n\n<Small>等级：</><Level>%d</>\n<Small>法力消耗：</><ManaCost>%.1f</>\n<Small>冷却时间：</><Cooldown>%.1f</>\n\n<Default>向四周发射 %d 个火球，每个火球都会返回并在返回时爆炸，造成 </><Damage>%d</><Default> 点范围火焰伤害，并有概率施加燃烧。</>"), Level, ManaCost, Cooldown, NumFireBalls, ScaledDamage);
}

TArray<AAuraFireBall*> UAuraFireBlast::SpawnFireBalls()
{
	TArray<AAuraFireBall*> FireBalls;
	const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
	const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();
	TArray<FRotator> Rotators = UAuraAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, 360.f, NumFireBalls);

	for (const FRotator& Rotator : Rotators)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(Location);
		SpawnTransform.SetRotation(Rotator.Quaternion());
		
		AAuraFireBall* FireBall = GetWorld()->SpawnActorDeferred<AAuraFireBall>(
			FireBallClass,
			SpawnTransform,
			GetOwningActorFromActorInfo(),
			CurrentActorInfo->PlayerController->GetPawn(),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		
		FireBall->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
		FireBall->ReturnToActor = GetAvatarActorFromActorInfo();
		FireBall->SetOwner(GetAvatarActorFromActorInfo());

		FireBall->ExplosionDamageParams = MakeDamageEffectParamsFromClassDefaults();
		FireBall->SetOwner(GetAvatarActorFromActorInfo());

		FireBalls.Add(FireBall);

		FireBall->FinishSpawning(SpawnTransform);
	}
	
	return FireBalls;
}
