// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralFPHandManagerInterface.h"
#include "Data/Struct_FPHandCRRuntimeData.h"
#include "Core/FPHandAnimWorker.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "ProceduralWeaponAnimSubsystem.generated.h"

class UCurveVector;
class UDA_FPHandAnimConfig;
class UCurveFloat;
class UCurveBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConfigChangedEventNew, UDA_FPHandAnimConfig*, UpdatedConfig);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnimStateChangedEventNew, bool, Enabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFPHandAnimInterpEventNew);

UCLASS(BlueprintType, meta = (Keywords = "PWA"))
class PROCEDURALFPHANDANIM_API UProceduralWeaponAnimSubsystem : public UGameInstanceSubsystem, public IProceduralFPHandManagerInterface, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;

	// Interface implementation~
	virtual void SetFPHandConfig_Implementation(UDA_FPHandAnimConfig* InConfig, bool bAutoEnable = true) override;
	virtual void DisableFPHandAnim_Implementation() override;
	virtual void EnableFPHandAnim_Implementation() override;
	virtual UDA_FPHandAnimConfig* GetFPHandConfig_Implementation() override;
	virtual FFPHandCRRuntimeData GetRuntimeFPHandData_Implementation() override;
	// ~End Interface Implementation
	
	UFUNCTION(BlueprintCallable)
	bool InitAndStart(ACharacter* InCharacter, APlayerController* InPlayerController);

	UFUNCTION(BlueprintCallable)
	bool StopAndDeinit();

	UFUNCTION(BlueprintCallable)
	void SetLookInput(FVector2D InLookInput);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime")
	float LocInterpSpeed = 10.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime")
	float RotInterpSpeed = 10.f;
	
	UFUNCTION(BlueprintCallable)
	FFPHandAnimDataInput GetInputData(float DeltaTime);

	bool IsHandAnimEnabled() const;
	double GetWorkerFrequency() const;
	FFPHandCRRuntimeData GetCurrentCRRuntimeData() const;
	FFPHandCRRuntimeData GetTargetCRRuntimeData() const;
	FVector2D GetLookInput() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	bool bHandAnimEnabled;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	double WorkerFrequency = 30.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	TObjectPtr<UDA_FPHandAnimConfig> CurrentConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	FFPHandAnimDataOutput AdditiveAnimDataOutput;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	FFPHandAnimDataOutput SmoothedAdditiveAnimDataOutput;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	FFPHandCRRuntimeData CurrentCRRuntimeData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	FFPHandCRRuntimeData TargetCRRuntimeData;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Runtime")
	FVector2D LookInput;

	// Event
	UPROPERTY(BlueprintAssignable)
	FOnConfigChangedEventNew OnConfigChangedEvent;

	UPROPERTY(BlueprintAssignable)
	FOnAnimStateChangedEventNew OnFPHandAnimStateChanged;

	UPROPERTY(BlueprintAssignable)
	FFPHandAnimInterpEventNew OnFPHandAnimStartInterp;

	UPROPERTY(BlueprintAssignable)
	FFPHandAnimInterpEventNew OnFPHandAnimStopInterp;

protected:
	void OnConfigChanged(UDA_FPHandAnimConfig* InConfig, bool bIsAutoEnable);
	void InterpFPHandAnimData();

private:
	void MainInterpFunc(float InDeltaTime);
	void CalculateLagParam(float DeltaTime);
	void CalculateAdditiveParam(float DeltaTime);
	float GetCurveLength(UCurveBase* InCurve);
	void StopWorker();

	UPROPERTY()
	TObjectPtr<UCurveFloat> InterpCurve;
	
	float ElapsedTime = -1.f;
	float InterpDuration = -1.f;
	bool bEnableInterp;

	FFPHandCRRuntimeData InterpStartCRRuntimeData;

	// Additive Param
	UPROPERTY()
	TObjectPtr<UCurveVector> HandSwayLocCurve;
	
	UPROPERTY()
	TObjectPtr<UCurveVector> HandSwayRotCurve;

	float AddAnimSpeedScale = 1.f;
	float LocOffsetAmplitudeScale = 1.f;
	float RotOffsetAmplitudeScale = 1.f;
	float SwayPhase = 0.f;
	float LocCurveLength = 0.f;

	//MultiThread
	TUniquePtr<FFPHandAnimWorker> Worker;
	FRunnableThread* Thread = nullptr;


private:
	bool bIsHandAnimEnabled;
	
	UPROPERTY()
	TWeakObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY()
	TWeakObjectPtr<APlayerController> OwnerPlayerController;

	float UpdateDuration = 0.f;

	FFPHandAnimDataOutput SmoothHandAnimOutput(
	const FFPHandAnimDataOutput& CurrentSmooth,
	const FFPHandAnimDataOutput& TargetRaw,
	float DeltaTime,
	float InLocInterpSpeed,
	float InRotInterpSpeed);
};
