#include <gtest/gtest.h>
#include <chrono>
#include <thread>

namespace
{
	class Timer
	{
	public:
		typedef std::chrono::high_resolution_clock DefaultClock;
		typedef DefaultClock::duration DefaultDuration;
		typedef std::chrono::time_point<DefaultClock> DefaultTimePoint;
		static constexpr DefaultTimePoint UninitializedTimePoint()
		{
			return DefaultTimePoint::min();
		}
		static DefaultTimePoint CurrentTime()
		{
			return DefaultClock::now();
		}

		template <class DurationT>
		explicit Timer(DurationT timeout)
			: m_timeout(std::chrono::duration_cast<DefaultDuration>(timeout))
			, m_startPoint(decltype(m_startPoint)::min())
		{}
		bool Expired() const
		{
			return TimeLeft() <= DefaultDuration(0);
		}
		void Start()
		{
			m_startPoint = CurrentTime();
		}
		DefaultDuration TimeLeft() const
		{
			return m_timeout - (CurrentTime() - m_startPoint);
		}
	private:
		DefaultDuration m_timeout;
		DefaultTimePoint m_startPoint;
	};
}

TEST(OriginalTimer, TimerIsExpiredAfterSleepForFullDuration)
{
	std::chrono::seconds duration(3);
	Timer timer(duration);
	timer.Start();
	EXPECT_FALSE(timer.Expired());
	std::this_thread::sleep_for(duration);
	EXPECT_TRUE(timer.Expired());
}

TEST(OriginalTimer, TimerIsNotExpiredAfterSleepForNotFullDuration)
{
    std::chrono::seconds duration(3);
    Timer timer(duration);
    timer.Start();
	EXPECT_FALSE(timer.Expired());
	for (int i = 0; i < 1000; ++i)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(2300));
	}
    EXPECT_FALSE(timer.Expired());
}