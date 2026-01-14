#pragma once
#include "IFPHandAnimComputeTemplate.h"
#include "Kismet/KismetMathLibrary.h"

struct FAnimComputation_Pistol : public IFPHandAnimComputeTemplate
{
	void Compute(const FFPHandAnimDataInput& In, FFPHandAnimDataOutput& Out, FFPHandAnimDataOutput CachedLastOutput) const override
	{
		// Base Param
		float GroundSpeed = In.Velocity.Size2D();
		
		float NormalSpeed = FMath::GetMappedRangeValueClamped(
			FVector2D(0.f, 300.f),
			FVector2D(0.f, 1.f),
			GroundSpeed
		);

		const FVector LocalVelocity = In.ActorTransform.InverseTransformVectorNoScale(In.Velocity);
		
		float NormalSpeedFB = FMath::GetMappedRangeValueClamped(
			FVector2D(-300.f, 300.f),
			FVector2D(-1.f, 1.f),
			LocalVelocity.X
		);
		float NormalSpeedLR = FMath::GetMappedRangeValueClamped(
			FVector2D(-300.f, 300.f),
			FVector2D(-1.f, 1.f),
			LocalVelocity.Y
		);
		
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

		/** Additive Global Offset
		float UpdateDuration = In.UpdateDuration;
		const float X = FMath::Lerp(0.f, -40.f, NormalSpeed);
		const float Y = FMath::Lerp(0.f,  20.f, NormalSpeed);

		const float Perlin = FMath::PerlinNoise1D(UpdateDuration * 3.f);
		const float SinVal = FMath::Sin((UpdateDuration + 233.f) * 10.f);
		
		const float Mixed  = FMath::Lerp(Perlin, SinVal, 0.75f);
		const float Noise01 = (Mixed + 1.f) / 2.f;               
		const float NoiseOffset = FMath::Lerp(-20.f, -15.f, Noise01);

		const float AimScaled = NormalAim * 5.f;
		const float Z = FMath::Lerp(AimScaled, NoiseOffset, NormalSpeed);
		const FVector TargetOffset(X, Y, Z);

		Out.Add_CenterLocationOffset = FMath::VInterpTo(
			CachedLastOutput.Add_CenterLocationOffset,
			TargetOffset,
			In.DeltaTime,
			10.f
		); */

		/** Additive Global Rot Offset */

		const float LookYawSpeed   = In.LookRotSpeed.X; // deg/s
		const float LookPitchSpeed = In.LookRotSpeed.Y;
		
		const float GlobalRotRoll = FMath::Clamp(LookYawSpeed * 0.01f,-3.f, 3.f);
		const float GlobalRotYaw = FMath::Clamp(LookYawSpeed * 0.01f,-2.f, 2.f);
		const float GlobalRotPitch  = -FMath::Clamp(LookPitchSpeed * 0.02f,-5.f, 5.f);

		const float NewRoll = UKismetMathLibrary::FloatSpringInterp(
					CachedLastOutput.Add_CenterRotationWS.Pitch,
					GlobalRotRoll,
					GlobalRotOffsetSS_Roll,
					400.f,              // Stiffness
					0.4f,              // CriticalDampingFactor
					In.DeltaTime,                 // DeltaTime
					0.6f,                // Mass
					5.f                 // TargetVelocityAmount
				);

		const float NewYaw = UKismetMathLibrary::FloatSpringInterp(
					CachedLastOutput.Add_CenterRotationWS.Yaw,
					GlobalRotYaw,
					GlobalRotOffsetSS_Yaw,
					400.f,              // Stiffness
					0.4f,              // CriticalDampingFactor
					In.DeltaTime,                 // DeltaTime
					0.6f,                // Mass
					5.f                 // TargetVelocityAmount
				);
		
		const float NewPitch = UKismetMathLibrary::FloatSpringInterp(
					CachedLastOutput.Add_CenterRotationWS.Roll,
					GlobalRotPitch,
					GlobalRotOffsetSS_Pitch,
					400.f,              // Stiffness
					0.4f,              // CriticalDampingFactor
					In.DeltaTime,                 // DeltaTime
					0.6f,                // Mass
					5.f                 // TargetVelocityAmount
				);


		
		const FRotator TargetRot(
			NewRoll,
			GlobalRotYaw,
			GlobalRotPitch
		);

		Out.Add_CenterRotationWS = TargetRot;

		/**Out.Add_CenterRotationWS = FMath::RInterpTo(
			CachedLastOutput.Add_CenterRotationWS,
			TargetRot,
			In.DeltaTime,
			30.f
		);*/
		
		/**Final Spring Interp
		const float CurYaw = Out.Add_CenterLocationOffset.X;

		const float NewYaw = UKismetMathLibrary::FloatSpringInterp(
			CurYaw,
			In.LookInput.X * 0.1f,
			HandOffsetSS_Yaw,   // 需要持久化
			400.f,              // Stiffness
			0.75f,              // CriticalDampingFactor
			In.DeltaTime,                 // DeltaTime
			1.f,                // Mass
			2.f                 // TargetVelocityAmount
		);

		const float TargetPitch = In.LookInput.Y * 0.5f;
		const float NewPitch = UKismetMathLibrary::FloatSpringInterp(
			NewYaw,             // ⚠️ 注意：这里是“第一个 spring 的输出”作为 Current（按图）
			TargetPitch,
			HandOffsetSS_Pitch,
			300.f,              // Stiffness
			0.85f,              // CriticalDampingFactor
			In.DeltaTime,
			1.f,                // Mass
			2.f                 // TargetVelocityAmount
		);

		Out.Add_CenterLocationOffset.X = NewYaw;
		Out.Add_CenterLocationOffset.Z = NewPitch;
		Out.Add_CenterLocationOffset.Y = 0.f;  */
	}

	mutable FFloatSpringState GlobalRotOffsetSS_Yaw;
	mutable FFloatSpringState GlobalRotOffsetSS_Roll;
	mutable FFloatSpringState GlobalRotOffsetSS_Pitch;
	
	mutable FFloatSpringState HandOffsetSS_Yaw;
	mutable FFloatSpringState HandOffsetSS_Pitch;
};
