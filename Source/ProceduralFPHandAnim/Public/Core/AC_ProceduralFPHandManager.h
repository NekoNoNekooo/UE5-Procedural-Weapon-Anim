// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralFPHandManagerInterface.h"
#include "Components/ActorComponent.h"
#include "Data/Struct_FPHandCRRuntimeData.h"
#include "FPHandAnimWorker.h"
#include "AC_ProceduralFPHandManager.generated.h"

class UCurveVector;
class UDA_FPHandAnimConfig;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConfigChangedEvent, UDA_FPHandAnimConfig*, UpdatedConfig);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnimStateChangedEvent, bool, Enabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFPHandAnimInterpEvent);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROCEDURALFPHANDANIM_API UAC_ProceduralFPHandManager : public UActorComponent, public IProceduralFPHandManagerInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UAC_ProceduralFPHandManager();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	
	// Interface implementation~
	virtual void SetFPHandConfig_Implementation(UDA_FPHandAnimConfig* InConfig, bool bAutoEnable = true) override;

	virtual UDA_FPHandAnimConfig* GetFPHandConfig_Implementation() override;

	virtual FFPHandCRRuntimeData GetRuntimeFPHandData_Implementation() override;
	// ~End Interface Implementation
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	bool bHandAnimEnabled;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	double WorkerFrequency = 60.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	TObjectPtr<UDA_FPHandAnimConfig> CurrentConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	FFPHandCRRuntimeData CurrentCRRuntimeData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
	FFPHandCRRuntimeData TargetCRRuntimeData;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Runtime")
	FVector2D LookInput;
	
	// Event
	
	UPROPERTY(BlueprintAssignable)
	FOnConfigChangedEvent OnConfigChangedEvent;

	UPROPERTY(BlueprintAssignable)
	FOnAnimStateChangedEvent OnFPHandAnimStateChanged;

	UPROPERTY(BlueprintAssignable)
	FFPHandAnimInterpEvent OnFPHandAnimStartInterp;

	UPROPERTY(BlueprintAssignable)
	FFPHandAnimInterpEvent OnFPHandAnimStopInterp;

	//

	UFUNCTION(BlueprintCallable,BlueprintNativeEvent)
	FFPHandAnimDataInput GetInputData(float DeltaTime);
	virtual FFPHandAnimDataInput GetInputData_Implementation(float DeltaTime);
protected:
	void OnConfigChanged(UDA_FPHandAnimConfig* InConfig, bool bIsAutoEnable);

	void InterpFPHandAnimData();

private:
	UPROPERTY()
	TObjectPtr<UCurveFloat> InterpCurve;
	
	float ElapsedTime = -1.f;
	float InterpDuration = -1.f;
	bool bEnableInterp;
	void MainInterpFunc(float InDeltaTime);

	FFPHandCRRuntimeData InterpStartCRRuntimeData;

	// Additive Param
	void CalculateLagParam(float DeltaTime);
	void CalculateAdditiveParam(float DeltaTime);
	float GetCurveLength(UCurveBase* InCurve);
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
	void StopWorker();
};
