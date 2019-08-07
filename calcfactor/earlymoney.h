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

/**
统计主动、被动交易的大小单
0~4万为小单、4~20万为中单、20万到100万为大单
**/
struct ActPasValues
{
    std::map<Symbol, std::vector< double>> ActivebuyAmtVec;// 1.div, 2.symbol
    std::map<Symbol, std::vector< double>> ActiveselAmtVec;
	std::map<Symbol, std::vector< double>> ActivebuyVolVec;// 1.div, 2.symbol
	std::map<Symbol, std::vector< double>> ActiveselVolVec;
	//std::map<Symbol, std::vector< double>> PasivebuyAmtVec;// 1.div, 2.symbol
	//std::map<Symbol, std::vector< double>> PasiveselAmtVec;

	ActPasValues(const std::vector<StockOrder> &stkVec);
};

class EarlyMoney: public KBarHandler
{
  private:

	  //std::vector<Symbol> _symbollist;

public:
	EarlyMoney();

    //
    void OnBar(Market* mkt, int tradeDate, std::vector<StockOrder> &stkVec) override;


	//因子要一天一天的存，这样可以任务中断的时候从中间继续，而不是从头再来
	/**
	@param tradeDate	存放的日期
	@param values		要存储的数据
	**/
	bool FactorToFile(Market *mkt,int tradeDate, std::vector<Symbol>& symbollist, ActPasValues &values);
};
}
