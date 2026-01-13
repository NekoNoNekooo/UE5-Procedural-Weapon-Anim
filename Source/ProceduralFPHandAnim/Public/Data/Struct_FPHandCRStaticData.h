#pragma once

#include "CoreMinimal.h"
#include "Struct_FPHandCRStaticData.generated.h"

USTRUCT(BlueprintType)
struct FFPHandCRStaticData
{
	FFPHandCRStaticData(){}
	
	GENERATED_BODY()

public:

	// ========== Static Offset Param ==========
	
	/** Hand Location Offset (Right) - Camera Space */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset")
	FVector HandOffsetR = FVector::ZeroVector;

	/** Hand Location Offset (Right) - Camera Space */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset")
	FRotator HandOffsetR_Rot = FRotator::ZeroRotator;
	
	/** Hand Location Offset (Left) - Camera Space */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset")
	FVector HandOffsetL = FVector::ZeroVector;
	
	/** Hand Location Offset (Left) - Camera Space */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset")
	FRotator HandOffsetL_Rot = FRotator::ZeroRotator;

	/** Hand Root Offset - Camera Space */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset")
	FVector HandRootOffset = FVector::ZeroVector;
	
	/** Center Offset - Camera Space */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset")
	FVector CenterOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset")
	float SnapLeftHandToRightHand = 0.f;
};