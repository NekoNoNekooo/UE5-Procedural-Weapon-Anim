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
        WorkEvent->Wait(5);
        if (bStop) break;

        FFPHandAnimDataInput Input;
        {
            FScopeLock Lock(&InputCS);
            if (!bHasInput)
            {
                continue;
            }
            Input = *LatestInputPtr;   // mailbox: 永远拿最新一份
            bHasInput = false;
        }

        // ---- 1) 累积“真实时间” ----
        const double ClampedDt = FMath::Clamp((double)Input.DeltaTime, 0.0, 0.1); // 防止大dt炸
        AccumulatedSec += ClampedDt;

        // ---- 2) fixed-step 积分 ----
        const int32 MaxStepsPerWake = 16; // 8有时不够，建议稍微加大
        int32 Steps = 0;

        while (AccumulatedSec >= FixedStepSec && Steps < MaxStepsPerWake)
        {
            const float StepDt = (float)FixedStepSec;

            // 2.1 复制输入，覆盖 step dt（让 Template/spring 用固定步长）
            FFPHandAnimDataInput StepInput = Input;
            StepInput.DeltaTime = StepDt;

            // 2.2 在 worker 内部计算 rot speed（deg/s）并平滑
            // 要求：StepInput.ControlRot 由外部填
            if (!bHasPrevControlRot)
            {
                PrevControlRot = StepInput.ControlRotation;
                bHasPrevControlRot = true;
            }

            // delta rot（deg/step），normalize 防止 180 跳变
            FRotator DeltaRot = (StepInput.ControlRotation - PrevControlRot).GetNormalized();
            PrevControlRot = StepInput.ControlRotation;

            float YawSpeed   = DeltaRot.Yaw   / StepDt;   // deg/s
            float PitchSpeed = DeltaRot.Pitch / StepDt;   // deg/s

            // 限幅 + 死区（推荐）
            auto ApplyDeadZone = [](float v, float dz)
            {
                return (FMath::Abs(v) < dz) ? 0.f : v;
            };

            YawSpeed   = ApplyDeadZone(FMath::Clamp(YawSpeed,   -720.f, 720.f), 2.f);
            PitchSpeed = ApplyDeadZone(FMath::Clamp(PitchSpeed, -720.f, 720.f), 2.f);

            // 速度自适应平滑（战地味更好）
            const float SpeedMag = FMath::Sqrt(YawSpeed * YawSpeed + PitchSpeed * PitchSpeed);
            const float TauMove = 0.03f;  // 转动时更跟手
            const float TauStop = 0.08f;  // 停手更稳
            const float Tau = (SpeedMag > 30.f) ? TauMove : TauStop;

            const float Alpha = 1.f - FMath::Exp(-StepDt / Tau);

            SmoothedRotSpeed.X = FMath::Lerp(SmoothedRotSpeed.X, YawSpeed, Alpha);
            SmoothedRotSpeed.Y = FMath::Lerp(SmoothedRotSpeed.Y, PitchSpeed, Alpha);

            // 把平滑后的 rot speed 写回 StepInput，让 Template 直接用
            StepInput.LookRotSpeed = SmoothedRotSpeed;

            // 2.3 执行计算（Template 内部用 StepInput.LookRotSpeed + StepInput.DeltaTime）
            FFPHandAnimDataOutput Output;
            Compute(StepInput, Output);

            AccumulatedSec -= FixedStepSec;
            Steps++;
        }

        // 如果积压太多（比如卡顿），丢掉多余时间，避免慢慢追赶导致“拖拽感”
        if (AccumulatedSec > FixedStepSec * 4.0)
        {
            AccumulatedSec = FixedStepSec * 2.0; // 留一点点，不要全清
        }

        // ---- 3) 发布频率控制：例如 30Hz 发布一次最新输出 ----
        const double Now = FPlatformTime::Seconds();
        if (Now - LastPublishTimeSec >= TargetIntervalSec)
        {
            LastPublishTimeSec = Now;

            FScopeLock Lock(&OutputCS);
            *LatestOutputPtr = CachedLastOutput;
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

	LocalTemplate->Compute(In, Out, CachedLastOutput);

	CachedLastOutput = Out;
}
