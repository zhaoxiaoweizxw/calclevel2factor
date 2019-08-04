#pragma once
#include  <io.h>
#include <stdint.h>
#include <string>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <ctime>
#include <iostream>
#include <iterator>
#include <fstream>
#include <vector>
#include <sstream> 

using namespace std;
class TC_Common
{
	public:

		/**
		windows实现strptime
		**/
		static  int conv_num(const char **, int *, int, int);
		static char * strptime(const char *buf, const char *fmt, struct tm *tm);

		static int64_t now2ms();

		/**
		* @brief  时间转换tm.
		*
		* @param t        时间结构
		*/
		static void tm2time(const time_t &t, struct tm &tt);

		static string tm2str(const struct tm &stTm, const string &sFormat);

		/**
		* @brief  时间转换成字符串.
		*
		* @param t        时间结构
		* @param sFormat  需要转换的目标格式，默认为紧凑格式
		* @return string  转换后的时间字符串
		*/
		static string tm2str(const time_t &t, const string &sFormat = "%Y%m%d%H%M%S");
		/**
		* @brief  当前时间转换成紧凑格式字符串
		* @param sFormat 格式，默认为紧凑格式
		* @return string 转换后的时间字符串
		*/
		static string now2str(const string &sFormat = "%Y%m%d%H%M%S");

		/**
		* @brief  字符串转换成时间结构.
		*
		* @param sString  字符串时间格式
		* @param sFormat  格式
		* @param stTm     时间结构
		* @return         0: 成功, -1:失败
		*/
		static int str2tm(const string &sString, const string &sFormat, struct tm &stTm);

		/**
		* @brief  格式化的字符串时间转为时间戳.
		*
		* @param sString  格式化的字符串时间
		* @param sFormat  格式化的字符串时间的格式，默认为紧凑格式
		* @return time_t  转换后的时间戳
		*/
		static time_t str2time(const string &sString, const string &sFormat = "%Y%m%d%H%M%S");

		/**
		* @brief  替换字符串.
		*
		* @param sString  输入字符串
		* @param sSrc     原字符串
		* @param sDest    目的字符串
		* @return string  替换后的字符串
		*/
		static string replace(const string &sString, const string &sSrc, const string &sDest);
		/**
		* @brief T型转换成字符串，只要T能够使用ostream对象用<<重载,即可以被该函数支持
		* @param t 要转换的数据
		* @return  转换后的字符串
		*/
		template<typename T>
		static string tostr(const T &t);

		template<typename T>
		static T strto(const string &sStr);

		/**
		* @brief  字符串转化成T型.
		*
		* @param sStr      要转换的字符串
		* @param sDefault  缺省值
		* @return T        转换后的T类型
		*/
		template<typename T>
		static T strto(const string &sStr, const string &sDefault);

		template<typename T>
		static vector<T> sepstr(const string &sStr, const string &sSep, bool withEmpty = false);
		/**
		* @brief  container 转换成字符串.
		*
		* @param iFirst  迭代器
		* @param iLast   迭代器
		* @param sSep    两个元素之间的分隔符
		* @return        转换后的字符串
		*/
		template<typename InputIter>
		static string tostr(InputIter iFirst, InputIter iLast, const string &sSep = "|");


};
/**
* @brief 常用文件名处理函数.
*
*/
class TC_File
{
public:
	/**
	* @brief 读取文件到string
	* 文件存在则返回文件数据，不存在或者读取文件错误的时候, 返回为空
	* @param sFullFileName 文件名称
	* @return              文件数据
	*/
	static string load2str(const string &sFullFileName);

	/**
	* @brief 判断给定路径的文件是否存在.
	* 如果文件是符号连接,则以符号连接判断而不是以符号连接指向的文件判断
	* @param sFullFileName 文件全路径
	* @param iFileType     文件类型, 缺省S_IFREG
	* @return           true代表存在，fals代表不存在
	*/
	static bool isFileExist(const string &sFullFileName);

	/**
	* @brief 遍历目录, 获取目录下面的所有文件和子目录.
	*
	* @param path       需要遍历的路径
	* @param files      目标路径下面所有文件
	* @param bRecursive 是否递归子目录
	*
	**/
	static void listDirectory(const string &path, vector<string> &files, bool bRecursive);

};
namespace p
{
	template<typename D>
	struct strto1
	{
		D operator()(const string &sStr)
		{
			string s = "0";

			if (!sStr.empty()) {
				s = sStr;
			}

			istringstream sBuffer(s);

			D t;
			sBuffer >> t;

			return t;
		}
	};

	template<>
	struct strto1<char>
	{
		char operator()(const string &sStr)
		{
			if (!sStr.empty()) {
				return sStr[0];
			}
			return 0;
		}
	};

	template<>
	struct strto1<short>
	{
		short operator()(const string &sStr)
		{
			if (!sStr.empty()) {
				if (sStr.find("0x") == 0) {
					return (short) ::strtol(sStr.c_str(), NULL, 16);
				}
				else {
					return atoi(sStr.c_str());
				}
			}
			return 0;
		}
	};

	template<>
	struct strto1<unsigned short>
	{
		unsigned short operator()(const string &sStr)
		{
			if (!sStr.empty()) {
				if (sStr.find("0x") == 0) {
					return (unsigned short) ::strtoul(sStr.c_str(), NULL, 16);
				}
				else {
					return (unsigned short)strtoul(sStr.c_str(), NULL, 10);
				}
			}
			return 0;
		}
	};

	template<>
	struct strto1<int>
	{
		int operator()(const string &sStr)
		{
			if (!sStr.empty()) {
				if (sStr.find("0x") == 0) {
					return ::strtol(sStr.c_str(), NULL, 16);
				}
				else {
					return atoi(sStr.c_str());
				}
			}
			return 0;
		}
	};

	template<>
	struct strto1<unsigned int>
	{
		unsigned int operator()(const string &sStr)
		{
			if (!sStr.empty()) {
				if (sStr.find("0x") == 0) {
					return ::strtoul(sStr.c_str(), NULL, 16);
				}
				else {
					return strtoul(sStr.c_str(), NULL, 10);
				}
			}
			return 0;
		}
	};

	template<>
	struct strto1<long>
	{
		long operator()(const string &sStr)
		{
			if (!sStr.empty()) {
				if (sStr.find("0x") == 0) {
					return ::strtol(sStr.c_str(), NULL, 16);
				}
				else {
					return atol(sStr.c_str());
				}
			}
			return 0;
		}
	};

	template<>
	struct strto1<long long>
	{
		long long operator()(const string &sStr)
		{
			if (!sStr.empty()) {
				if (sStr.find("0x") == 0) {
					return ::strtoll(sStr.c_str(), NULL, 16);
				}
				else {
					return atoll(sStr.c_str());
				}
			}
			return 0;
		}
	};

	template<>
	struct strto1<unsigned long>
	{
		unsigned long operator()(const string &sStr)
		{
			if (!sStr.empty()) {
				if (sStr.find("0x") == 0) {
					return ::strtoul(sStr.c_str(), NULL, 16);
				}
				else {
					return strtoul(sStr.c_str(), NULL, 10);
				}
			}
			return 0;
		}
	};

	template<>
	struct strto1<float>
	{
		float operator()(const string &sStr)
		{
			if (!sStr.empty()) {
				return (float)atof(sStr.c_str());
			}
			return 0;
		}
	};

	template<>
	struct strto1<double>
	{
		double operator()(const string &sStr)
		{
			if (!sStr.empty()) {
				return atof(sStr.c_str());
			}
			return 0;
		}
	};

	template<typename D>
	struct strto2
	{
		D operator()(const string &sStr)
		{
			istringstream sBuffer(sStr);

			D t;
			sBuffer >> t;

			return t;
		}
	};

	template<>
	struct strto2<string>
	{
		string operator()(const string &sStr)
		{
			return sStr;
		}
	};

}
template<typename T>
string TC_Common::tostr(const T &t)
{
	ostringstream sBuffer;
	sBuffer << t;
	return sBuffer.str();
}


template<typename T>
T TC_Common::strto(const string &sStr)
{
	//    typedef typename TL::TypeSelect<TL::TypeTraits<T>::isStdArith, p::strto1<T>, p::strto2<T> >::Result strto_type;

	typedef typename std::conditional<std::is_arithmetic<T>::value, p::strto1<T>, p::strto2<T> >::type strto_type;
	return strto_type()(sStr);
}

template<typename T>
T TC_Common::strto(const string &sStr, const string &sDefault)
{
	string s;

	if (!sStr.empty()) {
		s = sStr;
	}
	else {
		s = sDefault;
	}

	return strto<T>(s);
}

template<typename T>
vector<T> TC_Common::sepstr(const string &sStr, const string &sSep, bool withEmpty)
{
	vector<T> vt;

	string::size_type pos = 0;
	string::size_type pos1 = 0;

	while (true) {
		string s;
		pos1 = sStr.find_first_of(sSep, pos);
		if (pos1 == string::npos) {
			if (pos + 1 <= sStr.length()) {
				s = sStr.substr(pos);
			}
		}
		else if (pos1 == pos) {
			s = "";
		}
		else {
			s = sStr.substr(pos, pos1 - pos);
			pos = pos1;
		}

		if (withEmpty) {
			vt.push_back(std::move(strto<T>(s)));
		}
		else {
			if (!s.empty()) {
				vt.push_back(std::move(strto<T>(s)));
			}
		}

		if (pos1 == string::npos) {
			break;
		}

		pos++;
	}

	return vt;
}
template<typename InputIter>
string TC_Common::tostr(InputIter iFirst, InputIter iLast, const string &sSep)
{
	string sBuffer;
	InputIter it = iFirst;

	while (it != iLast) {
		sBuffer += tostr(*it);
		++it;

		if (it != iLast) {
			sBuffer += sSep;
		}
		else {
			break;
		}
	}

	return sBuffer;
}