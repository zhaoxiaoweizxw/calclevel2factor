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
		windowsʵ��strptime
		**/
		static  int conv_num(const char **, int *, int, int);
		static char * strptime(const char *buf, const char *fmt, struct tm *tm);

		static int64_t now2ms();

		/**
		* @brief  ʱ��ת��tm.
		*
		* @param t        ʱ��ṹ
		*/
		static void tm2time(const time_t &t, struct tm &tt);

		static string tm2str(const struct tm &stTm, const string &sFormat);

		/**
		* @brief  ʱ��ת�����ַ���.
		*
		* @param t        ʱ��ṹ
		* @param sFormat  ��Ҫת����Ŀ���ʽ��Ĭ��Ϊ���ո�ʽ
		* @return string  ת�����ʱ���ַ���
		*/
		static string tm2str(const time_t &t, const string &sFormat = "%Y%m%d%H%M%S");
		/**
		* @brief  ��ǰʱ��ת���ɽ��ո�ʽ�ַ���
		* @param sFormat ��ʽ��Ĭ��Ϊ���ո�ʽ
		* @return string ת�����ʱ���ַ���
		*/
		static string now2str(const string &sFormat = "%Y%m%d%H%M%S");

		/**
		* @brief  �ַ���ת����ʱ��ṹ.
		*
		* @param sString  �ַ���ʱ���ʽ
		* @param sFormat  ��ʽ
		* @param stTm     ʱ��ṹ
		* @return         0: �ɹ�, -1:ʧ��
		*/
		static int str2tm(const string &sString, const string &sFormat, struct tm &stTm);

		/**
		* @brief  ��ʽ�����ַ���ʱ��תΪʱ���.
		*
		* @param sString  ��ʽ�����ַ���ʱ��
		* @param sFormat  ��ʽ�����ַ���ʱ��ĸ�ʽ��Ĭ��Ϊ���ո�ʽ
		* @return time_t  ת�����ʱ���
		*/
		static time_t str2time(const string &sString, const string &sFormat = "%Y%m%d%H%M%S");

		/**
		* @brief  �滻�ַ���.
		*
		* @param sString  �����ַ���
		* @param sSrc     ԭ�ַ���
		* @param sDest    Ŀ���ַ���
		* @return string  �滻����ַ���
		*/
		static string replace(const string &sString, const string &sSrc, const string &sDest);
		/**
		* @brief T��ת�����ַ�����ֻҪT�ܹ�ʹ��ostream������<<����,�����Ա��ú���֧��
		* @param t Ҫת��������
		* @return  ת������ַ���
		*/
		template<typename T>
		static string tostr(const T &t);

		template<typename T>
		static T strto(const string &sStr);

		/**
		* @brief  �ַ���ת����T��.
		*
		* @param sStr      Ҫת�����ַ���
		* @param sDefault  ȱʡֵ
		* @return T        ת�����T����
		*/
		template<typename T>
		static T strto(const string &sStr, const string &sDefault);

		template<typename T>
		static vector<T> sepstr(const string &sStr, const string &sSep, bool withEmpty = false);
		/**
		* @brief  container ת�����ַ���.
		*
		* @param iFirst  ������
		* @param iLast   ������
		* @param sSep    ����Ԫ��֮��ķָ���
		* @return        ת������ַ���
		*/
		template<typename InputIter>
		static string tostr(InputIter iFirst, InputIter iLast, const string &sSep = "|");


};
/**
* @brief �����ļ���������.
*
*/
class TC_File
{
public:
	/**
	* @brief ��ȡ�ļ���string
	* �ļ������򷵻��ļ����ݣ������ڻ��߶�ȡ�ļ������ʱ��, ����Ϊ��
	* @param sFullFileName �ļ�����
	* @return              �ļ�����
	*/
	static string load2str(const string &sFullFileName);

	/**
	* @brief �жϸ���·�����ļ��Ƿ����.
	* ����ļ��Ƿ�������,���Է��������ж϶������Է�������ָ����ļ��ж�
	* @param sFullFileName �ļ�ȫ·��
	* @param iFileType     �ļ�����, ȱʡS_IFREG
	* @return           true������ڣ�fals��������
	*/
	static bool isFileExist(const string &sFullFileName);

	/**
	* @brief ����Ŀ¼, ��ȡĿ¼����������ļ�����Ŀ¼.
	*
	* @param path       ��Ҫ������·��
	* @param files      Ŀ��·�����������ļ�
	* @param bRecursive �Ƿ�ݹ���Ŀ¼
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