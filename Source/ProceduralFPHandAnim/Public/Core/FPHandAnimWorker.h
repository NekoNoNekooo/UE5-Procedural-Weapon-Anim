#pragma once
#include "ComputeTemplate/IFPHandAnimComputeTemplate.h"

struct FFPHandAnimDataOutput;
struct FFPHandAnimDataInput;
struct FFPHandCRRuntimeData;

class FFPHandAnimWorker : public FRunnable
{
public:
	FFPHandAnimWorker(double CalculateFrequency);
	virtual ~FFPHandAnimWorker() override;

	// FRunnable
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

	// GameThread API
	void PushInput(const FFPHandAnimDataInput& In);
	bool TryPopOutput(FFPHandAnimDataOutput& Out);

	void SetTemplate(TSharedPtr<const IFPHandAnimComputeTemplate, ESPMode::ThreadSafe> InTemplate);
private:
	void Compute(const FFPHandAnimDataInput& In, FFPHandAnimDataOutput& Out);

	TSharedPtr<const IFPHandAnimComputeTemplate, ESPMode::ThreadSafe> ActiveTemplate;

	FThreadSafeBool bStop = false;

	// 用事件来等待输入，避免无意义轮询
	FEvent* WorkEvent = nullptr;

	// latest input
	FCriticalSection InputCS;
	FFPHandAnimDataInput* LatestInputPtr = nullptr; // 用指针避免头文件必须 include 结构体定义
	bool bHasInput = false;

	// latest output
	FCriticalSection OutputCS;
	FFPHandAnimDataOutput* LatestOutputPtr = nullptr;
	bool bHasOutput = false;

	double TargetIntervalSec = 1.0 / 60.0; // 60Hz 默认
	double LastComputeTimeSec = 0.0;
};
