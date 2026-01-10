# Unreal Engine GAS：核心逻辑之应用 Gameplay Effect Spec

这段代码是**Unreal Engine GAS（Gameplay Ability System）核心逻辑**——`UGameplayAbilitySystemComponent`（ASC）中“应用Gameplay Effect Spec（GE Spec）到目标”的关键流程，本质是对“要生效的GE”做**全量合法性校验 + 堆叠/网络处理 + 最终生效执行**，我会分模块拆解核心逻辑，让你清晰理解每一步的作用：

### 一、代码整体功能总结

这段代码的核心是：**接收一个GE Spec（GE的实例化规格），经过“锁保护→网络权限→免疫→属性→概率→标签→自定义规则”等多轮校验后，将合法的GE应用到ASC所属角色身上（处理堆叠/持续时间/Gameplay Cue等），最终返回GE的活跃句柄（失败则返回空）**。

### 二、分模块逐段解析

#### 1. 前置准备：锁保护 + 上下文标记（线程/逻辑安全）

```C++

// 1. 加锁保护ActiveGameplayEffects容器，避免GE在添加过程中被篡改（线程安全）
FScopedActiveGameplayEffectLock ScopeLock(ActiveGameplayEffects);
// 2. 标记当前正在应用的GE Spec，供全局逻辑（如属性计算）识别
FScopeCurrentGameplayEffectBeingApplied ScopedGEApplication(&Spec, this);
// 3. 记录当前ASC所属Actor是否有网络权威（服务器=权威，客户端=非权威）
const bool bIsNetAuthority = IsOwnerActorAuthoritative();
```

- 核心目的：保证GE应用过程中容器不被并发修改，同时标记“当前正在应用GE”的上下文，避免逻辑冲突。

#### 2. 网络权限校验（防客户端作弊，GAS核心安全规则）

```C++

// 校验是否有应用GE的网络权限（服务器才有最终权威，客户端预测需符合规则）
if (!HasNetworkAuthorityToApplyGameplayEffect(PredictionKey))
{
    return FActiveGameplayEffectHandle(); // 无权限则直接返回失败
}

// 禁止对“周期性GE”做客户端预测（避免客户端伪造周期效果）
if(PredictionKey.IsValidKey() && Spec.GetPeriod() > 0.f)
{
    if(IsOwnerActorAuthoritative())
    {
        // 服务器端：清空无效的预测Key，继续执行（服务器权威覆盖客户端）
        PredictionKey = FPredictionKey();
    }
    else
    {
        // 客户端：直接返回失败，不允许预测周期性GE
        return FActiveGameplayEffectHandle();
    }
}
```

- 核心规则：GAS中**GE的最终生效必须由服务器判定**，客户端仅能对“即时GE（Instant）”做预测，周期性GE禁止客户端预测，防止作弊。

#### 3. 免疫校验（GE互斥，比如“无敌状态免疫中毒”）

```C++

const FActiveGameplayEffect* ImmunityGE=nullptr;
// 检查目标是否对当前GE有免疫（比如身上有“Immunity.Poison”标签）
if (ActiveGameplayEffects.HasApplicationImmunityToSpec(Spec, ImmunityGE))
{
    OnImmunityBlockGameplayEffect(Spec, ImmunityGE); // 触发“免疫阻挡GE”回调
    return FActiveGameplayEffectHandle(); // 免疫则返回失败
}
```

- 实现逻辑：通过`ApplicationImmunity`标签/规则判断，比如目标身上有“免疫所有伤害”的GE，则拒绝应用“伤害类GE”。

#### 4. 属性合法性校验（避免无效属性修改）

```C++

// 遍历GE的所有属性修改器，检查属性是否有效
for (const FGameplayModifierInfo& Mod : Spec.Def->Modifiers)
{
    if (!Mod.Attribute.IsValid())
    {
        // 日志警告：属性无效（比如拼写错误、未注册的属性）
        ABILITY_LOG(Warning, TEXT("%s has a null modifier attribute."), *Spec.Def->GetPathName());
        return FActiveGameplayEffectHandle(); // 属性无效则返回失败
    }
}
```

- 核心目的：确保GE要修改的属性（如`Strength`、`Health`）是已注册到Attribute Set的合法属性，避免运行时崩溃。

#### 5. 应用概率校验（GE生效的随机概率，比如“30%概率触发暴击Buff”）

```C++

float ChanceToApply = Spec.GetChanceToApplyToTarget();
// 概率<100%时，随机判定是否生效（SMALL_NUMBER是浮点精度容错）
if ((ChanceToApply < 1.f - SMALL_NUMBER) && (FMath::FRand() > ChanceToApply))
{
    return FActiveGameplayEffectHandle(); // 概率判定失败，返回空
}
```

- 示例：若`ChanceToApply=0.8`（80%概率），则有20%概率直接拒绝应用该GE。

#### 6. 标签规则校验（GE应用/移除的标签条件）

```C++

static FGameplayTagContainer MyTags;
MyTags.Reset();
GetOwnedGameplayTags(MyTags); // 获取目标当前的所有Gameplay Tag

// 校验“应用GE的标签条件”（比如需要目标有“State.Combat”标签才生效）
if (Spec.Def->ApplicationTagRequirements.RequirementsMet(MyTags) == false)
{
    return FActiveGameplayEffectHandle();
}

// 校验“移除GE的标签条件”（比如目标有“State.Dead”标签则拒绝应用）
if (!Spec.Def->RemovalTagRequirements.IsEmpty() && Spec.Def->RemovalTagRequirements.RequirementsMet(MyTags) == true)
{
    return FActiveGameplayEffectHandle();
}
```

- 核心逻辑：通过Tag规则控制GE的生效条件，是GAS中“条件化GE”的核心实现方式。

#### 7. 自定义应用规则校验（开发者扩展的校验逻辑）

```C++

// 遍历GE的自定义应用要求列表
for (const TSubclassOf<UGameplayEffectCustomApplicationRequirement>& AppReq : Spec.Def->ApplicationRequirements)
{
    // 调用自定义校验逻辑（比如“目标等级≥5级才允许应用”）
    if (*AppReq && AppReq->GetDefaultObject<UGameplayEffectCustomApplicationRequirement>()->CanApplyGameplayEffect(Spec.Def, Spec, this) == false)
    {
        return FActiveGameplayEffectHandle(); // 自定义规则不满足则返回失败
    }
}
bIsNetDirty = true; // 标记ASC有网络数据变更，需要同步到客户端
```

- 扩展能力：开发者可继承`UGameplayEffectCustomApplicationRequirement`实现自定义校验，比如“距离限制、等级限制、职业限制”等。

#### 8. 客户端预测特殊处理（即时GE的无限时长兼容）

```C++

// 客户端对“预测的即时GE”做特殊处理：临时设为无限时长（后续服务器同步后清理）
bool bTreatAsInfiniteDuration = GetOwnerRole() != ROLE_Authority && PredictionKey.IsLocalClientKey() && Spec.Def->DurationPolicy == EGameplayEffectDurationType::Instant;
```

- 原因：客户端预测即时GE时，需要临时保留GE状态，避免服务器同步前效果丢失，后续会通过网络同步修正。

#### 9. GE实际应用：处理堆叠/持续时间（核心执行逻辑）

```C++

FActiveGameplayEffectHandle MyHandle(INDEX_NONE); // GE活跃句柄（初始空）
bool bInvokeGameplayCueApplied = Spec.Def->DurationPolicy != EGameplayEffectDurationType::Instant; // 即时GE不触发持续的Gameplay Cue
bool bFoundExistingStackableGE = false; // 是否找到已存在的可堆叠GE
FActiveGameplayEffect* AppliedEffect = nullptr;
FGameplayEffectSpec* OurCopyOfSpec = nullptr;
TSharedPtr<FGameplayEffectSpec> StackSpec;

// 非即时GE（或客户端预测的即时GE）：处理堆叠/添加到活跃GE列表
if (Spec.Def->DurationPolicy != EGameplayEffectDurationType::Instant || bTreatAsInfiniteDuration)
{
    // 核心函数：将GE Spec应用到活跃GE列表，处理堆叠（Aggregate by Source/Target）
    AppliedEffect = ActiveGameplayEffects.ApplyGameplayEffectSpec(Spec, PredictionKey, bFoundExistingStackableGE);
    if (!AppliedEffect)
    {
        return FActiveGameplayEffectHandle(); // 应用失败（比如堆叠上限）
    }
    MyHandle = AppliedEffect->Handle; // 获取GE的活跃句柄
    OurCopyOfSpec = &(AppliedEffect->Spec); // 拿到GE Spec的副本（用于后续修改）

    // 日志：输出GE应用的详细信息（属性修改值、操作类型等）
    if (UE_LOG_ACTIVE(VLogAbilitySystem, Log)) { ... }
}

// 即时GE：创建临时Spec副本（用于属性计算/Gameplay Cue）
if (!OurCopyOfSpec)
{
    StackSpec = TSharedPtr<FGameplayEffectSpec>(new FGameplayEffectSpec(Spec));
    OurCopyOfSpec = StackSpec.Get();
    UAbilitySystemGlobals::Get().GlobalPreGameplayEffectSpecApply(*OurCopyOfSpec, this); // 全局预处理
    OurCopyOfSpec->CaptureAttributeDataFromTarget(this); // 捕获目标当前属性数据
}

// 客户端预测的即时GE：强制设为无限时长
if (bTreatAsInfiniteDuration)
{
    OurCopyOfSpec->SetDuration(UGameplayEffect::INFINITE_DURATION, true);
}
```

- 核心：`ActiveGameplayEffects.ApplyGameplayEffectSpec`是处理GE堆叠的关键函数，会根据`Stacking Type`（按来源/目标聚合）、`Stack Limit`（堆叠上限）等规则，决定是新增GE还是叠加层数。

#### 10. Gameplay Cue触发（视觉/音效等表现）

```C++

if (OurCopyOfSpec)
{
    UAbilitySystemGlobals::Get().SetCurrentAppliedGE(OurCopyOfSpec); // 更新全局当前应用的GE
}

// 触发Gameplay Cue（比如中毒的粒子特效、回血的音效）
if (!bSuppressGameplayCues && bInvokeGameplayCueApplied && AppliedEffect && !AppliedEffect->bIsInhibited && 
    (!bFoundExistingStackableGE || !Spec.Def->bSuppressStackingCues))
{
    if (OurCopyOfSpec->StackCount > Spec.StackCount)
    {
        // 堆叠层数增加：通过RPC通知客户端更新Gameplay Cue
        UAbilitySystemGlobals::Get().GetGameplayCueManager()->InvokeGameplayCueAddedAndWhileActive_FromSpec(this, *OurCopyOfSpec, PredictionKey);
    }
    else
    {
        // 首次应用：触发“激活”和“持续”的Gameplay Cue
        InvokeGameplayCueEvent(*OurCopyOfSpec, EGameplayCueEvent::OnActive);
        InvokeGameplayCueEvent(*OurCopyOfSpec, EGameplayCueEvent::WhileActive);
    }
}
```

- 作用：Gameplay Cue是GAS中“GE表现层”的核心，负责触发视觉、音效、震动等反馈，区分“首次激活”和“持续生效”两种事件。

#### 11. Gameplay Effect（GE）应用流程的收尾核心逻辑
```C++

// 至少执行一次该GE（如果是即时类型GE，执行一次后即完成；如果是持续型GE，已在上方添加至ActiveGameplayEffects列表）
	
// 如果是即时应用类GE，则执行相关逻辑
// bTreatAsInfiniteDuration是指客户端且instant类GE
if (bTreatAsInfiniteDuration)
{
	// 这是一个即时应用的GE，但为了客户端预测，我们将其视为无限时长类型。
	// 仍需预测触发execute阶段的GameplayCue（表现特效）。
	// （非预测场景下，此逻辑会在::ExecuteGameplayEffect内部执行）

	if (!bSuppressGameplayCues)
	{
		UAbilitySystemGlobals::Get().GetGameplayCueManager()->InvokeGameplayCueExecuted_FromSpec(this, *OurCopyOfSpec, PredictionKey);
	}
}
else if (Spec.Def->DurationPolicy == EGameplayEffectDurationType::Instant)
{
	if (OurCopyOfSpec->Def->OngoingTagRequirements.IsEmpty())
	{
		ExecuteGameplayEffect(*OurCopyOfSpec, PredictionKey);
	}
	else
	{
		ABILITY_LOG(Warning, TEXT("%s 是即时类型GE但包含标签依赖项。标签依赖仅可用于带时长的GameplayEffect，该GE将被忽略。"), *Spec.Def->GetPathName());
	}
}

if (Spec.GetPeriod() != UGameplayEffect::NO_PERIOD && Spec.TargetEffectSpecs.Num() > 0)
{
	ABILITY_LOG(Warning, TEXT("%s 是周期性GE，但同时向目标应用了其他GameplayEffect。这些GameplayEffect仅会应用一次，不会随周期重复触发。"), *Spec.Def->GetPathName());
}

// 评估当前GE应用后，是否需要移除目标身上已有的部分活跃GE
if (bIsNetAuthority)
{
	ActiveGameplayEffects.AttemptRemoveActiveEffectsOnEffectApplication(*OurCopyOfSpec, MyHandle);
}

// ------------------------------------------------------
// 应用联动的GameplayEffect（Linked effects）
// 待办：当前忽略了应用返回的句柄，是否应将这些句柄存入TArray并统一返回？
// ------------------------------------------------------
for (const FGameplayEffectSpecHandle& TargetSpec: Spec.TargetEffectSpecs)
{
	if (TargetSpec.IsValid())
	{
		ApplyGameplayEffectSpecToSelf(*TargetSpec.Data.Get(), PredictionKey);
	}
}

UAbilitySystemComponent* InstigatorASC = Spec.GetContext().GetInstigatorAbilitySystemComponent();

// 向自身发送回调通知
OnGameplayEffectAppliedToSelf(InstigatorASC, *OurCopyOfSpec, MyHandle);

// 向发起者（Instigator）发送回调通知
if (InstigatorASC)
{
	InstigatorASC->OnGameplayEffectAppliedToTarget(this, *OurCopyOfSpec, MyHandle);
}

return MyHandle;
```

### 三、核心总结

1. **校验优先**：GE应用前会经过**网络权限→免疫→属性→概率→标签→自定义规则** 6轮校验，任何一步失败都会直接返回，保证GE生效的合法性；

2. **网络安全**：服务器拥有最终权威，客户端仅能预测“即时GE”，周期性GE禁止预测，防止作弊；

3. **堆叠核心**：非即时GE通过`ActiveGameplayEffects.ApplyGameplayEffectSpec`处理堆叠，所有堆叠层共享一个GE Spec，仅记录层数和计时；

4. **表现分离**：GE的“逻辑生效”和“视觉表现”通过Gameplay Cue分离，保证逻辑与表现解耦。

> （注：文档部分内容可能由 AI 生成）
