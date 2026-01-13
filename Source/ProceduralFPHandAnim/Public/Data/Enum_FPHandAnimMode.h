#pragma once

#include "CoreMinimal.h"
#include "Enum_FPHandAnimMode.generated.h"

UENUM(BlueprintType)
enum class EFPHandAnimMode : uint8
{
	/** Only left hand animation is driven */
	LeftHandOnly	UMETA(DisplayName = "Left Hand Only"),

	/** Only right hand animation is driven */
	RightHandOnly	UMETA(DisplayName = "Right Hand Only"),

	/** Both hands are animated */
	BothHands		UMETA(DisplayName = "Both Hands")
};
