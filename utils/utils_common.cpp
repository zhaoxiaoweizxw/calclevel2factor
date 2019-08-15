
#include "utils_common.h"

#define TM_YEAR_BASE 1900

/*
* We do not implement alternate representations. However, we always
* check whether a given modifier is allowed for a certain conversion.
*/
#define ALT_E     0x01
#define ALT_O     0x02
#define LEGAL_ALT(x)    { if (alt_format & ~(x)) return (0); }

#define timegm _mkgmtime


static const char *day[7] = {
	"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
	"Friday", "Saturday"
};
static const char *abday[7] = {
	"Sun","Mon","Tue","Wed","Thu","Fri","Sat"
};
static const char *mon[12] = {
	"January", "February", "March", "April", "May", "June", "July",
	"August", "September", "October", "November", "December"
};
static const char *abmon[12] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
static const char *am_pm[2] = {
	"AM", "PM"
};

//用于计算时区差异!
class TimezoneHelper
{
public:
	TimezoneHelper()
	{
		struct tm timeinfo;
		time_t secs, local_secs, gmt_secs;
		time(&secs);

		//带时区时间
		localtime_s(&timeinfo, &secs);

		local_secs = mktime(&timeinfo);

		//不带时区时间
		gmtime_s(&timeinfo, &secs);

		gmt_secs = mktime(&timeinfo);
		timezone_diff_secs = local_secs - gmt_secs;
	}

	static int64_t timezone_diff_secs;
};

int64_t TimezoneHelper::timezone_diff_secs = 0;

int64_t TC_Common::now2ms()
{
	//struct timeval tv;

	//gettimeofday(&tv, 0);

	return 100;// tv.tv_sec * (int64_t)1000 + tv.tv_usec / 1000;
}

string TC_Common::tm2str(const struct tm &stTm, const string &sFormat)
{
	char sTimeString[255] = "\0";

	strftime(sTimeString, sizeof(sTimeString), sFormat.c_str(), &stTm);

	return string(sTimeString);
}

string TC_Common::tm2str(const time_t &t, const string &sFormat)
{
	struct tm tt;
	tm2time(t, tt);

	return tm2str(tt, sFormat);
}

void TC_Common::tm2time(const time_t &t, struct tm &tt)
{
	//加快速度, 否则会比较慢, 不用localtime_r(会访问时区文件, 较慢)
	static TimezoneHelper helper;
	time_t localt = t + TimezoneHelper::timezone_diff_secs;

	gmtime_s(&tt, &localt);
}

int TC_Common::str2tm(const string &sString, const string &sFormat, struct tm &stTm)
{
	char *p = strptime(sString.c_str(), sFormat.c_str(), &stTm);
	return (p != NULL) ? 0 : -1;
}

time_t TC_Common::str2time(const string &sString, const string &sFormat)
{
	struct tm stTm;
	if (0 == str2tm(sString, sFormat, stTm))
	{
		//注意这里没有直接用mktime, mktime会访问时区文件, 会巨慢!
		static TimezoneHelper helper;
		return timegm(&stTm) - TimezoneHelper::timezone_diff_secs;
	}
	return 0;
}

string TC_Common::now2str(const string &sFormat)
{
	time_t t = time(NULL);
	return tm2str(t, sFormat.c_str());
}


//window上自己实现strptime函数，linux已经提供strptime
//strptime函数windows平台上实现
char * TC_Common::strptime(const char *buf, const char *fmt, struct tm *tm)
{
	char c;
	const char *bp;
	size_t len = 0;
	int alt_format, i, split_year = 0;

	bp = buf;

	while ((c = *fmt) != '\0') {
		/* Clear `alternate' modifier prior to new conversion. */
		alt_format = 0;

		/* Eat up white-space. */
		if (isspace(c)) {
			while (isspace(*bp))
				bp++;

			fmt++;
			continue;
		}

		if ((c = *fmt++) != '%')
			goto literal;


	again:    switch (c = *fmt++) {
	case '%': /* "%%" is converted to "%". */
		literal :
			if (c != *bp++)
				return (0);
		break;

		/*
		* "Alternative" modifiers. Just set the appropriate flag
		* and start over again.
		*/
	case 'E': /* "%E?" alternative conversion modifier. */
		LEGAL_ALT(0);
		alt_format |= ALT_E;
		goto again;

	case 'O': /* "%O?" alternative conversion modifier. */
		LEGAL_ALT(0);
		alt_format |= ALT_O;
		goto again;

		/*
		* "Complex" conversion rules, implemented through recursion.
		*/
	case 'c': /* Date and time, using the locale's format. */
		LEGAL_ALT(ALT_E);
		if (!(bp = strptime(bp, "%x %X", tm)))
			return (0);
		break;

	case 'D': /* The date as "%m/%d/%y". */
		LEGAL_ALT(0);
		if (!(bp = strptime(bp, "%m/%d/%y", tm)))
			return (0);
		break;

	case 'R': /* The time as "%H:%M". */
		LEGAL_ALT(0);
		if (!(bp = strptime(bp, "%H:%M", tm)))
			return (0);
		break;

	case 'r': /* The time in 12-hour clock representation. */
		LEGAL_ALT(0);
		if (!(bp = strptime(bp, "%I:%M:%S %p", tm)))
			return (0);
		break;

	case 'T': /* The time as "%H:%M:%S". */
		LEGAL_ALT(0);
		if (!(bp = strptime(bp, "%H:%M:%S", tm)))
			return (0);
		break;

	case 'X': /* The time, using the locale's format. */
		LEGAL_ALT(ALT_E);
		if (!(bp = strptime(bp, "%H:%M:%S", tm)))
			return (0);
		break;

	case 'x': /* The date, using the locale's format. */
		LEGAL_ALT(ALT_E);
		if (!(bp = strptime(bp, "%m/%d/%y", tm)))
			return (0);
		break;

		/*
		* "Elementary" conversion rules.
		*/
	case 'A': /* The day of week, using the locale's form. */
	case 'a':
		LEGAL_ALT(0);
		for (i = 0; i < 7; i++) {
			/* Full name. */
			len = strlen(day[i]);
			if (strncmp(day[i], bp, len) == 0)
				break;

			/* Abbreviated name. */
			len = strlen(abday[i]);
			if (strncmp(abday[i], bp, len) == 0)
				break;
		}

		/* Nothing matched. */
		if (i == 7)
			return (0);

		tm->tm_wday = i;
		bp += len;
		break;

	case 'B': /* The month, using the locale's form. */
	case 'b':
	case 'h':
		LEGAL_ALT(0);
		for (i = 0; i < 12; i++) {
			/* Full name. */
			len = strlen(mon[i]);
			if (strncmp(mon[i], bp, len) == 0)
				break;

			/* Abbreviated name. */
			len = strlen(abmon[i]);
			if (strncmp(abmon[i], bp, len) == 0)
				break;
		}

		/* Nothing matched. */
		if (i == 12)
			return (0);

		tm->tm_mon = i;
		bp += len;
		break;

	case 'C': /* The century number. */
		LEGAL_ALT(ALT_E);
		if (!(conv_num(&bp, &i, 0, 99)))
			return (0);

		if (split_year) {
			tm->tm_year = (tm->tm_year % 100) + (i * 100);
		}
		else {
			tm->tm_year = i * 100;
			split_year = 1;
		}
		break;

	case 'd': /* The day of month. */
	case 'e':
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_mday, 1, 31)))
			return (0);
		break;

	case 'k': /* The hour (24-hour clock representation). */
		LEGAL_ALT(0);
		/* FALLTHROUGH */
	case 'H':
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_hour, 0, 23)))
			return (0);
		break;

	case 'l': /* The hour (12-hour clock representation). */
		LEGAL_ALT(0);
		/* FALLTHROUGH */
	case 'I':
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_hour, 1, 12)))
			return (0);
		if (tm->tm_hour == 12)
			tm->tm_hour = 0;
		break;

	case 'j': /* The day of year. */
		LEGAL_ALT(0);
		if (!(conv_num(&bp, &i, 1, 366)))
			return (0);
		tm->tm_yday = i - 1;
		break;

	case 'M': /* The minute. */
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_min, 0, 59)))
			return (0);
		break;

	case 'm': /* The month. */
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &i, 1, 12)))
			return (0);
		tm->tm_mon = i - 1;
		break;

	case 'p': /* The locale's equivalent of AM/PM. */
		LEGAL_ALT(0);
		/* AM? */
		if (strcmp(am_pm[0], bp) == 0) {
			if (tm->tm_hour > 11)
				return (0);

			bp += strlen(am_pm[0]);
			break;
		}
		/* PM? */
		else if (strcmp(am_pm[1], bp) == 0) {
			if (tm->tm_hour > 11)
				return (0);

			tm->tm_hour += 12;
			bp += strlen(am_pm[1]);
			break;
		}

		/* Nothing matched. */
		return (0);

	case 'S': /* The seconds. */
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_sec, 0, 61)))
			return (0);
		break;

	case 'U': /* The week of year, beginning on sunday. */
	case 'W': /* The week of year, beginning on monday. */
		LEGAL_ALT(ALT_O);
		/*
		* XXX This is bogus, as we can not assume any valid
		* information present in the tm structure at this
		* point to calculate a real value, so just check the
		* range for now.
		*/
		if (!(conv_num(&bp, &i, 0, 53)))
			return (0);
		break;

	case 'w': /* The day of week, beginning on sunday. */
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_wday, 0, 6)))
			return (0);
		break;

	case 'Y': /* The year. */
		LEGAL_ALT(ALT_E);
		if (!(conv_num(&bp, &i, 0, 9999)))
			return (0);

		tm->tm_year = i - TM_YEAR_BASE;
		break;

	case 'y': /* The year within 100 years of the epoch. */
		LEGAL_ALT(ALT_E | ALT_O);
		if (!(conv_num(&bp, &i, 0, 99)))
			return (0);

		if (split_year) {
			tm->tm_year = ((tm->tm_year / 100) * 100) + i;
			break;
		}
		split_year = 1;
		if (i <= 68)
			tm->tm_year = i + 2000 - TM_YEAR_BASE;
		else
			tm->tm_year = i + 1900 - TM_YEAR_BASE;
		break;

		/*
		* Miscellaneous conversions.
		*/
	case 'n': /* Any kind of white-space. */
	case 't':
		LEGAL_ALT(0);
		while (isspace(*bp))
			bp++;
		break;


	default:  /* Unknown/unsupported conversion. */
		return (0);
	}


	}

	/* LINTED functional specification */
	return ((char *)bp);
}

int TC_Common::conv_num(const char **buf, int *dest, int llim, int ulim)
{
	int result = 0;

	/* The limit also determines the number of valid digits. */
	int rulim = ulim;

	if (**buf < '0' || **buf > '9')
		return (0);

	do {
		result *= 10;
		result += *(*buf)++ - '0';
		rulim /= 10;
	} while ((result * 10 <= ulim) && rulim && **buf >= '0' && **buf <= '9');

	if (result < llim || result > ulim)
		return (0);

	*dest = result;
	return (1);
}
string TC_Common::replace(const string &sString, const string &sSrc, const string &sDest)
{
	if (sSrc.empty())
	{
		return sString;
	}

	string sBuf = sString;

	string::size_type pos = 0;

	while ((pos = sBuf.find(sSrc, pos)) != string::npos)
	{
		sBuf.replace(pos, sSrc.length(), sDest);
		pos += sDest.length();
	}

	return sBuf;
}



string TC_File::load2str(const string &sFullFileName)
{
	/*std::ifstream in(sFullFileName.data());
	if (!in)
		return "";

	std::istream_iterator<char> begin(in);
	std::istream_iterator<char> end;
	std::string some_str(begin, end);
	return some_str;*/

	string str;

	ifstream fin;
	fin.open(sFullFileName.data(), ios::in);
	stringstream buf;
	buf << fin.rdbuf();
	str = buf.str();
	fin.close();
	buf.str("");

	return str;

	/*int fd = open(sFullFileName.data(), O_RDONLY);
	if (fd < 0)
	return "";

	string s = "";
	int nread = -1;
	do {
	char buf[1024] = { '\0' };
	nread = read(fd, buf, sizeof(buf));
	if (nread > 0)
	s += buf;
	} while (nread > 0);
	close(fd);
	return s;*/
}

bool TC_File::isFileExist(const string &sFullFileName)
{
	if ((_access(sFullFileName.c_str(), 0)) != -1)
	{
		return true;
	}
	return false;

}


void TC_File::listDirectory(const string &path, vector<string> &files, bool bRecursive)
{

	long   hFile = 0;
	//文件信息    
	struct _finddata_t fileinfo;//用来存储文件信息的结构体    
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)  //第一次查找  
	{
		do
		{
			if ((fileinfo.attrib &  _A_SUBDIR) & bRecursive)  //如果查找到的是文件夹  
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)  //进入文件夹查找  
				{
					files.push_back(p.assign(path).append("\\").append(fileinfo.name));
					listDirectory(p.assign(path).append("\\").append(fileinfo.name), files, bRecursive);
				}
			}
			else //如果查找到的不是是文件夹   
			{
				//files.push_back(p.assign(fileinfo.name));  //将文件路径保存，也可以只保存文件名:    p.assign(path).append("\\").append(fileinfo.name)  
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}

		} while (_findnext(hFile, &fileinfo) == 0);

		_findclose(hFile); //结束查找  
	}

}