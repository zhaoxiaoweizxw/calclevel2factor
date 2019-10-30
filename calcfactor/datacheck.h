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
	因子1：逐笔成交量求和。
	**/
	struct DataCheckValues
	{
		std::map<Symbol, std::vector<double> > transVolSumVec;// 1.div, 2.symbol

		DataCheckValues(const std::vector<StockOrder> &stkVec);
	};

	class dataCheckL2 : public KBarHandler
	{
	private:

		//std::vector<Symbol> _symbollist;

	public:
		dataCheckL2();

		//
		void OnBar(Market* mkt, int tradeDate, std::vector<StockOrder> &stkVec) override;


		//因子要一天一天的存，这样可以任务中断的时候从中间继续，而不是从头再来
		/**
		@param tradeDate	存放的日期
		@param values		要存储的数据
		**/
		bool FactorToFile(Market *mkt, int tradeDate, std::vector<Symbol>& symbollist, DataCheckValues &values);
	};
}
