
#include <direct.h>
#include <thread>
#include <algorithm>
#include "BigSmall.h"
#include "DailyFactors.h"
#include "earlymoney.h"
#include "aggauctionAM.h"
#include "softbigsmall.h"
#include "datacheck.h"
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
	std::string threadnumb = getMapvalue("threadnum", mapValue, "3");

    std::string dataRoot = getMapvalue("dataRoot", mapValue,"/data/algo/level2data");
    std::string targetPath = getMapvalue("targetPath", mapValue,"/data/algo/factor");
	std::string FastworkPath = getMapvalue("fastrunPath", mapValue, dataRoot);
    std::vector<std::string> factorList = TC_Common::sepstr<std::string>(getMapvalue("factorlist", mapValue,""), ",|");
    if (factorList.empty())
    {
        factorList.push_back("datacheck");
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

	FastworkPath = FastworkPath[FastworkPath.size() - 1] == '/' ? std::string(FastworkPath.data(), FastworkPath.size() - 1) : FastworkPath;
	if (!TC_File::isFileExist(FastworkPath))
	{
		nRet = _mkdir(FastworkPath.c_str());
		if (nRet != 0)
		{
			std::cerr << "Create fastwordPath: " << FastworkPath << " failed" << std::endl;
			return nRet;
		}
	}

	//创建存储因子的文件夹
	for (string factorSaveName : factorList)
	{
		string factorSavePath = targetPath + "/" + factorSaveName ;
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

	vector<string> sourceSet{ "szlv2order", "szlv2trans", "shlv2trans","quote" };
	// 时间范围: 左闭右开区间，开盘集合竞价把时间放到92601是因为股票的开盘价在92530以后才出来，这种现象不少
	std::vector<std::pair<int, int>> timefilter{ { 91500, 92601 },{ 93000, 113001 },{ 130000, 150001 } };

    // 按天处理,避免修复数据是存储空间不足的问题
    for (auto const &d : tradingDays)
    {
        // 下载原始数据
        // 使用 shell 脚本下载
        //std::string cmd = std::string("bash level2downloader.sh -d ") + std::to_string(d);
		std::string tmpdataRoot(dataRoot);
		std::string tmpfastRoot(FastworkPath);
		std::string cmd = std::string("../scripts/level2downloader.bat ") + std::to_string(d) + " " + tmpdataRoot + " "+ tmpfastRoot+'/';
		replace(cmd.begin(), cmd.end(), '/', '\\');
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
			l2::aggauctionAM aggaucInam;
			l2::SoftbigAmall softbigsmall(200);
			l2::dataCheckL2 datachecker;
            std::vector<int> days;
            days.push_back(d);
            l2::Market market(FastworkPath, targetPath, days, sourceSet, timefilter);
            // 注册数据处理器
            if (std::find(factorList.begin(), factorList.end(), "earlymoney") != factorList.end())
            {
				earlymoney.SetFactorSaveName("earlymoney");
                market.RegHandler(&earlymoney);
            }
            if (std::find(factorList.begin(), factorList.end(), "bigsmall") != factorList.end())
            {
				bigsmall.SetFactorSaveName("bigsmall");
                market.RegHandler(&bigsmall);
            }
			if (std::find(factorList.begin(), factorList.end(), "aggAucAm") != factorList.end())
			{
				aggaucInam.SetFactorSaveName("aggAucAm");
				market.RegHandler(&aggaucInam);
			}
			if (std::find(factorList.begin(), factorList.end(), "softbigsmall") != factorList.end())
			{
				if (std::find(factorList.begin(), factorList.end(), "softbigsmallbreak") != factorList.end())
				{
					std::cerr << "Could not calc softbigsmall and softbigsmallbreak in one proc" << std::endl;
					return 0;
				}
				softbigsmall.SetFactorSaveName("softbigsmall");
				market.RegHandler(&softbigsmall);
			}
			if (std::find(factorList.begin(), factorList.end(), "softbigsmallbreak") != factorList.end())
			{
				softbigsmall.SetFactorSaveName("softbigsmallbreak");
				softbigsmall.setBreak(true);
				market.RegHandler(&softbigsmall);
			}
			if (std::find(factorList.begin(), factorList.end(), "datacheck") != factorList.end())
			{
				datachecker.SetFactorSaveName("datacheck");
				market.RegHandler(&datachecker);
			}
            market.Run(atoi(threadnumb.data()));
			finish = clock();
            //LOG_DEBUG << "tradedate=" << d << "|process finished elapse " << (t.elapse() / 1000) << " s" << std::endl;
			std::cout << "tradedate=" << d << "|process finished elapse " << ((finish-start) / CLOCKS_PER_SEC) << " s" << std::endl;
			std::string rmfile = "rd /S/Q " + FastworkPath + '/' + std::to_string(d);
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
