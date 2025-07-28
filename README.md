# Aura_demo
基于UE5.2 GAS插件开发的 RPG 多人联机游戏

在 HUD 上，使用自定义模板实现了怪物沟边，利用 MVC 模型构建 UI 系统，包括人物属性界面以及技能树界面等，通过委托和事件机制完善了属性点、技能点的养成交互。
人物技能实现上 ，利用单例模式自定义 GameplayTag、 BlueprintFunctionLibrary 等全局类来管理标签和全局函数等。通过构建多种修饰器属性类型的 GE 和 AbilityTask 实现了多种技能；
在伤害数值计算上 ，通过 ModifierMagnitudeCalculation 和 ExecutionCalculation 实现了角色各属性的初始化和伤害的公式化计算等 ，涉及到的 AttributeSet 内属性包括护甲穿透、暴击抗性等多种属性。

项目中中文注释均为项目开发笔记。
