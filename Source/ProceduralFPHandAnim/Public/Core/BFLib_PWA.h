// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BFLib_PWA.generated.h"

struct FCHTInput_PWA;
struct FCHTOutput_PWA;
struct FChooserEvaluationContext;
/**
 * 
 */
UCLASS()
class PROCEDURALFPHANDANIM_API UBFLib_PWA : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe, DisplayName="Evaluate PWA Anim Chooser (Thread Safe)"))
	static void EvaluatePWAChooser(const UObject* ChooserAsset, UPARAM(ref) FCHTInput_PWA& Input, UAnimSequence*& AnimSequence, FCHTOutput_PWA& Output);
};
