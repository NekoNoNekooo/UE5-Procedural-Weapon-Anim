#include "Core/FPHandAnimWorker.h"
#include "HAL/PlatformProcess.h"

// 这里包含你的 struct 定义头（必须是定义，不是 forward）
#include "Data/Struct_FPHandCRRuntimeData.h" // 假设这里定义了 FFPHandAnimDataInput / FFPHandCRRuntimeData

FFPHandAnimWorker::FFPHandAnimWorker(double CalculateFrequency)
{
	WorkEvent = FPlatformProcess::GetSynchEventFromPool(false);

	LatestInputPtr = new FFPHandAnimDataInput();
	LatestOutputPtr = new FFPHandAnimDataOutput();

	TargetIntervalSec = 1 / CalculateFrequency;
}

FFPHandAnimWorker::~FFPHandAnimWorker()
{
	// 注意：线程应该由外部（组件）Stop + WaitForCompletion 后再析构 Worker
	if (WorkEvent)
	{
		FPlatformProcess::ReturnSynchEventToPool(WorkEvent);
		WorkEvent = nullptr;
	}

	delete LatestInputPtr;
	LatestInputPtr = nullptr;

	delete LatestOutputPtr;
	LatestOutputPtr = nullptr;
}

bool FFPHandAnimWorker::Init()
{
	return true;
}

uint32 FFPHandAnimWorker::Run()
{
	while (!bStop)
	{
		// 等待输入
		WorkEvent->Wait(5);
		if (bStop) break;

		FFPHandAnimDataInput Input;
		{
			FScopeLock Lock(&InputCS);
			if (!bHasInput)
			{
				continue;
			}
			Input = *LatestInputPtr;
			bHasInput = false;
		}

		// ===== 频率控制 =====
		const double Now = FPlatformTime::Seconds();
		if (Now - LastComputeTimeSec < TargetIntervalSec)
		{
			// 丢弃本次（或你也可以缓存，下一次算）
			continue;
		}
		LastComputeTimeSec = Now;

		FFPHandAnimDataOutput Output;
		Compute(Input, Output);

		{
			FScopeLock Lock(&OutputCS);
			*LatestOutputPtr = Output;
			bHasOutput = true;
		}
	}

	return 0;
}

void FFPHandAnimWorker::Stop()
{
	bStop = true;
	if (WorkEvent)
	{
		WorkEvent->Trigger(); // 唤醒 Run，让它能尽快退出
	}
}

void FFPHandAnimWorker::Exit()
{
	// 可选：清理线程私有资源
}

void FFPHandAnimWorker::PushInput(const FFPHandAnimDataInput& In)
{
	{
		FScopeLock Lock(&InputCS);
		*LatestInputPtr = In;
		bHasInput = true;
	}
	if (WorkEvent)
	{
		WorkEvent->Trigger();
	}
}

bool FFPHandAnimWorker::TryPopOutput(FFPHandAnimDataOutput& Out)
{
	FScopeLock Lock(&OutputCS);
	if (!bHasOutput)
	{
		return false;
	}

	Out = *LatestOutputPtr;

	// 你可以选择：
	// A) bHasOutput=false：每个输出只消费一次
	// B) 不清：始终取 latest（Anim 更平滑）
	bHasOutput = false;

	return true;
}

void FFPHandAnimWorker::SetTemplate(TSharedPtr<const IFPHandAnimComputeTemplate, ESPMode::ThreadSafe> InTemplate)
{
	ActiveTemplate = MoveTemp(InTemplate);
}

void FFPHandAnimWorker::Compute(const FFPHandAnimDataInput& In, FFPHandAnimDataOutput& Out)
{
	const TSharedPtr<const IFPHandAnimComputeTemplate, ESPMode::ThreadSafe> LocalTemplate
		= ActiveTemplate;

	if (!LocalTemplate.IsValid())
	{
		return;
	}

	LocalTemplate->Compute(In, Out);
}
