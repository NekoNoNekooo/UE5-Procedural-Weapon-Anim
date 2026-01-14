// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/ProceduralWeaponAnimSubsystem.h"

#include "ComputeTemplate/AnimComputation_CommonItemOneHand.h"
#include "ComputeTemplate/AnimComputation_Fist.h"
#include "ComputeTemplate/AnimComputation_Pistol.h"
#include "ComputeTemplate/AnimComputation_Prybar.h"
#include "ComputeTemplate/AnimComputation_Rifle.h"
#include "Curves/CurveVector.h"
#include "GameFramework/Character.h"
#include "Data/DA_FPHandAnimConfig.h"

void UProceduralWeaponAnimSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UProceduralWeaponAnimSubsystem::Deinitialize()
{
	StopWorker();
	Super::Deinitialize();
}

void UProceduralWeaponAnimSubsystem::Tick(float DeltaTime)
{
	if (OwnerPlayerController != nullptr)
	{
		LastControllerRot = CurrentControllerRot;
		CurrentControllerRot = OwnerPlayerController->GetControlRotation();
		FRotator DeltaRot = (CurrentControllerRot - LastControllerRot);
		RotSpeed = DeltaRot * (1.f / DeltaTime);
		SmoothedDeltaRot = FMath::RInterpTo(SmoothedDeltaRot, CurrentControllerRot-LastControllerRot,DeltaTime,30.f);
	}
	
	if (bIsHandAnimEnabled && Thread)
	{
		Worker->PushInput(GetInputData(DeltaTime));

		FFPHandAnimDataOutput Latest;
		bool bHas = false;
		FFPHandAnimDataOutput Temp;
		while (Worker->TryPopOutput(Temp))
		{
			Latest = Temp;
			bHas = true;
		}
		if (bHas)
		{
			AdditiveAnimDataOutput = Latest;
		}

		UpdateDuration = UpdateDuration + DeltaTime;

		SmoothedAdditiveAnimDataOutput = SmoothHandAnimOutput(
			SmoothedAdditiveAnimDataOutput,
			AdditiveAnimDataOutput,
			DeltaTime,
			LocInterpSpeed,
			RotInterpSpeed
		);
	}

	/**
	if (bEnableInterp)
	{
		MainInterpFunc(DeltaTime);
	}

	if (bHandAnimEnabled)
	{
		CalculateLagParam(DeltaTime);
		CalculateAdditiveParam(DeltaTime);
	} */
}

TStatId UProceduralWeaponAnimSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UProceduralWeaponAnimSubsystem, STATGROUP_Tickables);
}

bool UProceduralWeaponAnimSubsystem::IsTickable() const
{
	return true;
}

void UProceduralWeaponAnimSubsystem::SetFPHandConfig_Implementation(UDA_FPHandAnimConfig* InConfig, bool bIsAutoEnable)
{
	if (!InConfig)
	{
		UE_LOG(LogTemp, Error, TEXT("Error: [ProceduralWeaponAnimSubsystem] SetFPHandConfig_Implementation - InConfig is not valid! "))
	}
	
	if (CurrentConfig == InConfig)
	{
		return;
	}

	OnConfigChanged(InConfig, bIsAutoEnable);
	OnConfigChangedEvent.Broadcast(InConfig);
}

void UProceduralWeaponAnimSubsystem::DisableFPHandAnim_Implementation()
{
	if (!bHandAnimEnabled)
	{
		return;
	}

	bHandAnimEnabled = false;
	OnFPHandAnimStateChanged.Broadcast(false);
}

void UProceduralWeaponAnimSubsystem::EnableFPHandAnim_Implementation()
{
	if (bHandAnimEnabled)
	{
		return;
	}

	bHandAnimEnabled = true;
	OnFPHandAnimStateChanged.Broadcast(true);
	InterpFPHandAnimData();
}

UDA_FPHandAnimConfig* UProceduralWeaponAnimSubsystem::GetFPHandConfig_Implementation()
{
	return CurrentConfig;
}

FFPHandCRRuntimeData UProceduralWeaponAnimSubsystem::GetRuntimeFPHandData_Implementation()
{
	return CurrentCRRuntimeData;
}

bool UProceduralWeaponAnimSubsystem::InitAndStart(ACharacter* InCharacter, APlayerController* InPlayerController)
{
	if (bIsHandAnimEnabled || Thread || Worker.IsValid())
	{
		return false;
	}
	
	if (!IsValid(InCharacter) || !IsValid(InPlayerController))
	{
		return false;
	}
	
	UWorld* World = GetWorld();
	if (!World || World->bIsTearingDown)
	{
		return false;
	}
	
	OwnerCharacter = InCharacter;
	OwnerPlayerController = InPlayerController;
	
	Worker = MakeUnique<FFPHandAnimWorker>(WorkerFrequency);
	Thread = FRunnableThread::Create(Worker.Get(), TEXT("FPHandProcAnimWorker"));

	if (!Thread)
	{
		Worker.Reset();
		OwnerCharacter.Reset();
		OwnerPlayerController.Reset();
		return false;
	}

	bIsHandAnimEnabled = true;
	return true;
}

bool UProceduralWeaponAnimSubsystem::StopAndDeinit()
{
	if (bIsHandAnimEnabled)
	{
		StopWorker();
		bIsHandAnimEnabled = false;
		return true;
	}

	return false;
}

void UProceduralWeaponAnimSubsystem::SetLookInput(FVector2D InLookInput)
{
	LookInput = InLookInput;
}

FFPHandAnimDataInput UProceduralWeaponAnimSubsystem::GetInputData(float DeltaTime)
{
	FFPHandAnimDataInput In;

	UWorld* World = GetWorld();
	if (!World || World->bIsTearingDown)
	{
		// 返回默认输入，避免 PIE 结束崩溃
		In.DeltaTime = DeltaTime;
		return In;
	}
	
	In.TimeSec = World->GetTimeSeconds();
	In.DeltaTime = DeltaTime;

	// 1) LookInput

	In.LookRotSpeed.X = RotSpeed.Yaw;
	In.LookRotSpeed.Y = RotSpeed.Pitch;

	AActor* OwnerActor = OwnerCharacter.Get();
	if (!IsValid(OwnerActor))
	{
		return In;
	}

	// 3) Velocity
	In.Velocity = OwnerActor->GetVelocity();

	// 4) ControlRotation
	if (OwnerPlayerController.IsValid())
	{
		In.ControlRotation = OwnerPlayerController->GetControlRotation();
	}

	In.ActorTransform = OwnerActor->GetActorTransform();
	In.UpdateDuration = UpdateDuration;
	
	return In;
}

bool UProceduralWeaponAnimSubsystem::IsHandAnimEnabled() const
{
	return bHandAnimEnabled;
}

double UProceduralWeaponAnimSubsystem::GetWorkerFrequency() const
{
	return WorkerFrequency;
}

FFPHandCRRuntimeData UProceduralWeaponAnimSubsystem::GetCurrentCRRuntimeData() const
{
	return CurrentCRRuntimeData;
}

FFPHandCRRuntimeData UProceduralWeaponAnimSubsystem::GetTargetCRRuntimeData() const
{
	return TargetCRRuntimeData;
}

FVector2D UProceduralWeaponAnimSubsystem::GetLookInput() const
{
	return LookInput;
}

void UProceduralWeaponAnimSubsystem::OnConfigChanged(UDA_FPHandAnimConfig* InConfig, bool bIsAutoEnable)
{
	if (!Worker)
	{
		return;
	}

	TSharedPtr<const IFPHandAnimComputeTemplate, ESPMode::ThreadSafe> NewTemplate;
	switch (InConfig->HandAnimCalculationTemplate)
	{
		case EFPHandAnimTemplate::Fist:
			{
				auto Fist = MakeShared<FAnimComputation_Fist, ESPMode::ThreadSafe>();
				NewTemplate = Fist;
			}
		case EFPHandAnimTemplate::Rifle:
			{
				auto Rifle = MakeShared<FAnimComputation_Rifle, ESPMode::ThreadSafe>();
				NewTemplate = Rifle;
				break;
			}
		case EFPHandAnimTemplate::Pistol:
			{
				auto Pistol = MakeShared<FAnimComputation_Pistol, ESPMode::ThreadSafe>();
				NewTemplate = Pistol;
				break;
			}
		case EFPHandAnimTemplate::Prybar:
			{
				auto Prybar = MakeShared<FAnimComputation_Prybar, ESPMode::ThreadSafe>();
				NewTemplate = Prybar;
				break;
			}
		case EFPHandAnimTemplate::CommonItemOneHand:
			{
				auto CIOH = MakeShared<FAnimComputation_CommonItemOneHand, ESPMode::ThreadSafe>();
				NewTemplate = CIOH;
				break;
			}
		default: 
			break;
	}

	Worker->SetTemplate(NewTemplate);
	
	CurrentConfig = InConfig;
	
	HandSwayLocCurve = CurrentConfig->HandSwayCurve_Location;
	HandSwayRotCurve = CurrentConfig->HandSwayCurve_Rotation;
	
	if (!HandSwayLocCurve || !HandSwayRotCurve)
	{
		CurrentCRRuntimeData.Add_HandLocOffsetL_Local = FVector::ZeroVector;
		CurrentCRRuntimeData.Add_HandRotOffsetL_Local = FRotator::ZeroRotator;
		UE_LOG(LogTemp, Error, TEXT("Error [UProceduralWeaponAnimSubsystem::OnConfigChanged] HandSwayLocCurve or HandSwayRotCurve is not valid"))
		return;
	}
	
	AddAnimSpeedScale = CurrentConfig->AdditiveAnimSpeedScale;
	LocOffsetAmplitudeScale = CurrentConfig->LocationOffsetAmplitudeScale;
	RotOffsetAmplitudeScale = CurrentConfig->RotationOffsetAmplitudeScale;
	LocCurveLength = GetCurveLength(HandSwayLocCurve);
	SwayPhase = 0.f;
	
	if (bIsAutoEnable && !bHandAnimEnabled)
	{
		bHandAnimEnabled = true;
		OnFPHandAnimStateChanged.Broadcast(true);
		InterpFPHandAnimData();
	}
	else if (bHandAnimEnabled)
	{
		InterpFPHandAnimData();
	}
}

void UProceduralWeaponAnimSubsystem::InterpFPHandAnimData()
{
	if (!CurrentConfig)
	{
		return;
	}

	InterpStartCRRuntimeData = CurrentCRRuntimeData;
	
	FFPHandCRRuntimeData NewData;
	NewData.HandOffsetR = CurrentConfig->HandOffsetR;
	NewData.HandOffsetL = CurrentConfig->HandOffsetL;
	NewData.CenterOffset = CurrentConfig->CenterOffset;
	NewData.LockLeftHand = 1.0f;
	
	TargetCRRuntimeData = NewData;


	//TODO::
	//这里需要把切换动作的Interp参数写成可配置的
	InterpDuration = 1.0f;
	ElapsedTime = 0.f;
	
	if (!IsValid(CurrentConfig->AnimSwitchInterpCurve))
	{
		UE_LOG(LogTemp, Error, TEXT("Error [UProceduralWeaponAnimSubsystem::InterpFPHandAnimData] - AnimSwitchInterpCurve is not valid"));

		return;
	}
	
	InterpCurve = CurrentConfig->AnimSwitchInterpCurve;
	bEnableInterp = true;
	OnFPHandAnimStartInterp.Broadcast();
}

void UProceduralWeaponAnimSubsystem::MainInterpFunc(float InDeltaTime)
{
	ElapsedTime += InDeltaTime;

	const float TimeRatio = FMath::Clamp(ElapsedTime / InterpDuration, 0.f, 1.f);
	float MinTime = 0.f;
	float MaxTime = 0.f;
	InterpCurve->GetTimeRange(MinTime, MaxTime);
	float CurveTime = TimeRatio * (MaxTime - MinTime) + MinTime;
	const float Value = InterpCurve->GetFloatValue(CurveTime);

	if (TimeRatio >= 1.f)
	{
		bEnableInterp = false;

		OnFPHandAnimStopInterp.Broadcast();
	}

	//TODO::
	//这里最终传给Control Rig的参数还没写完
	CurrentCRRuntimeData.HandOffsetR =
		FMath::Lerp(InterpStartCRRuntimeData.HandOffsetR, TargetCRRuntimeData.HandOffsetR, Value);

	CurrentCRRuntimeData.HandOffsetL =
		FMath::Lerp(InterpStartCRRuntimeData.HandOffsetL, TargetCRRuntimeData.HandOffsetL, Value);

	CurrentCRRuntimeData.CenterOffset =
		FMath::Lerp(InterpStartCRRuntimeData.CenterOffset, TargetCRRuntimeData.CenterOffset, Value);
}

void UProceduralWeaponAnimSubsystem::CalculateLagParam(float DeltaTime)
{
	float RightOffset = 0.0f;
	float UpOffset = 0.0f;

	RightOffset = LookInput.X * 1.0f;
	UpOffset = LookInput.Y * 1.0f;

	//TODO::
	//Interp Speed需要参数化
	FVector TargetOffset = FVector(RightOffset,0.0f, UpOffset);
	CurrentCRRuntimeData.Add_CenterLocationOffset = FMath::VInterpTo(CurrentCRRuntimeData.Add_CenterLocationOffset, TargetOffset, DeltaTime, 30.0f);
}

void UProceduralWeaponAnimSubsystem::CalculateAdditiveParam(float DeltaTime)
{
	//1. Calculate Hand Sway Param
	if (SwayPhase > 1)
	{
		SwayPhase = FMath::Fmod(SwayPhase, 1.0f);
	}

	float LocCurveMinX = 0.f;
	float LocCurveMaxX = 1.f;
	
	HandSwayLocCurve->GetTimeRange(LocCurveMinX, LocCurveMaxX);
	
	const float CurveTime = FMath::Lerp(LocCurveMinX, LocCurveMaxX, SwayPhase);

	CurrentCRRuntimeData.Add_HandLocOffsetL_Local = HandSwayLocCurve->GetVectorValue(CurveTime) * LocOffsetAmplitudeScale;

	float RotCurveMinX = 0.f;
	float RotCurveMaxX = 1.f;
	
	HandSwayRotCurve->GetTimeRange(RotCurveMinX, RotCurveMaxX);
	const FVector RotVec = HandSwayRotCurve->GetVectorValue(CurveTime);
	const float RotPitch = RotVec.X;
	const float RotRoll = RotVec.Y;
	const float RotYaw = RotVec.Z;
	CurrentCRRuntimeData.Add_HandRotOffsetL_Local = FRotator(RotPitch, RotYaw, RotRoll) * RotOffsetAmplitudeScale;
	
	//Final. Add Phase Value
	SwayPhase += DeltaTime*AddAnimSpeedScale;
}

float UProceduralWeaponAnimSubsystem::GetCurveLength(UCurveBase* InCurve)
{
	float MinTime = 0.f;
	float MaxTime = 0.f;
	InCurve->GetTimeRange(MinTime, MaxTime);
	return MaxTime - MinTime;
}

void UProceduralWeaponAnimSubsystem::StopWorker()
{
	if (!Thread) return;

	Worker->Stop();
	Thread->WaitForCompletion();

	delete Thread;
	Thread = nullptr;
	Worker.Reset();

	UpdateDuration = 0.f;
}

FFPHandAnimDataOutput UProceduralWeaponAnimSubsystem::SmoothHandAnimOutput(const FFPHandAnimDataOutput& CurrentSmooth,
	const FFPHandAnimDataOutput& TargetRaw, float DeltaTime, float InLocInterpSpeed, float InRotInterpSpeed)
{
	FFPHandAnimDataOutput Out = CurrentSmooth;

	// Center
	Out.Add_CenterLocationOffset = FMath::VInterpTo(
		CurrentSmooth.Add_CenterLocationOffset,
		TargetRaw.Add_CenterLocationOffset,
		DeltaTime,
		InLocInterpSpeed);

	Out.Add_CenterRotationWS = FMath::RInterpTo(
		CurrentSmooth.Add_CenterRotationWS,
		TargetRaw.Add_CenterRotationWS,
		DeltaTime,
		InRotInterpSpeed);

	// Hands - Location
	Out.Add_HandLocOffsetR_Local = FMath::VInterpTo(
		CurrentSmooth.Add_HandLocOffsetR_Local,
		TargetRaw.Add_HandLocOffsetR_Local,
		DeltaTime,
		InLocInterpSpeed);

	Out.Add_HandLocOffsetL_Local = FMath::VInterpTo(
		CurrentSmooth.Add_HandLocOffsetL_Local,
		TargetRaw.Add_HandLocOffsetL_Local,
		DeltaTime,
		InLocInterpSpeed);

	// Hands - Rotation
	Out.Add_HandRotOffsetR_Local = FMath::RInterpTo(
		CurrentSmooth.Add_HandRotOffsetR_Local,
		TargetRaw.Add_HandRotOffsetR_Local,
		DeltaTime,
		InRotInterpSpeed);

	Out.Add_HandRotOffsetL_Local = FMath::RInterpTo(
		CurrentSmooth.Add_HandRotOffsetL_Local,
		TargetRaw.Add_HandRotOffsetL_Local,
		DeltaTime,
		InRotInterpSpeed);

	return Out;
}
