#pragma once
#include "market.h"
#include "public.h"
#include <list>
#include <string>
#include <vector>
using std::list;
using std::string;
using std::vector;
namespace l2 {

struct BigSmallValues
{
    std::map<Symbol, std::vector< double>> buyVolVec; // 1.div, 2.symbol, 3.days
    std::map<Symbol, std::vector< double>> sellVolVec;
    std::map<Symbol, std::vector< double>> quitsVolVec;
    std::map<Symbol, std::vector< double>> buyAmountVec;
     std::map<Symbol, std::vector< double>> sellAmountVec;
    std::map<Symbol, std::vector< double>>  quitsAmountVec;

    BigSmallValues(const std::vector<StockOrder> &stkVec);
};

class BigSmall: public KBarHandler
{
  private:

	  //std::vector<Symbol> _symbollist;
	  // @param factorName	因子的系列名称，以此建立文件夹
	  std::string _factorSaveName;


public:
    BigSmall();

	void SetFactorSaveName(const char* strName);
    //
    void OnBar(Market* mkt, int tradeDate, std::vector<StockOrder> &stkVec) override;

    bool ToSQLite(Market *mkt, int tradeDate, BigSmallValues &values);

	//因子要一天一天的存，这样可以任务中断的时候从中间继续，而不是从头再来
	/**
	@param tradeDate	存放的日期
	@param values		要存储的数据
	**/
	bool FactorToFile(Market *mkt,int tradeDate, std::vector<Symbol>& symbollist,BigSmallValues &values);
};
}
