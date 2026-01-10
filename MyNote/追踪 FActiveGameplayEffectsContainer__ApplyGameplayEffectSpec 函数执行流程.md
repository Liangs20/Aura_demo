# 追踪 FActiveGameplayEffectsContainer::ApplyGameplayEffectSpec 函数执行流程


### 函数核心定位

这个函数是 GAS 中**应用 GameplayEffectSpec（GE规格）到活跃效果容器**的核心逻辑，负责：

1. 处理 GE 的「堆叠」（已有同类型活跃GE时叠加层数）；

2. 处理 GE 的「新建」（无同类型活跃GE时创建新实例）；

3. 初始化 GE 的时长、周期、属性修改、回调注册等关键逻辑；

4. 兼容服务器权威/客户端预测的同步逻辑。

---

### 完整执行流程（分8个核心阶段）

#### 阶段1：前置校验与初始化（安全检查+基础变量准备）

```C++

// 1. 性能统计+加锁（GAMEPLAYEFFECT_SCOPE_LOCK 保证线程安全）
SCOPE_CYCLE_COUNTER(STAT_ApplyGameplayEffectSpec);
GAMEPLAYEFFECT_SCOPE_LOCK();

// 2. 校验GE定义是否有效，无效则直接返回nullptr
if (!ensureMsgf(Spec.Def, ...)) { return nullptr; }

// 3. 初始化基础变量
bFoundExistingStackableGE = false; // 是否找到可堆叠的已有GE
AActor* OwnerActor = Owner ? Owner->GetOwnerActor() : nullptr;

// 4. 服务器端：刷新宿主Actor的网络休眠状态（确保同步）
if (IsNetAuthority() && OwnerActor) { OwnerActor->FlushNetDormancy(); }

// 5. 声明核心变量：最终应用的活跃GE、可堆叠的已有GE
FActiveGameplayEffect* AppliedActiveGE = nullptr;
FActiveGameplayEffect* ExistingStackableGE = FindStackableActiveGameplayEffect(Spec);
```

**核心目的**：做基础的安全校验和环境准备，避免无效GE应用，保证服务器同步状态正常。

#### 阶段2：处理「已有可堆叠GE」的场景（叠加层数）

这是函数的核心分支（已有同类型GE时，不新建而是叠加层数）：

```C++

if (ExistingStackableGE)
{
    // 2.1 客户端预测：暂时禁止堆叠预测，直接返回nullptr（服务器才有权处理堆叠）
    if (!IsNetAuthority()) { return nullptr; }
    // 2.2 服务器：清空预测键（客户端不预测堆叠，无需保留预测信息）
    else { InPredictionKey = FPredictionKey(); }

    bFoundExistingStackableGE = true;
    FGameplayEffectSpec& ExistingSpec = ExistingStackableGE->Spec;
    StartingStackCount = ExistingSpec.StackCount; // 记录原始堆叠层数

    // 2.3 标记当前正在应用的GE（全局上下文）
    UAbilitySystemGlobals::Get().SetCurrentAppliedGE(&ExistingSpec);

    // 2.4 堆叠层数溢出处理：若已达上限，调用溢出回调，失败则返回nullptr
    if (ExistingSpec.StackCount == ExistingSpec.Def->StackLimitCount)
    {
        if (!HandleActiveGameplayEffectStackOverflow(...)) { return nullptr; }
    }

    // 2.5 计算新堆叠层数（叠加当前Spec的层数，不超过上限）
    NewStackCount = ExistingSpec.StackCount + Spec.StackCount;
    if (ExistingSpec.Def->StackLimitCount > 0)
    {
        NewStackCount = FMath::Min(NewStackCount, ExistingSpec.Def->StackLimitCount);
    }

    // 2.6 注销旧的聚合器回调（避免源聚合器变化导致异常）
    ExistingSpec.CapturedRelevantAttributes.UnregisterLinkedAggregatorCallbacks(...);

    // 2.7 校验动态标签/资产标签一致性（堆叠时标签需一致，否则报错）
    ensureMsgf(ExistingSpec.DynamicGrantedTags == Spec.DynamicGrantedTags, ...);
    ensureMsgf(ExistingSpec.GetDynamicAssetTags() == Spec.GetDynamicAssetTags(), ...);

    // 2.8 保留首次应用时的技能授予信息（堆叠不重复授予技能）
    TArray<FGameplayAbilitySpecDef> GrantedSpecTempArray(MoveTemp(ExistingStackableGE->Spec.GrantedAbilitySpecs));

    // 2.9 更新已有GE的Spec：替换为新Spec，但保留原始技能授予信息，设置新堆叠层数
    ExistingStackableGE->Spec = Spec;
    ExistingStackableGE->Spec.StackCount = NewStackCount;
    ExistingStackableGE->Spec.GrantedAbilitySpecs = MoveTemp(GrantedSpecTempArray);
    AppliedActiveGE = ExistingStackableGE;

    // 2.10 处理时长/周期刷新策略
    if (GEDef->StackDurationRefreshPolicy == EGameplayEffectStackingDurationPolicy::NeverRefresh)
    {
        bSetDuration = false; // 不刷新时长
    }
    else
    {
        RestartActiveGameplayEffectDuration(*ExistingStackableGE); // 刷新时长
    }
    if (GEDef->StackPeriodResetPolicy == EGameplayEffectStackingPeriodPolicy::NeverReset)
    {
        bSetPeriod = false; // 不重置周期
    }
}
```

**核心目的**：服务器端处理GE堆叠，保证层数不超限、标签一致、技能不重复授予，按策略刷新时长/周期。

#### 阶段3：处理「无可用堆叠GE」的场景（新建活跃GE）

无同类型GE时，创建新的 `FActiveGameplayEffect` 实例：

```C++

else
{
    // 3.1 生成唯一的活跃GE句柄
    FActiveGameplayEffectHandle NewHandle = FActiveGameplayEffectHandle::GenerateNewHandle(Owner);

    // 3.2 处理「作用域锁定+无内存余量」的场景：放入待处理列表（避免内存分配异常）
    if (ScopedLockCount > 0 && GameplayEffects_Internal.GetSlack() <= 0)
    {
        if (*PendingGameplayEffectNext == nullptr)
        {
            AppliedActiveGE = new FActiveGameplayEffect(NewHandle, Spec, ...); // 新建实例
            *PendingGameplayEffectNext = AppliedActiveGE;
        }
        else
        {
            **PendingGameplayEffectNext = FActiveGameplayEffect(NewHandle, Spec, ...); // 复用内存
            AppliedActiveGE = *PendingGameplayEffectNext;
        }
        PendingGameplayEffectNext = &AppliedActiveGE->PendingNext;
    }
    // 3.3 正常场景：直接在容器内存池中新建实例
    else
    {
        AppliedActiveGE = new(GameplayEffects_Internal) FActiveGameplayEffect(NewHandle, Spec, ...);
    }
}
```

**核心目的**：创建新的活跃GE实例，兼容内存池/待处理列表机制，避免内存分配问题。

#### 阶段4：全局上下文+属性捕获/计算（核心逻辑准备）

```C++

// 4.1 标记当前正在应用的GE（全局上下文）
UAbilitySystemGlobals::Get().SetCurrentAppliedGE(&AppliedActiveGE->Spec);

// 4.2 全局预处理GE Spec
UAbilitySystemGlobals::Get().GlobalPreGameplayEffectSpecApply(AppliedEffectSpec, Owner);

// 4.3 捕获目标的GameplayTag（用于过滤无限期GE）
AppliedEffectSpec.CapturedTargetTags.GetActorTags().Reset();
Owner->GetOwnedGameplayTags(AppliedEffectSpec.CapturedTargetTags.GetActorTags());

// 4.4 捕获目标属性数据+计算修改器的数值（核心：属性加成的计算）
AppliedEffectSpec.CaptureAttributeDataFromTarget(Owner);
AppliedEffectSpec.CalculateModifierMagnitudes();

// 4.5 构建修改后的属性列表（给GameplayCue提供数值信息）
// （仅针对“有时长无周期”或“瞬时但有周期”的GE）
if (ShouldBuildModifiedAttributeList)
{
    // 遍历修改器，计算总数值并添加到ModifiedAttributes列表
    for (const FGameplayModifierInfo& Mod : AppliedEffectSpec.Def->Modifiers)
    {
        // 计算数值 → 加入ModifiedAttributes
    }
}
```

**核心目的**：完成GE的属性捕获、数值计算，为后续属性修改/GameplayCue触发做准备。

#### 阶段5：注册回调+计算时长（保证GE生命周期正常）

```C++

// 5.1 注册源/目标的聚合器回调（属性变化时自动更新GE数值）
AppliedEffectSpec.CapturedRelevantAttributes.RegisterLinkedAggregatorCallbacks(AppliedActiveGE->Handle);

// 5.2 计算GE时长（优先从Def中计算，其次用SetByCaller的数值）
float DefCalcDuration = 0.f;
if (AppliedEffectSpec.AttemptCalculateDurationFromDef(DefCalcDuration))
{
    AppliedEffectSpec.SetDuration(DefCalcDuration, false);
}
else if (AppliedEffectSpec.Def->DurationMagnitude.GetMagnitudeCalculationType() == EGameplayEffectMagnitudeCalculation::SetByCaller)
{
    AppliedEffectSpec.Def->DurationMagnitude.AttemptCalculateMagnitude(AppliedEffectSpec, AppliedEffectSpec.Duration);
}

// 5.3 处理时长修改（保证时长>0，避免变成瞬时/无限期）
const float DurationBaseValue = AppliedEffectSpec.GetDuration();
if (DurationBaseValue > 0.f)
{
    float FinalDuration = AppliedEffectSpec.CalculateModifiedDuration();
    if (FinalDuration <= 0.f)
    {
        FinalDuration = 0.1f; // 保底0.1秒
    }
    AppliedEffectSpec.SetDuration(FinalDuration, true);

    // 5.4 注册时长定时器（到期后触发CheckDurationExpired，结束GE）
    if (Owner && bSetDuration)
    {
        FTimerDelegate Delegate = FTimerDelegate::CreateUObject(Owner, &UAbilitySystemComponent::CheckDurationExpired, AppliedActiveGE->Handle);
        TimerManager.SetTimer(AppliedActiveGE->DurationHandle, Delegate, FinalDuration, false);
    }
}
```

**核心目的**：注册属性回调（保证数值动态更新），计算并校验时长，注册时长定时器（控制GE生命周期）。

#### 阶段6：注册周期定时器（处理周期性GE）

```C++

if (bSetPeriod && Owner && (AppliedEffectSpec.GetPeriod() > UGameplayEffect::NO_PERIOD))
{
    FTimerManager& TimerManager = Owner->GetWorld()->GetTimerManager();
    FTimerDelegate Delegate = FTimerDelegate::CreateUObject(Owner, &UAbilitySystemComponent::ExecutePeriodicEffect, AppliedActiveGE->Handle);
    
    // 6.1 若需在应用时立即执行周期逻辑，下帧触发
    if (AppliedEffectSpec.Def->bExecutePeriodicEffectOnApplication)
    {
        TimerManager.SetTimerForNextTick(Delegate);
    }

    // 6.2 注册周期定时器（重复触发，处理持续伤害/回血等）
    TimerManager.SetTimer(AppliedActiveGE->PeriodHandle, Delegate, AppliedEffectSpec.GetPeriod(), true);
}
```

**核心目的**：为周期性GE（如每秒回血）注册定时器，控制周期逻辑的执行。

#### 阶段7：同步逻辑（服务器/客户端预测的标记处理）

```C++

if (InPredictionKey.IsLocalClientKey() == false || IsNetAuthority())
{
    // 7.1 服务器/非预测客户端：标记GE为脏数据（触发网络同步）
    MarkItemDirty(*AppliedActiveGE);
}
else
{
    // 7.2 本地预测客户端：标记数组为脏数据，注册预测回调（同步后移除预测GE）
    MarkArrayDirty();
    InPredictionKey.NewRejectOrCaughtUpDelegate(FPredictionKeyEvent::CreateUObject(Owner, &UAbilitySystemComponent::RemoveActiveGameplayEffect_NoReturn, AppliedActiveGE->Handle, -1));
}
```

**核心目的**：区分服务器/客户端逻辑，处理预测GE的同步和回滚，保证网络一致性。

#### 阶段8：回调触发（通知GE添加/堆叠）

```C++

if (ExistingStackableGE)
{
    // 8.1 堆叠场景：触发堆叠层数变化回调
    OnStackCountChange(*ExistingStackableGE, StartingStackCount, NewStackCount);
}
else
{
    // 8.2 新建场景：触发GE添加回调
    InternalOnActiveGameplayEffectAdded(*AppliedActiveGE);
}

// 返回最终应用的活跃GE实例
return AppliedActiveGE;
```

**核心目的**：通知外部逻辑（如UI、技能系统）GE的添加/堆叠事件，完成整个应用流程。

---

### 总结

这个函数的核心流程可简化为：

1. **前置校验**：安全检查+环境准备；

2. **堆叠/新建**：已有GE则叠加层数，无则新建实例；

3. **属性处理**：捕获属性、计算修改数值；

4. **生命周期**：注册时长/周期定时器、属性回调；

5. **同步处理**：区分服务器/客户端，处理预测/同步；

6. **回调通知**：触发添加/堆叠事件，返回结果。

关键要点：

1. 服务器**独占GE堆叠逻辑**，客户端预测不处理堆叠；

2. 时长/周期的刷新策略由 `StackDurationRefreshPolicy`/`StackPeriodResetPolicy` 控制；

3. 客户端预测的GE会注册回滚回调，同步后自动清理；

4. 所有内存分配优先复用内存池，避免频繁堆分配。


> （注：文档部分内容可能由 AI 生成）