// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/BFLib_PWA.h"

#include "ChooserFunctionLibrary.h"
#include "Animation/AnimSequence.h"
#include "InstancedStruct.h"
#include "Data/Struct_PWA_CHTParam.h"

void UBFLib_PWA::EvaluatePWAChooser(const UObject* ChooserAsset, FCHTInput_PWA& Input, UAnimSequence*& AnimSequence,
                                    FCHTOutput_PWA& Output)
{
	AnimSequence = nullptr;

	const UChooserTable* ChooserTable = Cast<UChooserTable>(ChooserAsset);
	if (!ChooserTable)
	{
		return;
	}

	// 1) 构建评估上下文（相当于 BP Evaluate Chooser 节点自动生成的上下文）
	FChooserEvaluationContext Context;
	
	Context.AddStructParam(Input);
	Context.AddStructParam(Output);
	
	// 2) 把 ChooserTable 转成 InstancedStruct（BP Evaluate Chooser 底层也会做类似事情）
	FInstancedStruct ChooserStruct = UChooserFunctionLibrary::MakeEvaluateChooser(const_cast<UChooserTable*>(ChooserTable));

	// 3) 执行选择：期望返回一个 UObject（这里我们要 AnimSequence）
	UObject* ResultObj = UChooserFunctionLibrary::EvaluateObjectChooserBase(
		Context,
		ChooserStruct,
		UAnimSequence::StaticClass(),
		/*bResultIsClass=*/false);

	AnimSequence = Cast<UAnimSequence>(ResultObj);
}
