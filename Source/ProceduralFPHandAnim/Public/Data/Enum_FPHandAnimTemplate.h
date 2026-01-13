#pragma once

#include "CoreMinimal.h"
#include "Enum_FPHandAnimTemplate.generated.h"

UENUM(BlueprintType)
enum class EFPHandAnimTemplate : uint8
{
	Fist,
	Rifle,
	Pistol,
	Prybar,
	CommonItemOneHand,
};
