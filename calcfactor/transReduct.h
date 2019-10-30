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
	struct TransRestoreValues
	{
		//成交还原因子
		std::map<Symbol, std::vector<double> > transInfoVec;// 1.div, 2.symbol
		std::vector<std::string> TitleVec;

		TransRestoreValues(const std::vector<StockOrder> &stkVec);
	};

	//成交和委托的辅助信息，不影响其他因子的计算
	class clsTransExt
	{
	public:
		Trans* pTrans;
		int nBidBaseVol;
		double nBidBaseAmo;
		int nOfferBaseVol;
		double nOfferBaseAmo;
		bool bIsActiveBuy;
		bool bIsCrossPrice;
	};

	class clsOrderExt
	{
	public:
		clsOrderExt();
		Order* pOrder;
		//成交信息
		int nActVol;
		double nActAmo;
		int nPasVol;
		double dPasAmo;
		int nCancelVol;
		//委托信息
		int nSizekind;
		int nEnTruestVol;
		Price EnTrustPrice;
		double EnTrustAmo;
	};

	class clsTransReduct : public KBarHandler
	{
	private:

		//std::vector<Symbol> _symbollist;

	public:
		clsTransReduct();

		//
		void OnBar(Market* mkt, int tradeDate, std::vector<StockOrder> &stkVec) override;

		int ClassifyOfSize(int nEntrustVol,double nEntrustAmo);


		//因子要一天一天的存，这样可以任务中断的时候从中间继续，而不是从头再来
		/**
		@param tradeDate	存放的日期
		@param values		要存储的数据
		**/
		bool FactorToFile(Market *mkt, int tradeDate, std::vector<Symbol>& symbollist, TransRestoreValues &values);

	private:
		//vector<clsOrderExt> orderExtList;
		//vector<clsTransExt> transExtList;
		//std::map<OrderSeq, int> _orderIndexMap;

		ActiveType GetTradeAct(Symbol netsymbol, int ntradetime, int nDate, Price price, Price lstprice, bool& bBreaked, OrderSeq bidSeq, OrderSeq offerSeq, OrderSeq lstBidSeq, OrderSeq lstOfferSeq, ActiveType lstActType, const Quote& q, clsTransExt& extTrans);
		void CalcOrderSizeRange(clsOrderExt& order, clsTransExt& extTrans, Price curPrice);
		void initOrderExt(const vector<Order> &orderList, vector<clsOrderExt>& orderExtList, std::map<OrderSeq, int> _orderIndexMap);
		bool GetOrder(OrderSeq seq, const std::map<OrderSeq, int>& _orderIndexMap,const vector<clsOrderExt>& orderExtList, clsOrderExt &o);
	};
}
