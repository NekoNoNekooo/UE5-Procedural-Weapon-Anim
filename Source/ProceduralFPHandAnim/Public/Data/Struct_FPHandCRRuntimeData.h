#pragma once

#include "CoreMinimal.h"
#include "Struct_FPHandCRRuntimeData.generated.h"

USTRUCT(BlueprintType)
struct FFPHandCRRuntimeData
{
	FFPHandCRRuntimeData(){}
	
	GENERATED_BODY()

public:

	// ========== Static Offset Param ==========
	
	/** Hand Offset (Right) - Camera Space */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HandCRRuntimeData")
	FVector HandOffsetR = FVector::ZeroVector;

	/** Hand Offset (Left) - Camera Space */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HandCRRuntimeData")
	FVector HandOffsetL = FVector::ZeroVector;

	/** Center Offset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HandCRRuntimeData")
	FVector CenterOffset = FVector::ZeroVector;
	
	/** Lock Left Hand (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HandCRRuntimeData", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LockLeftHand = 0.0f;

	/** Hand IK Weight (Left) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HandCRRuntimeData", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HandIKWeightL = 1.0f;

	/** Hand IK Weight (Right) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HandCRRuntimeData", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HandIKWeightR = 1.0f;

	// ========== Additive Offset Param ==========
	
	/** Additive Center Location Offset  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HandCRRuntimeData")
	FRotator Add_CenterRotationWS = FRotator::ZeroRotator;

	/** Additive Center Rotation Offset, X for Right/Left , Y for Forward/Backward, Z for Up/Down */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HandCRRuntimeData")
	FVector Add_CenterLocationOffset = FVector::ZeroVector;

	/** Additive Hand Location Offset (Right, Local Space) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HandCRRuntimeData")
	FVector Add_HandLocOffsetR_Local = FVector::ZeroVector;

	/** Additive Hand Location Offset (Left, Local Space) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HandCRRuntimeData")
	FVector Add_HandLocOffsetL_Local = FVector::ZeroVector;
	
	/** Additive Hand Rotation Offset (Right, Local Space) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HandCRRuntimeData")
	FRotator Add_HandRotOffsetR_Local = FRotator::ZeroRotator;

	/** Additive Hand Rotation Offset (Left, Local Space) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HandCRRuntimeData")
	FRotator Add_HandRotOffsetL_Local = FRotator::ZeroRotator;


};

USTRUCT(BlueprintType)
struct FFPHandAnimDataInput
{
	FFPHandAnimDataInput(){}
	
	GENERATED_BODY()

public:
	
	double TimeSec = 0.0;
	float  DeltaTime = 0.f;
	float UpdateDuration = 0.f;

	FVector Velocity = FVector::ZeroVector;
	FVector2D LookRotSpeed = FVector2D::ZeroVector;
	FRotator ControlRotation = FRotator::ZeroRotator;
	FTransform ActorTransform;
};

USTRUCT(BlueprintType)
struct FFPHandAnimDataOutput
{
	FFPHandAnimDataOutput(){}
	
	GENERATED_BODY()

public:
	
	/** Additive Center Location Offset  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HandCRRuntimeData")
	FRotator Add_CenterRotationWS = FRotator::ZeroRotator;

	/** Additive Center Rotation Offset, X for Right/Left , Y for Forward/Backward, Z for Up/Down */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HandCRRuntimeData")
	FVector Add_CenterLocationOffset = FVector::ZeroVector;

	/** Additive Hand Location Offset (Right, Local Space) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HandCRRuntimeData")
	FVector Add_HandLocOffsetR_Local = FVector::ZeroVector;

	/** Additive Hand Location Offset (Left, Local Space) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HandCRRuntimeData")
	FVector Add_HandLocOffsetL_Local = FVector::ZeroVector;
	
	/** Additive Hand Rotation Offset (Right, Local Space) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HandCRRuntimeData")
	FRotator Add_HandRotOffsetR_Local = FRotator::ZeroRotator;

	/** Additive Hand Rotation Offset (Left, Local Space) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HandCRRuntimeData")
	FRotator Add_HandRotOffsetL_Local = FRotator::ZeroRotator;
};