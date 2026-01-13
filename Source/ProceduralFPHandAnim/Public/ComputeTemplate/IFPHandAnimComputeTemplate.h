#pragma once

struct FFPHandAnimDataOutput;
struct FFPHandAnimDataInput;
struct FFPHandCRRuntimeData;

struct IFPHandAnimComputeTemplate
{
	virtual ~IFPHandAnimComputeTemplate() = default;

	virtual void Compute(const FFPHandAnimDataInput& In,FFPHandAnimDataOutput& Out) const = 0;
};
