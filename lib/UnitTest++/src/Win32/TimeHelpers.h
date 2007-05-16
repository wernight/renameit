#ifndef UNITTEST_TIMEHELPERS_H
#define UNITTEST_TIMEHELPERS_H

namespace UnitTest {

class Timer
{
public:
    Timer();
	void Start();
	int GetTimeInMs() const;    

private:
    __int64 GetTime() const;

    void* m_threadId;
    unsigned long m_processAffinityMask;
	__int64 m_startTime;
	__int64 m_frequency;
};


namespace TimeHelpers
{
void SleepMs (int ms);
}


}



#endif
