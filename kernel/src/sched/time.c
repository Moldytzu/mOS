#include <sched/time.h>
#include <sched/hpet.h>
#include <misc/logger.h>

uint8_t timeCurrentSource = TIME_SOURCE_NONE;

void timeSleepMilis(uint64_t milis)
{
    switch (timeCurrentSource)
    {
    case TIME_SOURCE_HPET:
        hpetSleepMillis(milis);
        break;
    
    default:
        break;
    }
}

uint64_t timeNanos()
{
    switch (timeCurrentSource)
    {
    case TIME_SOURCE_HPET:
        return hpetNanos();
        break;

    default:
        break;
    }

    return 0;
}

const char *timers[] = {"none", "hpet"};

// probe for best available timer source
void timeSource()
{
    if (hpetAvailable())
        timeCurrentSource = TIME_SOURCE_HPET;

    logInfo("time: main time source is %s", timers[timeCurrentSource]);
}