// Copyright Druid Mechanics


#include "AbilitySystem/Data/CharacterClassInfo.h"

FCharacterClassDefaultInfo UCharacterClassInfo::GetClassDefaultInfo(ECharacterClass CharacterClass)
{
	//在哈希表中找到对应角色类的默认信息
	return CharacterClassInformation.FindChecked(CharacterClass);
}
