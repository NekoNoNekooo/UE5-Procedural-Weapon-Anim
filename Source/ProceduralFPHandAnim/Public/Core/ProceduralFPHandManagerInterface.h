// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/Struct_FPHandCRRuntimeData.h"
#include "UObject/Interface.h"
#include "ProceduralFPHandManagerInterface.generated.h"

class UDA_FPHandAnimConfig;
// This class does not need to be modified.
UINTERFACE()
class UProceduralFPHandManagerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROCEDURALFPHANDANIM_API IProceduralFPHandManagerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetFPHandConfig(UDA_FPHandAnimConfig* InConfig, bool bAutoEnable = true);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void DisableFPHandAnim();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void EnableFPHandAnim();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UDA_FPHandAnimConfig* GetFPHandConfig();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FFPHandCRRuntimeData GetRuntimeFPHandData();
};
