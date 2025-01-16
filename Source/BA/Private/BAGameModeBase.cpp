#include "BAGameModeBase.h"
#include "Engine/Engine.h"
#include "HAL/PlatformMemory.h"
#include <windows.h>
//#include "RHI.h"
#include <psapi.h>


float ABAGameModeBase::GetRamUsage() const
{
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc)))
    {
        SIZE_T physMemUsedByMe = pmc.WorkingSetSize;
        MEMORYSTATUSEX statex;
        statex.dwLength = sizeof(statex);

        if (GlobalMemoryStatusEx(&statex))
        {
            SIZE_T totalPhysicalMem = statex.ullTotalPhys;
            return static_cast<float>(physMemUsedByMe) / static_cast<float>(totalPhysicalMem) * 100.0f;
        }
    }
    return 0.0f;
}

float ABAGameModeBase::GetCpuUsage() const
{
    /*
    static ULONGLONG lastTime = 0;
    static ULONGLONG lastSystemTime = 0;

    FILETIME now, creationTime, exitTime, kernelTime, userTime;
    GetSystemTimeAsFileTime(&now);

    if (!GetProcessTimes(GetCurrentProcess(), &creationTime, &exitTime, &kernelTime, &userTime))
        return 0.0f;

    ULONGLONG systemTime = (reinterpret_cast<ULARGE_INTEGER*>(&kernelTime)->QuadPart +
        reinterpret_cast<ULARGE_INTEGER*>(&userTime)->QuadPart);

    if (lastTime == 0)
    {
        lastTime = reinterpret_cast<ULARGE_INTEGER*>(&now)->QuadPart;
        lastSystemTime = systemTime;
        return 0.0f;
    }

    ULONGLONG nowTime = reinterpret_cast<ULARGE_INTEGER*>(&now)->QuadPart;
    float cpuUsage = (static_cast<float>(systemTime - lastSystemTime) / (nowTime - lastTime));// *100.0f;

    lastTime = nowTime;
    lastSystemTime = systemTime;

    return cpuUsage;
    
    // one or the other but not both ^ v

    static ULONGLONG lastTime = 0;
    static ULONGLONG lastSystemTime = 0;
    static ULONGLONG totalTime = 0;
    static ULONGLONG totalSystemTime = 0;
    static int frameCount = 0;

    FILETIME now, creationTime, exitTime, kernelTime, userTime;
    GetSystemTimeAsFileTime(&now);

    if (!GetProcessTimes(GetCurrentProcess(), &creationTime, &exitTime, &kernelTime, &userTime))
        return 0.0f;

    ULONGLONG systemTime = (reinterpret_cast<ULARGE_INTEGER*>(&kernelTime)->QuadPart +
        reinterpret_cast<ULARGE_INTEGER*>(&userTime)->QuadPart);

    if (lastTime == 0)
    {
        lastTime = reinterpret_cast<ULARGE_INTEGER*>(&now)->QuadPart;
        lastSystemTime = systemTime;
        return 0.0f;
    }

    ULONGLONG nowTime = reinterpret_cast<ULARGE_INTEGER*>(&now)->QuadPart;

    // Akkumulieren der Zeiten
    totalTime += nowTime - lastTime;
    totalSystemTime += systemTime - lastSystemTime;
    frameCount++;

    // Berechne den durchschnittlichen CPU-Verbrauch über mehrere Frames
    if (frameCount >= 60) // Beispiel: Alle 60 Frames
    {
        float cpuUsage = (static_cast<float>(totalSystemTime) / totalTime) * 100.0f; // Multiplizieren mit 100 für Prozent
        // Zurücksetzen nach der Berechnung
        totalTime = 0;
        totalSystemTime = 0;
        frameCount = 0;
        return cpuUsage;
    }

    // Rückgabe eines vorläufigen Werts ohne Berechnung
    lastTime = nowTime;
    lastSystemTime = systemTime;
    return 0.0f;
    */
    return 0.0f;
}

float ABAGameModeBase::GetGpuUsage() const
{
    // GPU-Auslastung ist plattformspezifisch. Hier ein Platzhalter:
    // Erfordert APIs wie NVML für NVIDIA oder entsprechende Bibliotheken für andere GPUs.
    /*
     * nvml.h
     *
    nvmlInit();
    nvmlDevice_t device;
    nvmlDeviceGetHandleByIndex(0, &device);

    unsigned int gpuUsage = 0;
    nvmlUtilization_t utilization;
    nvmlDeviceGetUtilizationRates(device, &utilization);
    gpuUsage = utilization.gpu;

    nvmlShutdown();

    return static_cast<float>(gpuUsage);
    */

    float gputime = -1.0f;

    UWorld* w = GetWorld();

    if (w)
    {
        gputime = *(w->GetGameViewport()->GetStatUnitData()->GPUFrameTime);
    }

    gputime = *(GetWorld()->GetGameViewport()->GetStatUnitData()->GPUFrameTime);

    return gputime;
}

void ABAGameModeBase::BeginPlay()
{
    //InitialMemoryStats = FPlatformMemory::GetStats();

    if (GEngine)
    {
        //InitialStatUnitData = GEngine->GetStatUn
    }
}
