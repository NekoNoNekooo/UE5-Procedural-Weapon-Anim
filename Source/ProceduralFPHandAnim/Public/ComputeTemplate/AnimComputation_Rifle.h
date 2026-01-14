#pragma once
#include "IFPHandAnimComputeTemplate.h"
#include "Kismet/KismetMathLibrary.h"

struct FAnimComputation_Rifle : public IFPHandAnimComputeTemplate
{
	void Compute(const FFPHandAnimDataInput& In, FFPHandAnimDataOutput& Out, FFPHandAnimDataOutput CachedLastOutput) const override
	{
		const FRotator ControlRot = In.ControlRotation;
		const float Pitch = ControlRot.Pitch;
		float SelectedPitch;
		if (Pitch > 180.f)
		{
			SelectedPitch = Pitch - 360.f;
		}
		else
		{
			SelectedPitch = Pitch;
		}
		
		const float NormalAim = FMath::GetMappedRangeValueClamped(
			FVector2D(80.f, -80.f),
			FVector2D(-1.f, 1.f),
			SelectedPitch
		);


		// Hand Root Offset
		const float LookYawSpeed   = In.LookRotSpeed.X; // deg/s
		const float LookPitchSpeed = In.LookRotSpeed.Y;
		
		const float GlobalRotRoll = FMath::Clamp(LookYawSpeed * 0.01f,-4.f, 4.f);
		const float GlobalRotYaw = FMath::Clamp(LookYawSpeed * 0.01f,-2.f, 2.f);
		const float GlobalRotPitch  = -FMath::Clamp(LookPitchSpeed * 0.01f,-2.f, 2.f) - (NormalAim * 5.0f);

		const float NewRoll = UKismetMathLibrary::FloatSpringInterp(
					CachedLastOutput.Add_CenterRotationWS.Pitch,
					GlobalRotRoll,
					GlobalRotOffsetSS_Roll,   // 需要持久化
					300.f,              // Stiffness
					0.4f,              // CriticalDampingFactor
					In.DeltaTime,                 // DeltaTime
					0.8f,                // Mass
					5.f                 // TargetVelocityAmount
				);

		const float NewYaw = UKismetMathLibrary::FloatSpringInterp(
					CachedLastOutput.Add_CenterRotationWS.Yaw,
					GlobalRotYaw,
					GlobalRotOffsetSS_Yaw,   // 需要持久化
					400.f,              // Stiffness
					0.8f,              // CriticalDampingFactor
					In.DeltaTime,                 // DeltaTime
					0.5f,                // Mass
					2.f                 // TargetVelocityAmount
				);
		
		const float NewPitch = UKismetMathLibrary::FloatSpringInterp(
					CachedLastOutput.Add_CenterRotationWS.Roll,
					GlobalRotPitch,
					GlobalRotOffsetSS_Pitch,   // 需要持久化
					400.f,              // Stiffness
					1.f,              // CriticalDampingFactor
					In.DeltaTime,                 // DeltaTime
					0.5f,                // Mass
					2.f                 // TargetVelocityAmount
				);

		const FRotator TargetRot(NewRoll, NewYaw,NewPitch);

		Out.Add_CenterRotationWS = TargetRot;
		
		const float TargetLocLR = FMath::Clamp(-LookYawSpeed   * 0.0020f, -2.f, 2.f); // 左右
		const float TargetLocTB = FMath::Clamp( LookPitchSpeed * 0.0015f, -2.f, 2.f) + (NormalAim * 5); // 上下

		const float NewLocLR = UKismetMathLibrary::FloatSpringInterp(
			CachedLastOutput.Add_CenterLocationOffset.X, // 你需要在 Output 里加这个字段，或用已有 loc
			TargetLocLR,
			GlobalLocOffsetSS_LR,
			200.f,    // LocStiffness（比旋转低）
			0.9f,     // LocDamping（更稳）
			In.DeltaTime,
			1.0f,
			1.0f      //
		);

		const float NewLocTB = UKismetMathLibrary::FloatSpringInterp(
			CachedLastOutput.Add_CenterLocationOffset.Z, // 你需要在 Output 里加这个字段，或用已有 loc
			TargetLocTB,
			GlobalLocOffsetSS_TB,
			200.f,    // LocStiffness（比旋转低）
			0.9f,     // LocDamping（更稳）
			In.DeltaTime,
			1.0f,
			1.0f      // 位置一般不需要 TVA，避免“冲”
		);

		const FVector TargetLoc(NewLocLR, 0.f, NewLocTB);
		Out.Add_CenterLocationOffset = TargetLoc;
	}

private:

	FVector2f SmoothedRotSpeed = FVector2f::ZeroVector;
	
	mutable FFloatSpringState GlobalRotOffsetSS_Yaw;
	mutable FFloatSpringState GlobalRotOffsetSS_Roll;
	mutable FFloatSpringState GlobalRotOffsetSS_Pitch;
	
	mutable FFloatSpringState GlobalLocOffsetSS_LR;
	mutable FFloatSpringState GlobalLocOffsetSS_TB;
};
