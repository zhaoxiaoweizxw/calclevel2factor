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
	����1����ʳɽ�����͡�
	**/
	struct TransRestoreValues
	{
		//�ɽ���ԭ����
		std::map<Symbol, std::vector<double> > transInfoVec;// 1.div, 2.symbol
		std::vector<std::string> TitleVec;

		TransRestoreValues(const std::vector<StockOrder> &stkVec);
	};

	//�ɽ���ί�еĸ�����Ϣ����Ӱ���������ӵļ���
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
		//�ɽ���Ϣ
		int nActVol;
		double nActAmo;
		int nPasVol;
		double dPasAmo;
		int nCancelVol;
		//ί����Ϣ
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


		//����Ҫһ��һ��Ĵ棬�������������жϵ�ʱ����м�����������Ǵ�ͷ����
		/**
		@param tradeDate	��ŵ�����
		@param values		Ҫ�洢������
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
