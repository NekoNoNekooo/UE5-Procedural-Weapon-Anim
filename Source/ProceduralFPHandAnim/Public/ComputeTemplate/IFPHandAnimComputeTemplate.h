#pragma once
#include "Data/Struct_FPHandCRRuntimeData.h"

struct IFPHandAnimComputeTemplate
{
	virtual ~IFPHandAnimComputeTemplate() = default;

	virtual void Compute(const FFPHandAnimDataInput& In,FFPHandAnimDataOutput& Out, FFPHandAnimDataOutput CachedLastOutput ) const = 0;
};
