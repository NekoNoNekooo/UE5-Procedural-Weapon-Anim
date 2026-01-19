#pragma once

#include "CoreMinimal.h"
#include "Struct_FPHandCRStaticData.h"
#include "Data/Struct_FPHandCRRuntimeData.h"
#include "Struct_PWA_CHTParam.generated.h"

USTRUCT(BlueprintType)
struct FCHTInput_PWA
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsADS;
};

USTRUCT(BlueprintType)
struct FCHTOutput_PWA
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFPHandCRStaticData StaticHandData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BlendTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UBlendProfile> BlendProfile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayRate;
};