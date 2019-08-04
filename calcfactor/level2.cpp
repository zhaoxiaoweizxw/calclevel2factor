
#include <direct.h>
#include <thread>
#include <algorithm>
#include "BigSmall.h"
#include "DailyFactors.h"
#include "earlymoney.h"
//#include "business/refdata/MarketSessionManager.h"
#include "market.h"
#include "public.h"
#include "../utils/utils_common.h"
#include "../utils/TimeUtil.h"

#include <string>
using namespace std;

int getTradingDay(int beginDate, int endDate, std::vector<int>& tradingDays)
{
    // return MarketSessionManager::getInstance()->getTradingDay(MARKET_CHINAFUTURE, beginDate, endDate, tradingDays);
    if (beginDate > endDate || beginDate <= 19800101 || endDate <= 19800101)
    {
        return -1;
    }

    for (int d = beginDate; d < endDate;)
    {
        tradingDays.push_back(d);
        d = TimeUtil::nextDate(d);
    }
    return 0;
}

string getMapvalue(string sKey,map<string,string> mapValue,string sDefaultV)
{
	map<string, string>::iterator iter;
	iter = mapValue.find(sKey);

	if (iter != mapValue.end())
	{
		return iter->second ;
	}
	return sDefaultV;
}

int factorLevel2(map<string, string> mapValue)
{
    std::string dataRoot = getMapvalue("dataRoot", mapValue,"/data/algo/level2data");
    std::string targetPath = getMapvalue("targetPath", mapValue,"/data/algo/factor");
    std::vector<std::string> factorList = TC_Common::sepstr<std::string>(getMapvalue("factorlist", mapValue,""), ",|");
    if (factorList.empty())
    {
        factorList.push_back("earlymoney");
		factorList.push_back("bigsmall");
    }

	int nRet = 0;
	targetPath = targetPath[targetPath.size() - 1] == '/' ? std::string(targetPath.data(), targetPath.size() - 1) : targetPath;
	if (!TC_File::isFileExist(targetPath))
	{
		nRet = _mkdir(targetPath.c_str());
		if (nRet != 0)
		{
			std::cerr << "Create targetPath: " << targetPath << " failed" << std::endl;
			return nRet;
		}
	}

	//创建存储因子的文件夹
	for (string factorSaveName : factorList)
	{
		string factorSavePath = targetPath + "\\" + factorSaveName + "\\";
		if (!TC_File::isFileExist(factorSavePath))
		{
			nRet = _mkdir(factorSavePath.c_str());
			if (nRet != 0)
			{
				std::cerr << "Create Factor Savepath: " << factorSavePath << " failed" << std::endl;
				return nRet;
			}
		}
	}

    int beginDate = TC_Common::strto<int>(getMapvalue("begindate", mapValue, ""),  "0");
    int endDate = TC_Common::strto<int>(getMapvalue("enddate", mapValue, ""), "0");
    if (beginDate == 0)
    {
        beginDate = TC_Common::strto<int>(TC_Common::now2str("%Y%m%d"));
        // 15点前,就往前推一天
        if (TC_Common::strto<int>(TC_Common::now2str("%H%M%S")) < 1500)
        {
            beginDate = TimeUtil::prevDate(beginDate);
        }
    }
    if (endDate == 0)
    {
        endDate = TC_Common::strto<int>(TC_Common::now2str("%Y%m%d"));
        // 15点前,就往前推一天
        if (TC_Common::strto<int>(TC_Common::now2str("%H%M%S")) < 1500)
        {
            endDate = TimeUtil::prevDate(endDate);
        }
    }

    if (dataRoot.empty())
    {
        /*LOG_ERROR << "ERROR: parameter 'dataRoot' and 'end' is necessary." << std::endl;
        LOG_LOGIC_ERROR("ERROR: parameter 'dataRoot' and 'end' is necessary." << std::endl);*/

		std::cerr << "ERROR: parameter 'dataRoot' and 'end' is necessary." << std::endl;
		std::cerr << "ERROR: parameter 'dataRoot' and 'end' is necessary." << std::endl;
        return 1;
    }
    // 获取时间范围内的交易日
    std::vector<int> tradingDays;
    int rc = getTradingDay(beginDate, endDate, tradingDays);
    if (tradingDays.empty() || rc != 0)
    {
        /*LOG_ERROR "Can't find any trade date: beginDate=" << beginDate << "|endDate=" << endDate << std::to_string(rc);
        LOG_LOGIC_ERROR("Can't find any trade date: beginDate=" << beginDate << "|endDate=" << endDate
                                                                << std::to_string(rc));*/
		std::cerr << "Can't find any trade date: beginDate=" << beginDate << "|endDate=" << endDate << std::to_string(rc);
		std::cerr << "Can't find any trade date: beginDate=" << beginDate << "|endDate=" << endDate
			<< std::to_string(rc);
        return 0;
    }
    //LOG_DEBUG << TC_Common::tostr(tradingDays.begin(), tradingDays.end()) << std::endl;
	std::cout << TC_Common::tostr(tradingDays.begin(), tradingDays.end()) << std::endl;

    vector<string> sourceSet{"szlv2order", "szlv2trans", "shlv2trans"};
    // 时间范围: 左闭右开区间
    std::vector<std::pair<int, int>> timefilter{{91500, 92501}, {93000, 113001}, {130000, 150001}};

    // 按天处理,避免修复数据是存储空间不足的问题
    for (auto const &d : tradingDays)
    {
        // 下载原始数据
        // 使用 shell 脚本下载
        //std::string cmd = std::string("bash level2downloader.sh -d ") + std::to_string(d);
		std::string tmpdataRoot(dataRoot);
		replace(tmpdataRoot.begin(), tmpdataRoot.end(), '/', '\\');
		std::string cmd = std::string("..\\scripts\\level2downloader.bat ") + std::to_string(d) + " " + tmpdataRoot;
        int rc = ::system(cmd.c_str());
        if (rc != 0)
        {
			if (rc == 1)
				std::cerr << "file not exist: tradeDate" << d << std::endl;
			else if (rc ==2)
				std::cerr << "system not have 7z.exe ,please install first" << std::endl;
			else if (rc ==3)
				std::cerr << "file unzip error: tradeDate" << d << std::endl;
			else
				std::cerr << "unkown error: tradeDate " << d << std::endl;
            //LOG_ERROR << "download level2 source data failed: tradeDate " << d << std::endl;
			//std::cerr << "upzip level2 source data failed: tradeDate " << d << std::endl;
            continue;
        }

        try
        {
			time_t start, finish;
			start = clock();
            //l2::DailyFactors daily;
            l2::BigSmall bigsmall;
			l2::EarlyMoney earlymoney;
            std::vector<int> days;
            days.push_back(d);
            l2::Market market(dataRoot, targetPath, days, sourceSet, timefilter);
            // 注册数据处理器
            if (std::find(factorList.begin(), factorList.end(), "earlymoney") != factorList.end())
            {
                market.RegHandler(&earlymoney);
				earlymoney.SetFactorSaveName("earlymoney");
            }
            if (std::find(factorList.begin(), factorList.end(), "bigsmall") != factorList.end())
            {
                market.RegHandler(&bigsmall);
				bigsmall.SetFactorSaveName("bigsmall");
            }
            market.Run(3);
			finish = clock();
            //LOG_DEBUG << "tradedate=" << d << "|process finished elapse " << (t.elapse() / 1000) << " s" << std::endl;
			std::cout << "tradedate=" << d << "|process finished elapse " << ((finish-start) / CLOCKS_PER_SEC) << " s" << std::endl;
			replace(dataRoot.begin(), dataRoot.end(), '/', '\\');
			std::string rmfile = "rd /S/Q " + dataRoot + std::to_string(d);
            ::system(rmfile.c_str());
        }
        catch (std::exception &e)
        {
            //LOG_ERROR << "tradedate=" << d << "|level2 exception: " << e.what() << std::endl;
			std::cerr << "tradedate=" << d << "|level2 exception: " << e.what() << std::endl;
        }
    }
    return 0;
}
