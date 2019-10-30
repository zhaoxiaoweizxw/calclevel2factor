
#include "transReduct.h"
#include "../utils/utils_common.h"
#include <cmath>
#include <iostream>
#include <algorithm>

using std::cout;
using std::endl;
using std::fixed;
using std::max;
using std::min;
using std::range_error;
namespace l2
{

	/**
	ActiveSelΪ���������
	ActiveUnkΪ�жϲ���
	ActiveBuyΪ������������
	**/
	enum ActiveType { ActiveSel, ActiveUnk, ActiveBuy };

	double superMinPoint = 0.00001;
	static const char *ACTKIND_NAME[] = {"CRS", "ACT",   "PAS",""};// ������������Խ��,��
	static const char *SIZEKIND_NAME[] = { "HUGE",  "BIG", "MID",  "SML","MAIN" ,"RET"};//�ش󵥡��󵥡��е���С��
	static const char *BSKIND_NAME[] = { "B",  "S", "NET"};//����������
	static const char *STATKIND_NAME[] = { "TV", "TM", "T" ,"PM","P" };//�������ܶ������ÿ�ʽ�����
	static const unsigned int ACTKIND_NUMB = sizeof(ACTKIND_NAME) / sizeof(ACTKIND_NAME[0]);
	static const unsigned int SIZEKIND_NUMB = sizeof(SIZEKIND_NAME) / sizeof(SIZEKIND_NAME[0]);
	static const unsigned int BSKIND_NUMB = sizeof(BSKIND_NAME) / sizeof(BSKIND_NAME[0]);
	static const unsigned int STATKIND_NUMB = sizeof(STATKIND_NAME) / sizeof(STATKIND_NAME[0]);
	static const unsigned int DIV_SIZE = ACTKIND_NUMB * SIZEKIND_NUMB * BSKIND_NUMB * STATKIND_NUMB;


	TransRestoreValues::TransRestoreValues(const std::vector<StockOrder> &stkVec)
	{
		for (auto const &stk : stkVec)
		{
			transInfoVec[stk.GetSymbol()].resize(DIV_SIZE);
			for (size_t d = 0; d < DIV_SIZE; d++)
				transInfoVec[stk.GetSymbol()].at(d) = 0.0;
		}
		//����ָ�������
		for (int k1 = 0; k1 < ACTKIND_NUMB; k1++)
			for (int k2 = 0; k2 < SIZEKIND_NUMB; k2++)
				for (int k3 = 0; k3 < BSKIND_NUMB; k3++)
					for (int k4 = 0; k4 < STATKIND_NUMB; k4++)
					{
						string strTitle = string(ACTKIND_NAME[k1]) + "_" + string(SIZEKIND_NAME[k2]) + "_" + string(BSKIND_NAME[k3]) + "_" + string(STATKIND_NAME[k4]) ;
						TitleVec.push_back(strTitle);
					}
	}
	clsOrderExt::clsOrderExt():nSizekind(-1)
	{
	}

	///
	//1����ÿ�ʳɽ����ֳ�������������Խ��
	//2���õ���ί�е��Ĵ�С�������ں����жϸñʳɽ��Ľ���˫�������������Ǳ���
	//3���������ͳ�ƣ�����ۡ�������
	///
	void clsTransReduct::OnBar(Market *mkt, int tradeDate, std::vector<StockOrder> &stkVec)
	{

		std::vector<Symbol> symbollist;
		symbollist.reserve(stkVec.size());

		TransRestoreValues values(stkVec);
		for (auto &stk : stkVec)
		{
			Symbol netsymbol = stk.GetSymbol();
			symbollist.emplace_back(netsymbol);
			const vector<Trans> &transList = stk.GetTrans(); // market->GetTransList(symbol);
			const vector<Quote> &quoteList = stk.GetQuote(); // market->GetTransList(symbol);
			const vector<Order> &orderList = stk.GetOrder(); // market->GetTransList(symbol);

			//��չ��trans��orders
			vector<clsOrderExt> orderExtList;
			vector<clsTransExt> transExtList;
			std::map<OrderSeq, int> _orderIndexMap;

			transExtList.reserve(transList.size());
			orderExtList.reserve(orderList.size());
			initOrderExt(orderList, orderExtList, _orderIndexMap);

			//�������ݼ��жϵĸ�����Ϣ
			OrderSeq lstBidSeq = 0, lstOfferSeq = 0;
			long long lstbidAmt = 0, lstofferAmt = 0;
			long lstbidVol = 0, lstofferVol = 0;
			ActiveType lstActType = ActiveType::ActiveUnk;
			ActiveType crtActType = ActiveType::ActiveUnk;

			Price price = 0;
			Price lstprice = 0.0;

			//�ж�Խ�۵��ĸ�����Ϣ
			bool  bBreaked = false;

			for (const Trans &trans : transList)
			{
				const Trans *transInfo = &trans;
				
				int ntradetime = transInfo->time;
				int nDate = transInfo->date;

				
				//�������պ�ί��
				clsTransExt extTrans;
				extTrans.pTrans = const_cast<Trans*>(transInfo);

				Quote q;
				if (!stk.GetPreTimeQuote(ntradetime, q))
				{
					std::cerr << " Get quote error!!" << tradeDate << " " << netsymbol << " " << ntradetime << endl;
					continue;
				}
				//

				OrderSeq bidSeq = transInfo->bidSeq;
				OrderSeq offerSeq = transInfo->offerSeq;
				bool bSearchOrderB, bSearchOrderS;

				clsOrderExt orderB, orderS;
				bSearchOrderB = GetOrder(bidSeq, _orderIndexMap, orderExtList, orderB);
				bSearchOrderS = GetOrder(offerSeq, _orderIndexMap, orderExtList, orderS);
				if (bSearchOrderB || bSearchOrderS)
				{
					std::cerr << "Get order Error " << tradeDate << " " << netsymbol << " " << transInfo->bidSeq << " " << transInfo->offerSeq << endl;
					continue;
				}
				CalcOrderSizeRange(orderB, extTrans, q.curKClose);
				CalcOrderSizeRange(orderS, extTrans, q.curKClose);
				

				//�жϳɽ������Խ�۵�
				if (transInfo->type == 'F' || transInfo->type == 'N')
				{
					crtActType = GetTradeAct(netsymbol, ntradetime, nDate, price, lstprice, bBreaked, transInfo->bidSeq, transInfo->offerSeq,lstBidSeq, lstOfferSeq, lstActType, q, extTrans);
					lstBidSeq = transInfo->bidSeq;
					lstOfferSeq = transInfo->offerSeq;
					if (ntradetime < 145700 || (netsymbol > 600000 && tradeDate < 20180820))
						lstprice = price;
					lstActType = crtActType;

					if (crtActType == ActiveType::ActiveBuy)
					{
						orderB.nActAmo += transInfo->num;
					}
				}
				transExtList.push_back(extTrans);
			}

			double dActBuyHugeAmo, dActBuyBigAmo, dActBuyMidAmo, dActBuySmlAmo;
			double dActBuyHugeVol, dActBuyBigVol, dActBuyMidVol, dActBuySmlVol;
			for (const clsTransExt&  extTrans: transExtList)
			{
				double dValArray[ACTKIND_NUMB][SIZEKIND_NUMB][BSKIND_NUMB][STATKIND_NUMB] = { 0 };

				int nBidSizeRank = ClassifyOfSize(extTrans.nBidBaseVol,extTrans.nBidBaseAmo);
				int nOfferSizeRank = ClassifyOfSize(extTrans.nOfferBaseVol, extTrans.nOfferBaseAmo);

				vector<int > vecIndexAct,vecIndexSize,vecIndexBS;
				//��
				if (extTrans.bIsActiveBuy)
				{
					vecIndexAct.push_back(0);
					if (extTrans.bIsCrossPrice)
						vecIndexAct.push_back(2);
					vecIndexSize.push_back(nBidSizeRank);
					vecIndexBS.push_back(0);
					for (int k1 : vecIndexAct)
						for (int k2 : vecIndexSize)
							for (int k3 : vecIndexBS)
							{
								//�������ܶ������ÿ��������
								dValArray[0][nBidSizeRank][1][0] += extTrans.pTrans->num;
								dValArray[0][nBidSizeRank][1][1] += extTrans.pTrans->num * extTrans.pTrans ->price/100.0 ;
							}

				}
				
			}

		}


		FactorToFile(mkt, tradeDate, symbollist, values);
		//ToSQLite(mkt, tradeDate, values);
	}
	//�����ж�����Ϣ��order�б�����������
	void clsTransReduct::initOrderExt(const vector<Order> &orderList, vector<clsOrderExt>& orderExtList, std::map<OrderSeq, int> _orderIndexMap)
	{
		//�����м۵�û�м۸񣬴˴������ܼ������ж�����ί�н��
		for (int i = 0; i < orderList.size();i++)
		{
			clsOrderExt extOrder;
			extOrder.pOrder = const_cast<Order*>(&orderList[i]);
			orderExtList.push_back(extOrder);
			_orderIndexMap.insert(make_pair(orderList[i].seq, i));
		}
	}
	bool clsTransReduct::GetOrder(OrderSeq seq, const std::map<OrderSeq, int>& _orderIndexMap, const vector<clsOrderExt>& orderExtList, clsOrderExt &o)
	{
		auto itIndex = _orderIndexMap.find(seq);
		if (itIndex == _orderIndexMap.end())
		{
			return false;
		}
		o = orderExtList.at(itIndex->second);
		return true;
	}

	//���չtrans��baseAmo��Ϣ
	void clsTransReduct::CalcOrderSizeRange(clsOrderExt& order,clsTransExt& extTrans,Price curPrice)
	{
		if (order.pOrder->direct == 1)
		{
			extTrans.nBidBaseVol = order.pOrder->num;
			if (order.nSizekind < 0)
			{
				//��Ч�۸�
				Price effectPrice = order.pOrder->price;
				if (order.pOrder->type == 'U')
					effectPrice = curPrice;
				extTrans.nBidBaseAmo = order.pOrder->num * effectPrice;

				order.nEnTruestVol = order.pOrder->num;
				order.EnTrustPrice = effectPrice;
				order.EnTrustAmo = extTrans.nBidBaseAmo;
				order.nSizekind = ClassifyOfSize(order.nEnTruestVol, order.EnTrustAmo);
			}
			else
				extTrans.nBidBaseAmo = order.EnTrustAmo;
		}
		else
		{
			extTrans.nOfferBaseVol = order.pOrder->num;
			if (order.nSizekind < 0)
			{
				//��Ч�۸�
				Price effectPrice = order.pOrder->price;
				if (order.pOrder->type == 'U')
					effectPrice = curPrice;
				extTrans.nOfferBaseAmo = order.pOrder->num * effectPrice;

				order.nEnTruestVol = order.pOrder->num;
				order.EnTrustPrice = effectPrice;
				order.EnTrustAmo = extTrans.nOfferBaseAmo;
				order.nSizekind = ClassifyOfSize(order.nEnTruestVol, order.EnTrustAmo);
			}
			else
				extTrans.nOfferBaseAmo = order.EnTrustAmo;
		}

	}

	ActiveType clsTransReduct::GetTradeAct(Symbol netsymbol, int ntradetime, int nDate, Price price, Price lstprice,bool& bBreaked, OrderSeq bidSeq, OrderSeq offerSeq, OrderSeq lstBidSeq, OrderSeq lstOfferSeq, ActiveType lstActType, const Quote& q, clsTransExt& extTrans)
	{
		//��������ĳ�������Ӧ��Ҳ���Ǳ�������
		//1������۸���ڵ�����һ�ʿ��յ���һ��
		//2����һ��Ϊ�գ��Ƿ���[0.098,0.102],[0.048,0.052]����һ�γɽ��۸���ڵ�����һ�ʼ۸��϶���ͣ����ʱ�ɽ����϶���������
		//3�����̼��Ͼ��ۣ��ɽ��۴�������
		//4��β�̼��Ͼ��ۣ��ɽ��������һ���۸�

		//
		ActiveType crtActType = ActiveType::ActiveUnk;
		if ((q.offerPrice[0] > superMinPoint && price >= q.offerPrice[0] && q.bidPrice[0] > superMinPoint && q.time > 92500) ||
			((q.offerPrice[0] < superMinPoint) && (price >= lstprice) && ((price < q.preClose * 1.105 && price > q.preClose * 1.095) || (price < q.preClose * 1.055 && price > q.preClose * 1.045))) ||
			((q.preClose < price) && (ntradetime <= 92500))
			)
		{
			crtActType = ActiveType::ActiveBuy;
		}

		//���������ĳ�������Ӧ��Ҳ���Ǳ�������
		//1�������۸�С�ڵ�����һ�ʿ��յ���һ��
		//2����һ��Ϊ�գ��Ƿ���[-0.098,-0.102],[-0.048,-0.052]����һ�γɽ��۸�С�ڵ�����һ�ʼ۸��϶���ͣ����ʱ�ɽ����϶���������
		//3�����̼��Ͼ��ۣ��ɽ���С������
		//4��β�̼��Ͼ��ۣ��ɽ�С�����һ���۸�

		if ((q.bidPrice[0] > superMinPoint && price <= q.bidPrice[0] && q.offerPrice[0] > superMinPoint && q.time > 92500) ||
			((q.bidPrice[0] < superMinPoint) && (price <= lstprice) && ((price >q.preClose * (1 - 0.105) && price < q.preClose * (1 - 0.095)) || (price >q.preClose * (1 - 0.055) && price < q.preClose * (1 - 0.045)))) ||
			(price > superMinPoint && (q.preClose > price) && (ntradetime <= 92500))
			)
		{
			crtActType = ActiveType::ActiveSel;
		}

		// β�̼��Ͼ��ۣ��Ϻ��г�����Ĺ����򣬵����г�����
		if ((ntradetime >= 150000) && (netsymbol < 600000 || (netsymbol >= 600000 && nDate >= 20180820)))
		{
			if (price >lstprice || ((price < q.preClose * 1.105 && price > q.preClose * 1.095) || (price < q.preClose * 1.055 && price > q.preClose * 1.045)))
				crtActType = ActiveType::ActiveBuy;
			else if (price <lstprice || ((price >q.preClose * (1 - 0.105) && price < q.preClose * (1 - 0.095)) || (price >q.preClose * (1 - 0.055) && price < q.preClose * (1 - 0.045))))
				crtActType = ActiveType::ActiveSel;
		}

		//3���ɽ��۸�����һ��һ֮�䣬��ί����ſ����Ϊ��������
		//3���ɽ��۸�����һ��һ֮�䣬��ί����ſ�ǰ��Ϊ��������
		if (crtActType == ActiveType::ActiveUnk)
		{
			if (bidSeq > offerSeq)
				crtActType = ActiveType::ActiveBuy;
			else
				crtActType = ActiveType::ActiveSel;
		}

		if (crtActType == ActiveType::ActiveBuy)
		{
			//����ί����룬�������������룻����һ����������һ�۹ҵ���������һ����Ȼ�����ϱ����ɽ�
			if (lstBidSeq == bidSeq && lstActType == crtActType)
			{
				if (lstprice < price)
					bBreaked = true;
			}
			else
			{
				bBreaked = false;
			}
			extTrans.bIsCrossPrice = bBreaked;
			extTrans.bIsActiveBuy = true;
		}
		else
		{
			if (lstOfferSeq == offerSeq && lstActType == crtActType)
			{
				if (lstprice > price)
					bBreaked = true;
			}
			else
			{
				bBreaked = false;
			}
			extTrans.bIsCrossPrice = bBreaked;
			extTrans.bIsActiveBuy = false;
		}
		return crtActType;

	}


	bool clsTransReduct::FactorToFile(Market *mkt, int tradeDate, std::vector<Symbol>& symbollist, PerBuckValues &values)
	{
		// <symbol,<value>>
		std::unordered_map<int, std::vector<double>> results;

		string title = "symbol";
		for (int j = 0; j < this->Buck_DIV_KIND.size(); j++)
			for (int i = 0; i < this->DIV_SIZE; ++i)
				title += "," + Buck_DIV_KIND[j] + "_" + this->Buck_DIV_NAME[i];

		for (int stk : symbollist)
		{
			results[stk].insert(results[stk].end(), values.ActiveBuyAmtVec[stk].begin(), values.ActiveBuyAmtVec[stk].end());
			results[stk].insert(results[stk].end(), values.ActiveSelAmtVec[stk].begin(), values.ActiveSelAmtVec[stk].end());
		}
		mkt->FactorToFile(_factorSaveName, tradeDate, title, results);

		results.clear();
		for (auto iter = results.begin(); iter != results.end(); ++iter)
		{
			std::vector<double>().swap(iter->second);
		}
		results.clear();

		return true;

	}

}
