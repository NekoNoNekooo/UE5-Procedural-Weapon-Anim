#pragma once

#include "CoreMinimal.h"
#include "Data/Struct_FPHandCRRuntimeData.h"
#include "Struct_CHTParam_ProcedualFPHand.generated.h"

class UCurveVector;

USTRUCT(BlueprintType)
struct FCHTInput_FPHand
{
	GENERATED_BODY()

public:

	
};

USTRUCT(BlueprintType)
struct FCHTOutput_FPHand
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimationAsset> Pose;
	
	/**
	 * 手部摇晃动画位置偏移曲线。
	 *
	 * - 曲线时间轴为归一化时间 [0 - 1]，表示一个完整的摇晃周期
	 * - XYZ 表示摄像机空间下的骨骼偏移方向
	 * - 曲线值表示偏移幅度
	 * - 通过调整 AdditiveAnimSpeedScale 和 LocationOffsetAmplitudeScale 的参数来调整手臂摆动循环动画的速度以及幅度
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UCurveVector> HandSwayCurve_Location;

	/**
	 * 手部摇晃动画旋转偏移曲线。
	 *
	 * - 曲线时间轴为归一化时间 [0 - 1]，表示一个完整的摇晃周期
	 * - XYZ 表示摄像机空间下的Pitch Roll Yaw偏移
	 * - 曲线值表示偏移幅度
	 * - 通过调整 AdditiveAnimSpeedScale 和 RotationOffsetAmplitudeScale 的参数来调整手臂摆动循环动画的速度以及幅度
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UCurveVector> HandSwayCurve_Rotation;

	/** 手部摇晃程序动画的时间长度缩放 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.01", UIMin = "0.01", UIMax = "10"))
	float AdditiveAnimSpeedScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.01", UIMin = "0.01", UIMax = "10"))
	float LocationOffsetAmplitudeScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.01", UIMin = "0.01", UIMax = "10"))
	float RotationOffsetAmplitudeScale = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFPHandCRRuntimeData OutFPHandCRData;
};