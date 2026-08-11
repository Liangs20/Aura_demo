// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HighlightInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UHighlightInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AURA_API IHighlightInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent)
	void HighlightActor();
	
	UFUNCTION(BlueprintNativeEvent)
	void UnHighlightActor();

	//功能是当玩家点击地面时，玩家角色会移动到点击的位置，这个函数的功能就是提供一个接口，让玩家角色知道要移动到哪里
	UFUNCTION(BlueprintNativeEvent)
	void SetMoveToLocation(FVector& OutDestination);
};
