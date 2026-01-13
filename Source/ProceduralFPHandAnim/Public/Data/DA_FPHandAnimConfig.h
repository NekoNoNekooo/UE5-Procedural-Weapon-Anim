// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enum_FPHandAnimMode.h"
#include "Enum_FPHandAnimTemplate.h"
#include "Struct_CHTParam_ProcedualFPHand.h"
#include "Struct_FPHandCRStaticData.h"
#include "Engine/DataAsset.h"
#include "DA_FPHandAnimConfig.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class PROCEDURALFPHANDANIM_API UDA_FPHandAnimConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	
	// ==================== 静态情况下的手部偏移配置， 用于校准Pose在摄像机内的表现 ====================

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFPHandCRStaticData StaticData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> AnimLayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFPHandAnimTemplate HandAnimCalculationTemplate;
	
	/** Hand Pose */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequence> Pose;

	/** 手部动画模式 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFPHandAnimMode HandAnimMode;
	
	/** Hand Location Offset (Right) - Camera Space */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset")
	FVector HandOffsetR = FVector::ZeroVector;

	/** Hand Location Offset (Left) - Camera Space */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset")
	FVector HandOffsetL = FVector::ZeroVector;
	
	/** Hand Location Offset (Right) - Camera Space */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset")
    FRotator HandOffsetR_Rot = FRotator::ZeroRotator;
    
    /** Hand Location Offset (Left) - Camera Space */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset")
    FRotator HandOffsetL_Rot = FRotator::ZeroRotator;

	/** Center Offset - Camera Space */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset")
	FVector CenterOffset = FVector::ZeroVector;

	// ==================== Hand Additive Anim的配置 ====================
	
	/**
	 * 手部摇晃动画位置偏移曲线。
	 *
	 * - 曲线时间轴为归一化时间 [0 - 1]，表示一个完整的摇晃周期
	 * - XYZ 表示摄像机空间下的骨骼偏移方向
	 * - 曲线值表示偏移幅度
	 * - 通过调整 AdditiveAnimSpeedScale 和 LocationOffsetAmplitudeScale 的参数来调整手臂摆动循环动画的速度以及幅度
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Additive Anim")
	TObjectPtr<UCurveVector> HandSwayCurve_Location;

	/**
	 * 手部摇晃动画旋转偏移曲线。
	 *
	 * - 曲线时间轴为归一化时间 [0 - 1]，表示一个完整的摇晃周期
	 * - XYZ 表示摄像机空间下的Pitch Roll Yaw偏移
	 * - 曲线值表示偏移幅度
	 * - 通过调整 AdditiveAnimSpeedScale 和 RotationOffsetAmplitudeScale 的参数来调整手臂摆动循环动画的速度以及幅度
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand Additive Anim")
	TObjectPtr<UCurveVector> HandSwayCurve_Rotation;

	/** 手部摇晃程序动画的时间长度缩放 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.01", UIMin = "0.01", UIMax = "10"), Category = "Hand Additive Anim")
	float AdditiveAnimSpeedScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.01", UIMin = "0.01", UIMax = "10"), Category = "Hand Additive Anim")
	float LocationOffsetAmplitudeScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.01", UIMin = "0.01", UIMax = "10"), Category = "Hand Additive Anim")
	float RotationOffsetAmplitudeScale = 1.0f;

	// ==================== 手部位置迟滞插值配置 ====================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interpolation|Lag")
	float HandLocationInterpSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interpolation|Lag")
	float HandRotationInterpSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interpolation")
	TObjectPtr<UCurveFloat> AnimSwitchInterpCurve;
};
