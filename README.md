# ___Aura_demo___

基于 UE5.2 GAS 插件开发的 RPG 多人联机游戏

> **声明**：本项目仅供学习使用，无任何商用目的。原项目版权归 Druid Mechanics 所有。
> 项目作者 Stephen Ulibarri 是 Druid Mechanics 的创始人与核心运营者。

**参考链接：**
- 原项目仓库：[DruidMech/GameplayAbilitySystem_Aura](https://github.com/DruidMech/GameplayAbilitySystem_Aura)
- Udemy 课程：[Unreal Engine 5 - GAS Top Down RPG](https://www.udemy.com/course/unreal-engine-5-gas-top-down-rpg/)
- 大佬笔记(暮志未晚NAN)：[CSDN 专栏](https://blog.csdn.net/qq_30100043/category_12552698.html)

注：本项目在学习课程过程发现原Github项目和课程内容存在部分差异，且原项目已不再维护更新。
因此本人在学习过程中对项目进行了部分重构和功能扩展，以确保最终效果与课程内容一致，同时对原版游戏项目进行了汉化处理，降低游玩门槛。
个人学习过程中的注释和笔记记录在此项目中，仅供参考，欢迎交流讨论。
项目中中文注释均为项目开发笔记，本 md 文档作亮点功能总结

---

## 项目亮点总览

### 一、多样化技能体系 — 11 个 Gameplay Ability

基于 `AuraGameplayAbility` → `AuraDamageGameplayAbility` 的继承体系，实现了多种技能类型：

| 技能类型 | 代表技能 | 说明 |
|---------|---------|------|
| **投射物** | `AuraFireBolt` | 支持多发散射、追踪弹道（Homing），弹道数量和加速度随技能等级缩放 |
| **射线** | `Electrocute`（继承自 `AuraBeamSpell`） | 射线检测 + 链式传导，可自动锁定最多 5 个附近目标 |
| **AOE** | `AuraFireBlast` / `ArcaneShards` | 火焰爆破生成环绕火球；奥术碎片支持范围径向伤害 |
| **召唤** | `AuraSummonAbility` | 在角色周围扇形区域内随机生成多个仆从，种类和数量可配置 |
| **被动** | `AuraPassiveAbility` | 生命/法力汲取、保护光环等，配合 `PassiveNiagaraComponent` 实现持续视觉特效 |
| **近战** | `AuraMeleeAttack` | 基于蒙太奇标签的近战攻击 |

继承图如下：

![GameplayAbility.png](MyNote/GameplayAbility.png)

每个技能均通过 `GetDescription` / `GetNextLevelDescription` 提供多等级技能描述，用于技能树 UI。

---

### 二、公式化伤害计算系统

通过自定义 `ExecCalc_Damage`（`GameplayEffectExecutionCalculation`）实现完整的伤害公式，捕获并参与计算的属性包括：

- **攻方属性**：护甲穿透、暴击率、暴击伤害
- **守方属性**：护甲、格挡率、暴击抗性、四种元素抗性（火/雷/奥术/物理）

伤害计算流程中还包含：
- **格挡判定** — 成功格挡时伤害减半
- **护甲减伤** — 受护甲穿透系数影响
- **暴击判定** — 暴击率受暴击抗性削减，暴击时额外加成
- **Debuff 判定** — 根据伤害类型映射对应 Debuff（灼烧/眩晕/奥术/物理），基于 Debuff 概率与目标抗性进行概率投掷

属性初始化由两个 `ModMagCalc`（`MMC_MaxHealth`、`MMC_MaxMana`）配合 `CharacterClassInfo` 数据资产完成，不同职业类型（Elementalist / Warrior / Ranger）拥有独立的主属性 GE 和初始技能配置。

---

### 三、自定义 GameplayEffectContext 扩展

继承 `FGameplayEffectContext` 实现 `FAuraGameplayEffectContext`，额外携带：
- 暴击 / 格挡命中标记
- Debuff 成功标记及参数（伤害、持续时间、频率）
- 伤害类型（`FGameplayTag`）
- 击退力 / 死亡冲击力
- 径向伤害参数（内外半径、原点）

在 `AuraAbilitySystemGlobals` 中注册，使整条 GE 管线都能访问上述自定义数据。

---

### 四、丰富的角色属性集（AttributeSet）

`AuraAttributeSet` 定义了全套属性，全部支持网络复制（`ReplicatedUsing`）：

| 分类 | 属性 |
|------|------|
| 主属性 | 力量、智力、韧性、活力 |
| 次属性 | 护甲、护甲穿透、格挡率、暴击率、暴击伤害、暴击抗性、生命/法力回复、最大生命/法力 |
| 抗性属性 | 火焰、雷电、奥术、物理 |
| 元属性 | 经验值（`IncomingXP`）、受到的伤害（`IncomingDamage`） |

利用 `TMap<FGameplayTag, TStaticFuncPtr<FGameplayAttribute()>>` 建立标签到属性获取函数的映射，便于在 UI 和数据驱动流程中统一通过标签访问任意属性。

### 五、UI 系统 — MVC + MVVM 双模式
**MVC 部分（游戏内 HUD）：**
- `AuraWidgetController` 基类持有 PlayerController / PlayerState / ASC / AttributeSet 四组件引用
- 派生出 `OverlayWidgetController`（血条、法力条、经验条、浮动消息）、`AttributeMenuWidgetController`（属性菜单）、`SpellMenuWidgetController`（技能树）
- 通过多播委托将属性变化、技能状态变化、经验/升级事件实时广播至 Widget

**MVVM 部分（存档界面）：**
- 引入 UE5 **UMG ViewModel** 插件，`MVVM_LoadScreen` / `MVVM_LoadSlot` 作为视图模型
- 支持 3 个存档槽位，包含新建、选择、删除、开始等完整存档操作
- Widget 通过 `PropertyPath` 绑定视图模型属性，实现数据与视图的自动同步


---

### 六、完整的存档系统

- `LoadScreenSaveGame` 继承 `USaveGame`，序列化玩家等级、经验、属性点、技能点、已学技能（含等级和槽位）、地图状态（`FSavedMap` / `FSavedActor`）
- `AuraGameModeBase` 提供 `SaveSlotData` / `GetSaveSlotData` / `DeleteSlot` / `SaveWorldState` / `LoadWorldState` 等完整存取接口
- 检查点（`Checkpoint`）继承 `PlayerStart` + `ISaveInterface` + `IHighlightInterface`，支持发光提示和重叠触发自动保存；`MapEntrance` 实现跨地图传送并保存进度

---

### 七、多人联机网络架构

全链路考虑了网络同步：
- 角色属性全部通过 `ReplicatedUsing` 回调同步
- 技能释放、升级、装备等操作使用 `Server RPC`（Reliable）+ `Client RPC` 保证权威性
- 被动技能特效通过 `NetMulticast`（Unreliable）同步到所有客户端
- 眩晕、灼烧、电击等状态使用 `Replicated` 属性同步表现
- 玩家等级、经验、属性点、技能点均在 `AuraPlayerState` 中复制

---

### 八、AI 敌人系统

- `AuraAIController` 配合行为树驱动敌人逻辑
- `BTService_FindNearestPlayer`：行为树服务，持续搜索最近玩家并更新黑板键
- `BTTask_Attack`：行为树任务，通过 `CombatInterface` 设置战斗目标并触发攻击 GA
- 敌人支持三种职业分类（Elementalist / Warrior / Ranger），各自拥有独立属性初始化 GE、初始技能和经验奖励
- `AuraEnemySpawnVolume` + `AuraEnemySpawnPoint` 实现区域化敌人生成

---

### 九、全局管理 — 单例标签 & 函数库

**`FAuraGameplayTags`（单例模式）：**
- 在 `AuraAssetManager` 启动阶段统一注册所有 Native GameplayTag
- 维护 `DamageTypesToResistances` / `DamageTypesToDebuffs` 等 TMap，驱动伤害-抗性-Debuff 的数据关联

**`AuraAbilitySystemLibrary`（`BlueprintFunctionLibrary`）：**
- 提供获取 WidgetController、初始化角色属性、读写自定义 EffectContext 等全局静态函数
- 封装了从 GameMode 获取 `CharacterClassInfo` / `AbilityInfo` / `LootTiers` 等数据资产的便捷访问

---

### 十、掉落 & 经验成长

- `LootTiers` 数据资产配置掉落表，每项包含生成概率、最大数量和等级覆盖选项
- `LevelUpInfo` 数据资产驱动升级所需经验曲线
- `AuraPlayerState` 管理经验值增减、升级、属性点/技能点分配，并通过委托通知 UI 更新

---

### 十一、接口解耦设计

项目定义了 5 个 UInterface 实现模块间解耦：

| 接口 | 职责 |
|------|------|
| `ICombatInterface` | 战斗通用：获取等级、攻击插槽位置、蒙太奇、受击、死亡、召唤计数等 |
| `IEnemyInterface` | 敌人特有：设置/获取战斗目标 |
| `IHighlightInterface` | 高亮交互：鼠标悬停时角色/物体描边高亮 |
| `IPlayerInterface` | 玩家特有：经验奖励、升级、属性点/技能点变更 |
| `ISaveInterface` | 存档序列化：控制 Actor 是否需要保存 Transform、加载时恢复状态 |

---

### 源码目录结构参考

```
Source/Aura/
├── AbilitySystem/          # GAS 核心：ASC、AttributeSet、GA、GE计算、数据资产、Debuff/被动特效
│   ├── Abilities/          #   11 个 GameplayAbility
│   ├── AbilityTasks/       #   TargetDataUnderMouse
│   ├── AsyncTasks/         #   WaitCooldownChange
│   ├── Data/               #   AbilityInfo, AttributeInfo, CharacterClassInfo, LevelUpInfo, LootTiers
│   ├── Debuff/             #   DebuffNiagaraComponent
│   ├── ExecCalc/           #   ExecCalc_Damage
│   ├── ModMagCalc/         #   MMC_MaxHealth, MMC_MaxMana
│   └── Passive/            #   PassiveNiagaraComponent
├── Actor/                  # 场景 Actor：发射物、火球、魔法圈、效果Actor、敌人生成点/区域
├── AI/                     # AI控制器、行为树服务与任务
├── Character/              # 角色基类、玩家角色、敌人类
├── Checkpoint/             # 检查点（存档点）、地图入口
├── Game/                   # 游戏实例、游戏模式、SaveGame
├── Input/                  # 输入组件与输入配置（Tag驱动的技能输入绑定）
├── Interaction/            # 5 个 UInterface
├── Player/                 # 玩家控制器、玩家状态
└── UI/                     # HUD、Widget、WidgetController（MVC）、ViewModel（MVVM）
```


