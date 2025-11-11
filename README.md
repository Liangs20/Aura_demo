# ___Aura_demo___
基于UE5.2 GAS插件开发的 RPG 多人联机游戏

~~在 HUD 上，使用自定义模板实现了怪物沟边，利用 MVC 模型构建 UI 系统，包括人物属性界面以及技能树界面等，通过委托和事件机制完善了属性点、技能点的养成交互。
人物技能实现上 ，利用单例模式自定义 GameplayTag、 BlueprintFunctionLibrary 等全局类来管理标签和全局函数等。通过构建多种修饰器属性类型的 GE 和 AbilityTask 实现了多种技能；
在伤害数值计算上 ，通过 ModifierMagnitudeCalculation 和 ExecutionCalculation 实现了角色各属性的初始化和伤害的公式化计算等 ，涉及到的 AttributeSet 内属性包括护甲穿透、暴击抗性等多种属性。~~

>如下按文件架构划分模块，便于理解项目结构和功能划分。


## 一、技能系统模块
### a） 11个Gameplay Ability
URL类图见
[GameplayAbility.vsdx](../../../Visio/Store/GameplayAbility.vsdx)

### b） 1个Ability Task

### c） 1个异步Task

### d） 5个数据信息

### e） 1个Debuff特效组件

### f） 

### 根）4个

## 二、创生物模块



## 三、AI模块



## 四、角色类模块



## 五、检查点模块



## 六、游戏全局模块



## 七、输入模块



## 八、接口模块



## 九、玩家控制器和状态模块



## 十、UI模块



## 根、全局类模块
项目中中文注释均为项目开发笔记。
