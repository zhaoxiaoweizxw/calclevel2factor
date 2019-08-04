#include "TimeUtil.h"
#include "utils_common.h"
//#include <sys/time.h>

string TimeUtil::nextDate(const string &sDate)
{
	time_t tomorrow = TC_Common::str2time(sDate + "000000", "%Y%m%d%H%M%S") + 60 * 60 * 24;

	return TC_Common::tm2str(tomorrow, "%Y%m%d");
}

string TimeUtil::prevDate(const string &sDate)
{
	time_t tomorrow = TC_Common::str2time(sDate + "000000", "%Y%m%d%H%M%S") - 60 * 60 * 24;

	return TC_Common::tm2str(tomorrow, "%Y%m%d");
}

int TimeUtil::nextDate(int iDate)
{
	return TC_Common::strto<int>(nextDate(TC_Common::tostr(iDate)));
}

int TimeUtil::prevDate(int iDate)
{
	return TC_Common::strto<int>(prevDate(TC_Common::tostr(iDate)));
}

string TimeUtil::nextMonth(const string &sMonth)
{
    if (sMonth.length() != 6)
    {
        return "197001";
    }
    int year = TC_Common::strto<int>(sMonth.substr(0, 4));
    int month = TC_Common::strto<int>(sMonth.substr(4 ,2));

    int monthNew = month + 1;
    int yearNew = year;
    if (monthNew == 13)
    {
        monthNew = 1;
        yearNew = year + 1;
    }

    yearNew = (yearNew >= 1970 && yearNew <= 9999) ? yearNew : 1970;
    monthNew = (monthNew >= 1 && monthNew <= 12) ? monthNew : 0;

    char szMonth[4] = { 0 };
    snprintf(szMonth, sizeof(szMonth) - 1, "%02d", monthNew);
    return TC_Common::tostr(yearNew) + szMonth;
}

string TimeUtil::prevMonth(const string &sMonth)
{
    if (sMonth.length() != 6)
    {
        return "197001";
    }
    int year = TC_Common::strto<int>(sMonth.substr(0, 4));
    int month = TC_Common::strto<int>(sMonth.substr(4, 2));

    int monthNew = month - 1;
    int yearNew = year;
    if (monthNew == 0)
    {
        monthNew = 12;
        yearNew = year - 1;
    }
    yearNew = (yearNew >= 1970 && yearNew <= 9999) ? yearNew : 1970;
    monthNew = (monthNew >= 1 && monthNew <= 12) ? monthNew : 0;

    char szMonth[4] = { 0 };
    snprintf(szMonth, sizeof(szMonth) - 1, "%02d", monthNew);
    return TC_Common::tostr(yearNew) + szMonth;
}

string TimeUtil::nextYear(const string &sYear)
{
    if (sYear.length() != 4)
    {
        return "1970";
    }
    int year = TC_Common::strto<int>(sYear.substr(0,4));
    int yearNew = year + 1;

    yearNew = (yearNew >= 1970 && yearNew <= 9999) ? yearNew : 1970;

    return TC_Common::tostr(yearNew);      
}

string TimeUtil::prevYear(const string &sYear)
{
    if (sYear.length() != 4)
    {
        return "1970";
    }
    int year = TC_Common::strto<int>(sYear.substr(0, 4));
    int yearNew = year - 1;

    yearNew = (yearNew >= 1970 && yearNew <= 9999) ? yearNew : 1970;

    return TC_Common::tostr(yearNew);
}
//
//int TimeUtil::secondsToDateInt(time_t seconds)
//{
//    struct tm tt;
//
//    TC_Common::tm2tm(seconds, tt);
//    return (1900 + tt.tm_year) * 10000 + (tt.tm_mon + 1) * 100 + tt.tm_mday;
//}
//
//string TimeUtil::secondsToDateString(time_t seconds)
//{
//	return TC_Common::tostr(secondsToDateInt(seconds));
//}

string TimeUtil::secondsToMondayString(time_t seconds)
{
    //换到周一
    int week = TC_Common::strto<int>(TC_Common::tm2str(seconds, "%w"));
    seconds -= (week - 1) * 86400;

    return TC_Common::tm2str(seconds, "%Y%m%d");
}

string TimeUtil::secondsToMonthString(time_t seconds)
{
    return TC_Common::tm2str(seconds, "%Y-%m");
}

string TimeUtil::msToTimeString(int64_t ms)
{

    int64_t sec  = ms/1000 ;
    int64_t msec = ms%1000 ;

    char szMS[5] = { 0 };
    snprintf(szMS, sizeof(szMS) - 1, "%03d", (int)msec);

    return TC_Common::tm2str(sec, "%Y%m%d-%H%M%S-") + string(szMS);
}


int64_t TimeUtil::timeStringToMs(const string & timeStr)
{
    vector<string> vt = TC_Common::sepstr<string>(timeStr, "-");
    if (vt.size() != 3)
    {
        return 0;
    }

    string & date = vt[0];
    string & time = vt[1];
    string & ms = vt[2];

    if (date.size()!=8 || time.size() != 6 || ms.size() != 3)
    {
        return 0;
    }

    int64_t lsec = TC_Common::str2time(date+time);
    int64_t lms = TC_Common::strto<int64_t>(ms);

    return lsec * 1000 + lms;
}

 bool TimeUtil::getSectionFromMs(int64_t ms, string &date, string& time, string &mstick)
{
     string timeStr = msToTimeString(ms);
     vector<string> vt = TC_Common::sepstr<string>(timeStr, "-");
     if (vt.size() != 3)
     {
         return false;
     }

     date = vt[0];
     time = vt[1];
     mstick = vt[2];

     return true;
}

 string TimeUtil::getDateFromMs(int64_t ms)
 {
     string date;
     string time;
     string mstick;
     if (!getSectionFromMs(ms, date, time, mstick))
     {
         return "00000000";
     }

     return date;
 }

 string TimeUtil::getTimeFromMs(int64_t ms)
 {
     string date;
     string time;
     string mstick;
     if (!getSectionFromMs(ms, date, time, mstick))
     {
         return "00000000";
     }

     return time;
 }

int TimeUtil::msToNowSeconds(int64_t ms)
{
	int64_t sec = ms / 1000;

	//换成当天的秒
	int64_t now = TC_Common::str2time(TC_Common::tm2str(sec, "%Y%m%d") + "000000", "%Y%m%d%H%M%S");

	return (sec - now);
}
//int TimeUtil::nowDaySeconds()
//{
//    return TimeUtil::msToNowSeconds(TC_TimeProvider::getInstance()->getNowMs());
//}

int64_t TimeUtil::msToNowMs(int64_t ms)
{
	int64_t sec = ms / 1000;

	//换成当天的秒
	int64_t now = TC_Common::str2time(TC_Common::tm2str(sec, "%Y%m%d") + "000000", "%Y%m%d%H%M%S");

	return now * 1000;
}

//int64_t TimeUtil::us()
//{
//    struct timeval tv;
//    TC_Common::gettimeofday(tv);
//    return tv.tv_sec * (int64_t)1000000 + tv.tv_usec;
//}

int64_t TimeUtil::dateToMs(const string &sDate)
{
	int64_t now;

	if (sDate.find('-') != string::npos)
	{
		now = TC_Common::str2time(sDate + "000000", "%Y-%m-%d%H%M%S");
	}
	else
	{
		//换成当天的秒
		now = TC_Common::str2time(sDate + "000000", "%Y%m%d%H%M%S");
	}

	return now * 1000;
}

int64_t TimeUtil::timeToDaySec(int64_t hour, int64_t min)
{
    return hour * 3600 + min * 60;
}

//int64_t TimeUtil::getNextAbsClockMs(int64_t clockSec)
//{
//    int64_t now = TC_TimeProvider::getInstance()->getNowMs();
//    int64_t nowSec = TimeUtil::msToNowSeconds(now);
//    int64_t currentDateMs = TimeUtil::dateToMs(TimeUtil::getDateFromMs(now));
//
//    //如果已经超过了通知时间,则改到第二天继续通知
//    int64_t notityRealTimeMs = 0;
//    if (nowSec > clockSec + 1)
//    {
//        notityRealTimeMs = currentDateMs + clockSec * 1000 + ONE_DAY_MS;
//    }
//    else
//    {
//        notityRealTimeMs = currentDateMs + clockSec * 1000;
//    }
//
//    return notityRealTimeMs;
//}


