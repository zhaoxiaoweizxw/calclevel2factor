#include "market.h"
#include "../utils/utils_common.h"
#include <algorithm>
#include <thread>
#include <time.h>
#include <mutex>

// using boost::lexical_cast;
using namespace std;
// using namespace boost::posix_time;
// using namespace boost::gregorian;
// using namespace boost::filesystem;
// using boost::format;

#define LOG_DATETIME(m, d, t)                                                                                          \
    do                                                                                                                 \
    {                                                                                                                  \
        time_t __tt__ = TC_Common::str2time(std::to_string(d * 1000000L)) + t;                                         \
        tm *__t__ = ::localtime(&__tt__);                                                                              \
        LOG_DEBUG << m << "|" << TC_Common::tm2str(*__t__) << " " << t << std::endl;                                   \
    } while (0)
// #define LOG_DATETIME(d,t)

namespace l2
{

	std::mutex mtx;

// Market -------------------------------------------------
Market::Market(const string &dataPath, const string &outPath, const std::vector<int> &tradingDays,
               const vector<string> &sourceSet, const std::vector<std::pair<int, int>> &timefilter)
{
    this->_timefilter = timefilter;
    this->_tradingDays = tradingDays;
    // this->_channels.resize(1);
    // this->symbolSet = symbolSet;
    this->sourceSet = sourceSet;
    this->dataPath =
        dataPath[dataPath.size() - 1] == '/' ? std::string(dataPath.data(), dataPath.size() - 1) : dataPath;
	this->outPath = 
		outPath[outPath.size() - 1] == '/' ? std::string(outPath.data(), outPath.size() - 1) : outPath;
}

Market::~Market() { _csvFiles.clear(); }

void Market::Run(int threadCnt)
{
    for (auto d : _tradingDays)
    {
        std::cout << "process trade date " << d << std::endl;
		//LOG_DEBUG << "process trade date " << d << std::endl;
        _csvFiles.clear();
        GetStockFile(d, _csvFiles);

        std::string currentDatePath(dataPath + "/" + std::to_string(d));
        if (!TC_File::isFileExist(currentDatePath))
        {
            /*LOG_DEBUG << "file not exist: " << dataPath << "/" << std::to_string(d) << std::endl;*/
			std::cout << "file not exist: " << dataPath << "/" << std::to_string(d) << std::endl;
            return;
        }

        std::vector<std::thread> thdGroup;
        std::vector<std::vector<StockOrder>> stkGroup;
        stkGroup.resize(threadCnt);
        thdGroup.reserve(threadCnt);

        // 分组计算
        int cnt = 0;
        for (auto const &v : _csvFiles)
        {
			/*if (v.first.substr(0, 8).compare("sz000901") != 0)
				continue;*/

            Symbol symbol = std::stoi(v.first.substr(2, 6));
			//不处理指数
			if (v.first[1] == 'h' && symbol < 600000)
				continue;
            const std::string &trans = v.second.transfile;
            const std::string &order = v.second.orderfile;
			const std::string &quota = v.second.quotafile;
            stkGroup[cnt % threadCnt].emplace_back(StockOrder(symbol, trans, order, quota));
			
            cnt++;
			/*if (cnt >= 20)
				break;*/
        }
        // 多线程
        for (size_t i = 0; i < stkGroup.size(); i++)
        {
            if (stkGroup.empty())
            {
                continue;
            }
            std::thread t(std::bind(&Market::ProcessOne, this, d, std::ref(stkGroup[i])));
            thdGroup.push_back(std::move(t));
        }
        for (auto &t : thdGroup)
        {
            t.join();
        }
    }
}

void Market::ProcessOne(int tradeDate, std::vector<StockOrder> &stkVec)
{
    // 加载数据
	int i = 0;

	clock_t startTime, endTime;
	startTime = clock();//计时开始

	//线程内分组，节约内存
	int nPerSlipeStks = 20;
	int nSlipe = ceil(stkVec.size() * 1.0 / nPerSlipeStks);
	for (int j = 0; j < nSlipe; ++j)
	{
		std::vector<StockOrder>::const_iterator first1 = stkVec.begin() + j * nPerSlipeStks;
		std::vector<StockOrder>::const_iterator last1 = stkVec.end() ;
		if (j < nSlipe-1)
			last1 = stkVec.begin() + j * nPerSlipeStks + nPerSlipeStks;
		std::vector<StockOrder> stkSlipVec(first1, last1);
				
		int nloadtime = 0;
		int npreparetime = 0;
		for (auto &stk : stkSlipVec)
		{
			startTime = clock();//计时开始
			stk.Load(_timefilter);
			endTime = clock();//计时结束
			nloadtime += (endTime - startTime);

			startTime = clock();//计时开始
			stk.PrepareData();
			endTime = clock();//计时结束
			npreparetime += (endTime - startTime);
		}

		/*cout << "The PrepareDate time is:" << nloadtime << "ms" << endl;
		cout << "The PrepareDate time is:" << npreparetime << "ms" << endl;*/

		startTime = clock();//计时开始
		// 计算
		for (auto h : _handler)
		{
			h->OnBar(this, tradeDate, stkSlipVec);
		}
		endTime = clock();//计时结束

		//cout << "The handle time is:" << (endTime - startTime)  << "ms" << endl;

		//std::cout << "handleDate allready:" << "  --threadID: " << std::this_thread::get_id() << std::endl;
		// 释放资源
		for (auto &v : stkSlipVec)
		{
			v.Release();
		}
	}
}

void Market::GetSymbolList(std::vector<Symbol> &symbolList)
{
    for (auto const &v : _csvFiles)
    {
        Symbol symbol = std::stoi(v.first.substr(2, 6));
        // LOG_DEBUG << "SYMBOL: " << v.first.substr(2, 6) << std::endl;
        auto it = std::find(symbolList.begin(), symbolList.end(), symbol);
        if (it == symbolList.end())
        {
            symbolList.push_back(symbol);
        }
    }
}


bool Market::IsStock(const string &symbol)
{
    if (symbol.size() < 6)
    {
        return false;
    }
    if (symbol.substr(0, 3) == string("000") || symbol.substr(0, 3) == string("001") ||
        symbol.substr(0, 3) == string("002") || symbol.substr(0, 3) == string("300") ||
        symbol.substr(0, 3) == string("600") || symbol.substr(0, 3) == string("601") ||
        symbol.substr(0, 3) == string("603") || symbol.substr(0, 3) == string("688"))
    {
        return true;
    }
    return false;
}

void Market::GetStockFile(int tradeDate, std::unordered_map<string, tagLevel2File> &fileNameSet)
{

    for (auto &s : sourceSet)
    {
        GetStockFile(dataPath + "/" + std::to_string(tradeDate) + "/" + s, fileNameSet);
    }
}
void Market::GetStockFile(const std::string &pathStr,
                          std::unordered_map<std::string, tagLevel2File> &fileNameSet)
{
    // <sz{SYMBOL}, <trans,order>>
    // std::unordered_map<std::string, std::pair<std::string, std::string>> symfile;

    if (!TC_File::isFileExist(pathStr))
    {
        //LOG_ERROR << "path does not exists " << pathStr << std::endl;
		std::cerr << "path does not exists " << pathStr << std::endl;
        return;
    }
    std::vector<std::string> dirs;
    TC_File::listDirectory(pathStr, dirs, true);
    for (auto const &fileName : dirs)
    {
        // LOG_DEBUG << "FILE: " << fileName << std::endl;
        std::string sym;
        try
        {
            sym = fileName.substr(fileName.size() - 10, 6);
        }
        catch (std::exception &e)
        {
            //LOG_ERROR << "exception: " << e.what() << "|" << fileName << std::endl;
			std::cerr << "exception: " << e.what() << "|" << fileName << std::endl;
            continue;
        }
        // LOG_DEBUG << "SYMBOL: " <<  sym << std::endl;
        if (Market::IsStock(sym))
        {
            // fileNameSet.emplace_back(std::make_pair(fileName, std::stoi(sym)));

            if (fileName.find("order") != std::string::npos)
            {
                // LOG_DEBUG << "order " << fileName << std::endl;
                fileNameSet[fileName.substr(fileName.size() - 12, 8)].orderfile = fileName;
            }
            else if (fileName.find("trans") != std::string::npos)
            {
                // LOG_DEBUG << "trans " << fileName << std::endl;
                fileNameSet[fileName.substr(fileName.size() - 12, 8)].transfile = fileName;
            }
			else if (fileName.find("quote") != std::string::npos)
			{
				// LOG_DEBUG << "trans " << fileName << std::endl;
				fileNameSet[fileName.substr(fileName.size() - 12, 8)].quotafile = fileName;
			}
        }
    }
}

const string Market::GetSymbolStr(const Symbol symbol)
{
    char buf[16] = {0x00};
    std::snprintf(buf, sizeof(buf), "%06d.CS", symbol);
    return std::string(buf);
}

size_t Market::GetSymbolMaxSize() { return _csvFiles.size(); }


bool Market::FactorToFile(std::string& factorSaveName, int tradeDate, std::string& factortitle, std::unordered_map<int, std::vector<double>>& results)
{
	{
		std::lock_guard<std::mutex> lck(mtx);


		std::string curDayFactorFile = outPath + "/" + factorSaveName + "/" + std::to_string(tradeDate) + ".csv";
		bool bExist = TC_File::isFileExist(curDayFactorFile);

		ofstream outFactorFile;
		outFactorFile.open(curDayFactorFile, ios::app);
		if (!bExist)
			outFactorFile << factortitle << std::endl;

		//std::cout << "save stklist:" << endl;
		for (auto& v : results)
		{
			/*if (stkSet.find(v.first) != stkSet.end())
				std::cerr << v.first << " allready in file !!!!!!!" << endl;
			stkSet.emplace(v.first);
			std::cout << v.first <<" ";*/

			outFactorFile << v.first;
			std::vector<double>& vvalue = v.second;
			for (size_t i = 0; i < vvalue.size(); i++)
				outFactorFile << "," << vvalue[i];
			outFactorFile << std::endl;
		}
		//std::cout <<  endl;

		outFactorFile.close();

		//std::cout << "Outer save factors...threadID: " << std::this_thread::get_id() << std::endl;
	}

	return true;
}

}
