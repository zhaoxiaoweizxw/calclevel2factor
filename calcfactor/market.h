#pragma once
#include "datasource.h"
#include "public.h"
#include <list>
#include <set>
#include <utility>
#include <map>
#include <vector>
#include <unordered_map>
using std::list;
using std::set;
using std::string;

namespace l2 {


class Market;
typedef void (*BarMonitor)(Market &);

struct tagLevel2File {
	std::string quotafile;
	std::string transfile;
	std::string orderfile;
};
class Market
{
    private:
    std::vector<int> _tradingDays;
    std::vector<KBarHandler*> _handler;
    // <{sz,sh}{SYMBOL}, <trans,order>>
    std::unordered_map<string, tagLevel2File > _csvFiles;
    int _symbolMaxSize{0};
  protected:
    string dataPath;
	string outPath;
    vector<string> sourceSet;

    std::vector<std::pair<int,int>> _timefilter;
  public:
    Market(const string& dataPath, const std::string& outPath,const std::vector<int>& tradingDays, const vector<string>& sourceSet,
           const std::vector<std::pair<int,int>>& timefilter);
    ~Market();
    void Run(int threadCnt = 3);
    void ProcessOne(int tradeDate, std::vector<StockOrder> &stkVec);
    void GetSymbolList(std::vector<Symbol> &symbolList);

    void RegHandler(KBarHandler* h) {_handler.push_back(h);}

    static bool IsStock(const string &symbol);
    // <{sz,sh}{SYMBOL}, <trans,order>>
    void GetStockFile(int tradeDate, std::unordered_map<string, tagLevel2File > &fileNameSet);
    static void GetStockFile(const string& pathStr, std::unordered_map<string, tagLevel2File> &fileNameSet);
    static const string GetSymbolStr(const Symbol symbol);
    size_t GetSymbolMaxSize();

	std::set<int> stkSet;
	bool FactorToFile(std::string& factorSaveName, int tradeDate, std::string& factortitle, std::unordered_map<int, std::vector<double>>& results);
};

}
