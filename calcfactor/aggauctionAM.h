#pragma once
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
	因子1：隔夜涨幅，即开盘价相对前收盘价的涨幅。
	因子2：阶段1（9:15-9:20，该阶段可以撤单）涨幅；
	因子3：阶段2（9:20-9:25，该阶段无法撤单）涨幅；
	因子4：集合竞价量比（240×集合竞价成交额/过去五日日均成交额）；#保存成交额即可
	因子5：阶段1是否涨停；# 保存最高价
	因子6：阶段1是否跌停；
	因子7：阶段2价格是否平稳上升。
	因子8：因子7的辅助，记录阶段2价格个数。
	因子9：因子5的辅助，记录该股票的涨停价。
	因子10：因子6的辅助，记录该股票的跌停价。
	**/
	struct AggAuctionValues
	{
		std::map<Symbol, std::vector<double> > crsDayPctVec;// 1.div, 2.symbol
		std::map<Symbol, std::vector<double> > stage1PctVec;
		std::map<Symbol, std::vector<double> > stage2PctVec;
		std::map<Symbol, std::vector<double> > aggAucAmAmoutVec;
		std::map<Symbol, std::vector<double> > stage1HighVec;
		std::map<Symbol, std::vector<double> > stage1LowVec;
		std::map<Symbol, std::vector<double> > stepUpDownVec;
		std::map<Symbol, std::vector<double> > step2DnumVec;
		std::map<Symbol, std::vector<double> > highLimitVec;
		std::map<Symbol, std::vector<double> > lowLimitVec;

		AggAuctionValues(const std::vector<StockOrder> &stkVec);
	};

	class aggauctionAM : public KBarHandler
	{
	private:

		//std::vector<Symbol> _symbollist;

	public:
		aggauctionAM();

		//
		void OnBar(Market* mkt, int tradeDate, std::vector<StockOrder> &stkVec) override;


		//因子要一天一天的存，这样可以任务中断的时候从中间继续，而不是从头再来
		/**
		@param tradeDate	存放的日期
		@param values		要存储的数据
		**/
		bool FactorToFile(Market *mkt, int tradeDate, std::vector<Symbol>& symbollist, AggAuctionValues &values);
	};
}
