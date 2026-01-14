// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/AC_ProceduralFPHandManager.h"

#include "ComputeTemplate/AnimComputation_Rifle.h"
#include "Core/FPHandAnimWorker.h"
#include "Curves/CurveVector.h"
#include "Data/DA_FPHandAnimConfig.h"


// Sets default values for this component's properties
UAC_ProceduralFPHandManager::UAC_ProceduralFPHandManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UAC_ProceduralFPHandManager::BeginPlay()
{
	Super::BeginPlay();

	// ...
	Worker = MakeUnique<FFPHandAnimWorker>(WorkerFrequency);
	Thread = FRunnableThread::Create(Worker.Get(), TEXT("FPHandProcAnimWorker"));
}

void UAC_ProceduralFPHandManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SetComponentTickEnabled(false);
	StopWorker();
	Super::EndPlay(EndPlayReason);
}


// Called every frame
void UAC_ProceduralFPHandManager::TickComponent(float DeltaTime, ELevelTick TickType,
                                               FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bEnableInterp)
	{
		MainInterpFunc(DeltaTime);
	}

	if (bHandAnimEnabled)
	{
		CalculateLagParam(DeltaTime);
		CalculateAdditiveParam(DeltaTime);

		Worker->PushInput(GetInputData(DeltaTime));

		FFPHandAnimDataOutput OutputData;
		if (Worker->TryPopOutput(OutputData))
		{
			
		}
	}
}

void UAC_ProceduralFPHandManager::SetFPHandConfig_Implementation(UDA_FPHandAnimConfig* InConfig, bool bIsAutoEnable)
{
	if (!InConfig)
	{
		UE_LOG(LogTemp,Error,TEXT("Error: [ProceduralFPHandManager] SetFPHandConfig_Implementation - InConfig is not valid! "))
	}
	
	if (CurrentConfig == InConfig)
	{
		return;
	}

	OnConfigChanged(InConfig, bIsAutoEnable);

	OnConfigChangedEvent.Broadcast(InConfig);
}

UDA_FPHandAnimConfig* UAC_ProceduralFPHandManager::GetFPHandConfig_Implementation()
{
	return CurrentConfig;
}

FFPHandCRRuntimeData UAC_ProceduralFPHandManager::GetRuntimeFPHandData_Implementation()
{
	return CurrentCRRuntimeData;
}

FFPHandAnimDataInput UAC_ProceduralFPHandManager::GetInputData_Implementation(float DeltaTime)
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

	// 1) LookInput（你已有）
	In.LookRotSpeed = LookInput;

	// 2) Owner / Pawn
	AActor* OwnerActor = GetOwner();
	if (!IsValid(OwnerActor))
	{
		return In;
	}

	// 3) Velocity（通常用 OwnerActor）
	In.Velocity = OwnerActor->GetVelocity();

	// 4) ControlRotation（通常从 Pawn/Controller）
	if (const APawn* Pawn = Cast<APawn>(OwnerActor))
	{
		if (const AController* Controller = Pawn->GetController())
		{
			In.ControlRotation = Controller->GetControlRotation();
		}
		else
		{
			// 没 Controller 时，退化为 Actor Rotation
			In.ControlRotation = OwnerActor->GetActorRotation();
		}
	}
	else
	{
		In.ControlRotation = OwnerActor->GetActorRotation();
	}

	return In;
}

void UAC_ProceduralFPHandManager::OnConfigChanged(UDA_FPHandAnimConfig* InConfig, bool bIsAutoEnable)
{
	TSharedPtr<const IFPHandAnimComputeTemplate, ESPMode::ThreadSafe> NewTemplate;
	switch (InConfig->HandAnimCalculationTemplate)
	{
		case EFPHandAnimTemplate::Fist:
			{
				break;
			}
		case EFPHandAnimTemplate::Rifle:
			{
				auto Rifle = MakeShared<FAnimComputation_Rifle, ESPMode::ThreadSafe>();
				NewTemplate = Rifle;
				break;
			}
		case EFPHandAnimTemplate::Pistol:
			{
				break;
			}
		case EFPHandAnimTemplate::Prybar:
			{
				break;
			}
		case EFPHandAnimTemplate::CommonItemOneHand:
			{
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
		UE_LOG(LogTemp, Error, TEXT("Error [UAC_ProceduralFPHandManager::OnConfigChanged] HandSwayLocCurve or HandSwayRotCurve is not valid"))
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

void UAC_ProceduralFPHandManager::InterpFPHandAnimData()
{
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
		UE_LOG(LogTemp,Error,TEXT("Error [UAC_ProceduralFPHandManager::InterpFPHandAnimData] - AnimSwitchInterpCurve is not valid"));

		return;
	}
	
	InterpCurve = CurrentConfig->AnimSwitchInterpCurve;
	bEnableInterp = true;
	OnFPHandAnimStartInterp.Broadcast();
}

void UAC_ProceduralFPHandManager::MainInterpFunc(float InDeltaTime)
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

void UAC_ProceduralFPHandManager::CalculateLagParam(float DeltaTime)
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

void UAC_ProceduralFPHandManager::CalculateAdditiveParam(float DeltaTime)
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

float UAC_ProceduralFPHandManager::GetCurveLength(UCurveBase* InCurve)
{
	float MinTime = 0.f;
	float MaxTime = 0.f;
	InCurve->GetTimeRange(MinTime, MaxTime);
	return MaxTime - MinTime;
}

void UAC_ProceduralFPHandManager::StopWorker()
{
	if (!Thread) return;

	Worker->Stop();
	Thread->WaitForCompletion();

	delete Thread;
	Thread = nullptr;
	Worker.Reset();
}

