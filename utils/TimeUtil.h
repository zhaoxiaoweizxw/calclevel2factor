/*
 * TimeUtil.h
 *
 *  Created on: 2017年3月6日
 *      Author: jarod
 */

#ifndef LIB_UTILS_TIMEUTIL_H_
#define LIB_UTILS_TIMEUTIL_H_

#include <string>
#include <chrono>
#include <thread>

using namespace std;

static const int64_t ONE_DAY_MS = 24 * 3600 * 1000L;
static const int64_t ONE_HOUR_MS = 1 * 3600 * 1000L;
static const int64_t ONE_MIN_MS = 60 * 1000L;
static const int64_t ONE_DAY_SEC = 86400;

class TimeUtil
{
public:
	//下一天日期, sDate是:%Y%m%d
	static string nextDate(const string &sDate);
	//上一天日期, sDate是:%Y%m%d
	static string prevDate(const string &sDate);
	//下一天日期, iDate是:%Y%m%d
	static int nextDate(int iDate);
	//上一天日期, iDate是:%Y%m%d
	static int prevDate(int iDate);

    //下一个月份 sMonth是:%Y%m
    static string nextMonth(const string &sMonth);
    //上一个月份 sMonth是:%Y%m
    static string prevMonth(const string &sMonth);
    //下一个年份 sYear是:%Y
    static string nextYear(const string &sYear);
    //上一个年份 sYear是:%Y
    static string prevYear(const string &sYear);

	//秒到日期%Y%m%d
	static int secondsToDateInt(time_t seconds);
	//秒到日期%Y%m%d
	static string secondsToDateString(time_t seconds);

	//秒到周一%Y%m%d
	static string secondsToMondayString(time_t seconds);
	//秒到月 %Y-%m
	static string secondsToMonthString(time_t seconds);

	//日期(%Y%m%d)到毫秒
	static int64_t dateToMs(const string &sDate);

    //MS到字符串 %Y%m%d-%H%M%S-xxx
    static string msToTimeString(int64_t ms);
    //字符串到MS %Y%m%d-%H%M%S-xxx，转换失败返回0
    static int64_t timeStringToMs(const string & timeStr);

    //MS 中取出日期(%Y%m%d)、时间字符串(%H%M%S)、MS字符串(xxx)
    static bool getSectionFromMs(int64_t ms,string &date,string& time,string &mstick);
    //MS 中取出日期(%Y%m%d)，失败返回"00000000"
    static string getDateFromMs(int64_t ms);
    //MS 中取出时间(%H%M%S)，失败返回"000000"
    static string getTimeFromMs(int64_t ms);

	//毫秒换成当天的秒钟
	static int msToNowSeconds(int64_t ms);
    static int nowDaySeconds();
	//换成当天开始的毫秒
	static int64_t msToNowMs(int64_t ms);

    static int64_t us();

    //当前的小时换成日期秒数 0 ~ (86400-1),hour 0~23 ;min 0~59
    static int64_t timeToDaySec(int64_t hour, int64_t min);

    //获取下一个通知的绝对时间，clockSec为当天换成秒钟
    static int64_t getNextAbsClockMs( int64_t clockSec);
};



#endif /* LIB_UTILS_TIMEUTIL_H_ */
