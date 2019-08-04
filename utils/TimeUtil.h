/*
 * TimeUtil.h
 *
 *  Created on: 2017��3��6��
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
	//��һ������, sDate��:%Y%m%d
	static string nextDate(const string &sDate);
	//��һ������, sDate��:%Y%m%d
	static string prevDate(const string &sDate);
	//��һ������, iDate��:%Y%m%d
	static int nextDate(int iDate);
	//��һ������, iDate��:%Y%m%d
	static int prevDate(int iDate);

    //��һ���·� sMonth��:%Y%m
    static string nextMonth(const string &sMonth);
    //��һ���·� sMonth��:%Y%m
    static string prevMonth(const string &sMonth);
    //��һ����� sYear��:%Y
    static string nextYear(const string &sYear);
    //��һ����� sYear��:%Y
    static string prevYear(const string &sYear);

	//�뵽����%Y%m%d
	static int secondsToDateInt(time_t seconds);
	//�뵽����%Y%m%d
	static string secondsToDateString(time_t seconds);

	//�뵽��һ%Y%m%d
	static string secondsToMondayString(time_t seconds);
	//�뵽�� %Y-%m
	static string secondsToMonthString(time_t seconds);

	//����(%Y%m%d)������
	static int64_t dateToMs(const string &sDate);

    //MS���ַ��� %Y%m%d-%H%M%S-xxx
    static string msToTimeString(int64_t ms);
    //�ַ�����MS %Y%m%d-%H%M%S-xxx��ת��ʧ�ܷ���0
    static int64_t timeStringToMs(const string & timeStr);

    //MS ��ȡ������(%Y%m%d)��ʱ���ַ���(%H%M%S)��MS�ַ���(xxx)
    static bool getSectionFromMs(int64_t ms,string &date,string& time,string &mstick);
    //MS ��ȡ������(%Y%m%d)��ʧ�ܷ���"00000000"
    static string getDateFromMs(int64_t ms);
    //MS ��ȡ��ʱ��(%H%M%S)��ʧ�ܷ���"000000"
    static string getTimeFromMs(int64_t ms);

	//���뻻�ɵ��������
	static int msToNowSeconds(int64_t ms);
    static int nowDaySeconds();
	//���ɵ��쿪ʼ�ĺ���
	static int64_t msToNowMs(int64_t ms);

    static int64_t us();

    //��ǰ��Сʱ������������ 0 ~ (86400-1),hour 0~23 ;min 0~59
    static int64_t timeToDaySec(int64_t hour, int64_t min);

    //��ȡ��һ��֪ͨ�ľ���ʱ�䣬clockSecΪ���컻������
    static int64_t getNextAbsClockMs( int64_t clockSec);
};



#endif /* LIB_UTILS_TIMEUTIL_H_ */
