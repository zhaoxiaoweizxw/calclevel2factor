
#include "softbigsmall.h"
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


	unsigned int SoftbigAmall::GetAmountInterval(double amount)
	{
		if (amount <= 0)
		{
			return DIV_SIZE;
		}
		for (unsigned int d = 0; d < DIV_SIZE-1; d++)
		{
			if (amount >= Buck_DIV_ARR[d] && amount < Buck_DIV_ARR[d + 1])
			{
				return d;
			}
		}
		return DIV_SIZE - 1;
	}
	void PerBuckValues::initBuck(const std::vector<StockOrder> &stkVec,int DIV_SIZE)
	{
		for (auto const &stk : stkVec)
		{
			ActiveBuyAmtVec[stk.GetSymbol()].resize(DIV_SIZE);
			ActiveSelAmtVec[stk.GetSymbol()].resize(DIV_SIZE);
			PassivBuyAmtVec[stk.GetSymbol()].resize(DIV_SIZE);
			PassivSelAmtVec[stk.GetSymbol()].resize(DIV_SIZE);
		}

		for (auto const &stk : stkVec)
		{
			for (size_t d = 0; d < DIV_SIZE; d++)
			{
				ActiveBuyAmtVec[stk.GetSymbol()].at(d) = 0.0;
				ActiveSelAmtVec[stk.GetSymbol()].at(d) = 0.0;
				PassivBuyAmtVec[stk.GetSymbol()].at(d) = 0.0;
				PassivSelAmtVec[stk.GetSymbol()].at(d) = 0.0;
			}
		}
	}

	PerBuckValues::~PerBuckValues()
	{
		for (auto iter = ActiveBuyAmtVec.begin(); iter != ActiveBuyAmtVec.end();++iter)
		{
			std::vector< double>().swap(iter->second);
		}
		for (auto iter = ActiveSelAmtVec.begin(); iter != ActiveSelAmtVec.end(); ++iter)
		{
			std::vector< double>().swap(iter->second);
		}
		for (auto iter = PassivBuyAmtVec.begin(); iter != PassivBuyAmtVec.end(); ++iter)
		{
			std::vector< double>().swap(iter->second);
		}
		for (auto iter = PassivSelAmtVec.begin(); iter != PassivSelAmtVec.end(); ++iter)
		{
			std::vector< double>().swap(iter->second);
		}
		ActiveBuyAmtVec.clear();
		ActiveSelAmtVec.clear();
		PassivBuyAmtVec.clear();
		PassivSelAmtVec.clear();
	}

	SoftbigAmall::SoftbigAmall(int UpLimit,int nSpan )
	{
		vector<int > dnlimitVec;
		for (int i = 0; i < UpLimit;)
		{
			dnlimitVec.push_back(i);
			if (i < 30)
				i += nSpan;
			else
				i += 2 * nSpan;
		}
		this->bCalcBreakPrice = false;
		this->DIV_SIZE = dnlimitVec.size() + 1;

		this->Buck_DIV_ARR.reserve(this->DIV_SIZE);
		this->Buck_DIV_NAME.reserve(this->DIV_SIZE);
		for (int i = 0; i < UpLimit;)
		{
			this->Buck_DIV_ARR.emplace_back(i*10000);
			char* name = new char[16];
			int tmpSpan = (i >= 30) ? 2 * nSpan : nSpan;
			sprintf_s(name, 16,"%u_%u", i, i + tmpSpan);
			this->Buck_DIV_NAME.emplace_back(name);
			i += tmpSpan;
			delete[] name;
		}
		this->Buck_DIV_ARR.push_back(UpLimit);

		char* name = new char[16];
		sprintf_s(name, 16,"%u_Inf", dnlimitVec.back());
		this->Buck_DIV_NAME.push_back(name);
		delete[] name;

		this->Buck_DIV_KIND.push_back("ActiveBuy");
		this->Buck_DIV_KIND.push_back("ActiveSel");

	}

	SoftbigAmall::~SoftbigAmall()
	{
		vector<double>().swap(this->Buck_DIV_ARR);
		vector<string>().swap(this->Buck_DIV_NAME);
		vector<string>().swap(this->Buck_DIV_KIND);

	}

	void SoftbigAmall::setBreak(bool bBreak)
	{
		this->bCalcBreakPrice = bBreak;
	}
	void SoftbigAmall::OnBar(Market *mkt, int tradeDate, std::vector<StockOrder> &stkVec)
	{

		std::vector<Symbol> symbollist;
		symbollist.reserve(stkVec.size());

		PerBuckValues values;
		values.initBuck(stkVec, this->DIV_SIZE);


		for (auto &stk : stkVec)
		{
			Symbol netsymbol = stk.GetSymbol();
			symbollist.emplace_back(netsymbol);
			const vector<Trans> &transList = stk.GetTrans(); // market->GetTransList(symbol);
			const vector<Quote> &quoteList = stk.GetQuote(); // market->GetTransList(symbol);

			OrderSeq lstBidSeq = 0, lstOfferSeq = 0;
			long long lstbidAmt = 0, lstofferAmt = 0;
			long lstbidVol = 0, lstofferVol = 0;
			ActiveType lstActType = ActiveType::ActiveUnk;
			ActiveType crtActType = ActiveType::ActiveUnk;



			Price price = 0;
			Price lstprice = 0.0;
			long volumn = 0;
			long long amount = 0;

			bool  bBreaked = false;

			for (auto const &trans : transList)
			{
				const Trans *transInfo = &trans;
				int ntradetime = transInfo->time;
				int nDate = transInfo->date;

				crtActType = ActiveType::ActiveUnk;


				Quote q;
				if (! stk.GetPreTimeQuote(ntradetime,q))
				{
					std::cerr << " Get quote error!!" << tradeDate << " " << netsymbol << " "<<ntradetime << endl;
					continue;
				}

				OrderSeq bidSeq = transInfo->bidSeq;
				OrderSeq offerSeq = transInfo->offerSeq;

				price = transInfo->price;
				volumn = transInfo->num;
				amount = price * volumn ;


				if (transInfo->type == 'F' || transInfo->type == 'N')
				{ // transact
					//��������ĳ�������Ӧ��Ҳ���Ǳ�������
					//1������۸���ڵ�����һ�ʿ��յ���һ��
					//2����һ��Ϊ�գ��Ƿ���[0.098,0.102],[0.048,0.052]����һ�γɽ��۸���ڵ�����һ�ʼ۸��϶���ͣ����ʱ�ɽ����϶���������
					//3�����̼��Ͼ��ۣ��ɽ��۴�������
					//4��β�̼��Ͼ��ۣ��ɽ��������һ���۸�


					if ((q.offerPrice[0] > superMinPoint && price >= q.offerPrice[0] && q.bidPrice[0] > superMinPoint && q.time >= 92500) ||
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
					
					if ((q.bidPrice[0] > superMinPoint && price <= q.bidPrice[0] && q.offerPrice[0] > superMinPoint && q.time >= 92500) ||
						((q.bidPrice[0] < superMinPoint) && (price <= lstprice) && ((price >q.preClose * (1-0.105) && price < q.preClose * (1-0.095)) || (price >q.preClose * (1 - 0.055) && price < q.preClose * (1 - 0.045)))) ||
						(price > superMinPoint && (q.preClose > price) && (ntradetime <= 92500))
						)
					{
						crtActType = ActiveType::ActiveSel;
					}

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
						if (bidSeq > offerSeq )
							crtActType = ActiveType::ActiveBuy;
						else
							crtActType = ActiveType::ActiveSel;
					}

					//std::cout << transInfo->seq <<"\t" << crtActType << endl;
					if (crtActType == ActiveType::ActiveBuy) //����һ�ʿ��յ���һ�����жϱ�׼
					{
						if (lstBidSeq == bidSeq && lstActType == crtActType)
						{
							//��ί�е������ͬ��������ί�е���Ŵ�����ί����ţ��϶���һ�������������뵥
							lstbidAmt += amount;
							lstbidVol += volumn;

							//ֻ��Խ�۵�
							if (bCalcBreakPrice && lstprice < price)
							{
								bBreaked = true;
								lstbidAmt = amount;
								lstbidVol = volumn;
							}
						}
						else
						{
							//��ʼ��һ������������һ������������
							if(!bCalcBreakPrice || bBreaked)
								insertToBuck(netsymbol, lstbidAmt, lstofferAmt, values);
							lstbidAmt = amount;
							lstbidVol = volumn;
							bBreaked = false;
						}
						lstofferAmt = 0;
						lstofferVol = 0;
					}
					else
					{
						if (lstOfferSeq == offerSeq && lstActType == crtActType)
						{
							//��ί�е������ͬ��������ί�е���Ŵ�����ί����ţ��϶���һ���������������������������Ƿ��������
							lstofferAmt += amount;
							lstofferVol += volumn;
							//ֻ��Խ�۵�
							if (bCalcBreakPrice && lstprice > price)
							{
								bBreaked = true;
								lstofferAmt = amount;
								lstofferVol = volumn;
							}
						}
						else
						{
							//��ʼ��һ������������һ������������
							if (!bCalcBreakPrice || bBreaked)
								insertToBuck(netsymbol, lstbidAmt, lstofferAmt, values);
							lstofferAmt = amount;
							lstofferVol = volumn;
							bBreaked = false;
						}
						lstbidAmt = 0;
						lstbidVol = 0;
					}
					lstBidSeq = bidSeq;
					lstOfferSeq = offerSeq;
					if(ntradetime < 145700 || (netsymbol > 600000 && tradeDate < 20180820))
						lstprice = price;
					lstActType = crtActType;
				}
			}
			
			insertToBuck(netsymbol, lstbidAmt, lstofferAmt, values);
		}

		FactorToFile(mkt, tradeDate, symbollist, values);
		//ToSQLite(mkt, tradeDate, values);
	}

	void SoftbigAmall::insertToBuck(Symbol netsymbol, long lstbidAmt, long lstofferAmt,PerBuckValues& values)
	{

		int bidDivSeq = -1;
		int offerDivSeq = -1;

		if (lstbidAmt > superMinPoint)
		{
			bidDivSeq = GetAmountInterval(lstbidAmt / 100.0);
			if (bidDivSeq < DIV_SIZE)
			{
				values.ActiveBuyAmtVec[netsymbol].at(bidDivSeq) += lstbidAmt/100.0;
			}
		}
		else if (lstofferAmt > superMinPoint)
		{
			offerDivSeq = GetAmountInterval(lstofferAmt / 100.0);
			if (offerDivSeq < DIV_SIZE)
			{
				values.ActiveSelAmtVec[netsymbol].at(offerDivSeq) += lstofferAmt /100.0;
			}
		}
	}

	bool SoftbigAmall::FactorToFile(Market *mkt, int tradeDate, std::vector<Symbol>& symbollist, PerBuckValues &values)
	{
		// <symbol,<value>>
		std::unordered_map<int, std::vector<double>> results;

		string title = "symbol";
		for (int j = 0; j < this->Buck_DIV_KIND.size();j++)
			for (int i = 0; i < this->DIV_SIZE; ++i)
					title += "," + Buck_DIV_KIND[j]+ "_" + this->Buck_DIV_NAME[i];

		for (int stk : symbollist)
		{
			results[stk].insert(results[stk].end(), values.ActiveBuyAmtVec[stk].begin(), values.ActiveBuyAmtVec[stk].end());
			results[stk].insert(results[stk].end(), values.ActiveSelAmtVec[stk].begin(), values.ActiveSelAmtVec[stk].end());
		}
		mkt->FactorToFile(_factorSaveName, tradeDate, title, results);

		results.clear();
		for (auto iter = results.begin(); iter != results.end();++iter)
		{
			std::vector<double>().swap(iter->second);
		}
		results.clear();

		return true;

	}

}
