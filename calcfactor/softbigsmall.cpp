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

	PerBuckValues::PerBuckValues(const std::vector<StockOrder> &stkVec,int DIV_SIZE)
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
		ActiveBuyAmtVec.clear();
	}

	SoftbigAmall::SoftbigAmall(int UpLimit,int nSpan )
	{
		this->DIV_SIZE = ceil(UpLimit / nSpan * 1.0) + 1;

		this->Buck_DIV_ARR.reserve(this->DIV_SIZE);
		this->Buck_DIV_NAME.reserve(this->DIV_SIZE);
		for (int i = 0; i < UpLimit;)
		{
			this->Buck_DIV_ARR.emplace_back(i*10000);
			char* name = new char[16];
			sprintf_s(name, 16,"%u_%u", i, min(i + nSpan,UpLimit));
			this->Buck_DIV_NAME.emplace_back(name);
			i += nSpan;
			delete[] name;
		}
		this->Buck_DIV_ARR.push_back(UpLimit);

		char* name = new char[16];
		sprintf_s(name, 16,"%u_Inf",  UpLimit);
		this->Buck_DIV_NAME.push_back(name);
		delete[] name;

		this->Buck_DIV_KIND.push_back("ActiveBuy");
		this->Buck_DIV_KIND.push_back("ActiveSel");

	}

	SoftbigAmall::~SoftbigAmall()
	{
		vector<double>().swap(this->Buck_DIV_ARR);
		vector<string>().swap(this->Buck_DIV_NAME);
		if (pvalue)
			delete pvalue;
	}
	void SoftbigAmall::intiStatDataSet(const std::vector<StockOrder> &stkVec, int div_size)
	{
		pvalue = new PerBuckValues(stkVec, div_size);
	}

	void SoftbigAmall::OnBar(Market *mkt, int tradeDate, std::vector<StockOrder> &stkVec)
	{

		std::vector<Symbol> symbollist;
		symbollist.reserve(stkVec.size());

		this->intiStatDataSet(stkVec, this->DIV_SIZE);

		PerBuckValues& values = *pvalue;

		for (auto &stk : stkVec)
		{
			Symbol netsymbol = stk.GetSymbol();
			symbollist.emplace_back(netsymbol);
			const vector<Trans> &transList = stk.GetTrans(); // market->GetTransList(symbol);
			const vector<Quote> &quoteList = stk.GetQuote(); // market->GetTransList(symbol);

			OrderSeq lstBidSeq = 0, lstOfferSeq = 0;
			double lstbidAmt = 0.0, lstofferAmt = 0.0;
			double lstbidVol = 0.0, lstofferVol = 0.0;
			ActiveType lstActType = ActiveType::ActiveUnk;
			ActiveType crtActType = ActiveType::ActiveUnk;


			int bidDivSeq = -1;
			int offerDivSeq = -1;

			double price = 0.0;
			double lstprice = 0.0;
			double volumn = 0.0;
			double amount = 0.0;

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

				price = double(transInfo->price) / 100;
				volumn = transInfo->num;
				amount = price * volumn;

				bidDivSeq = -1;
				offerDivSeq = -1;


				if (transInfo->type == 'F' || transInfo->type == 'N')
				{ // transact
					//��������ĳ�������Ӧ��Ҳ���Ǳ�������
					//1������۸���ڵ�����һ�ʿ��յ���һ��
					//2����һ��Ϊ�գ��Ƿ���[0.098,0.102],[0.048,0.052]����һ�γɽ��۸���ڵ�����һ�ʼ۸��϶���ͣ����ʱ�ɽ����϶���������
					//3�����̼��Ͼ��ۣ��ɽ��۴�������
					//4��β�̼��Ͼ��ۣ��ɽ���������һ���۸�


					if ((q.offerPrice[0] > superMinPoint && price >= q.offerPrice[0]) ||
						((q.offerPrice[0] < superMinPoint) && (price >= lstprice) && ((price >q.preClose * 1.102 && price < q.preClose * 1.098) || (price >q.preClose * 1.052 && price < q.preClose * 1.048))) ||
						((q.preClose < price) && (ntradetime <= 92500))
						)
					{
						crtActType = ActiveType::ActiveBuy;
					}

					//���������ĳ�������Ӧ��Ҳ���Ǳ�������
					//1�������۸�С�ڵ�����һ�ʿ��յ���һ��
					//2����һ��Ϊ�գ��Ƿ���[-0.098,-0.102],[-0.048,-0.052]����һ�γɽ��۸�С�ڵ�����һ�ʼ۸��϶���ͣ����ʱ�ɽ����϶���������
					//3�����̼��Ͼ��ۣ��ɽ��۴�������
					//4��β�̼��Ͼ��ۣ��ɽ���������һ���۸�
					
					if ((q.bidPrice[0] > superMinPoint && price <= q.bidPrice[0]) ||
						((q.offerPrice[0] < superMinPoint) && (price >= lstprice) && ((price >q.preClose * (1-0.102) && price < q.preClose * (1-0.098)) || (price >q.preClose * (1 - 0.102) && price < q.preClose * (1 - 0.048)))) ||
						(price > superMinPoint && (q.preClose > price) && (ntradetime <= 92500))
						)
					{
						crtActType = ActiveType::ActiveSel;
					}

					if ((ntradetime >= 150000) && (netsymbol < 600000 || (netsymbol >= 600000 && nDate >= 20180820)))
					{
						if (price >lstprice)
							crtActType = ActiveType::ActiveBuy;
						else if (price <lstprice)
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


					if (crtActType == ActiveType::ActiveBuy) //����һ�ʿ��յ���һ�����жϱ�׼
					{
						if (lstBidSeq == bidSeq)
						{
							//��ί�е������ͬ��������ί�е���Ŵ�����ί����ţ��϶���һ�������������뵥
							lstbidAmt += amount;
							lstbidVol += volumn;
						}
						else
						{
							//��ʼ��һ������������һ������������
							if (lstbidAmt > 0)
							{
								bidDivSeq = GetAmountInterval(lstbidAmt);
								if (bidDivSeq < DIV_SIZE)
								{
									values.ActiveBuyAmtVec[netsymbol].at(bidDivSeq) += lstbidAmt;
								}
							}
							else if (lstofferAmt > 0)
							{
								offerDivSeq = GetAmountInterval(lstofferAmt);
								if (offerDivSeq < DIV_SIZE)
								{
									values.ActiveSelAmtVec[netsymbol].at(offerDivSeq) += lstofferAmt;
								}
							}
							lstbidAmt = amount;
							lstbidVol = volumn;
						}
						crtActType = ActiveType::ActiveBuy;
						lstofferAmt = 0.0;
						lstofferVol = 0.0;
					}
					else
					{
						if (lstOfferSeq == offerSeq)
						{
							//��ί�е������ͬ��������ί�е���Ŵ�����ί����ţ��϶���һ���������������������������Ƿ��������
							lstofferAmt += amount;
							lstofferVol += volumn;
						}
						else
						{
							//��ʼ��һ������������һ������������
							if (lstbidAmt > 0)
							{
								bidDivSeq = GetAmountInterval(lstbidAmt);
								if (bidDivSeq < DIV_SIZE)
								{
									values.ActiveBuyAmtVec[netsymbol].at(bidDivSeq) += lstbidAmt;
								}
							}
							else if (lstofferAmt > 0)
							{
								offerDivSeq = GetAmountInterval(lstofferAmt);
								if (offerDivSeq < DIV_SIZE)
								{
									values.ActiveSelAmtVec[netsymbol].at(offerDivSeq) += lstofferAmt;
								}
							}
							lstofferAmt = amount;
							lstofferVol = volumn;
						}
						crtActType = ActiveType::ActiveSel;
						lstbidAmt = 0.0;
						lstbidVol = 0.0;
					}
					lstBidSeq = bidSeq;
					lstOfferSeq = offerSeq;
					if(ntradetime < 145700 || (netsymbol > 600000 && tradeDate < 20180820))
						lstprice = price;
				}
			}
		}

		FactorToFile(mkt, tradeDate, symbollist, values);
		//ToSQLite(mkt, tradeDate, values);
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

		return true;

	}

}
