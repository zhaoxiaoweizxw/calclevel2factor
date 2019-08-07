#include "earlymoney.h"
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

static const double AMOUNT_LIMIT = 9999999999999999;
static const double DIV_ARR[] = {0, 40000, 200000, 1000000};
static const char *DIV_NAME[] = {"0_4",   "4_20",    "20_100",  "100_INF"};
static const char *DIV_KIND[] = { "ActivBuy",  "ActivSell", "ActivBuyVol",  "ActivSellVol" };
static const unsigned int DIV_SIZE = sizeof(DIV_ARR) / sizeof(DIV_ARR[0]);
static const unsigned int DIV_KIND_SIZE = sizeof(DIV_KIND) / sizeof(DIV_KIND[0]);

/**
ActiveSel为主动买结束
ActiveUnk为判断不出
ActiveBuy为主动卖结束，
**/
enum ActiveType { ActiveSel, ActiveUnk, ActiveBuy};


static unsigned int GetAmountInterval(double amount)
{
	if (amount <= 0)
	{
		return DIV_SIZE;
	}
	for (unsigned int d = 0; d < DIV_SIZE; d++)
	{
		if (amount >= DIV_ARR[d] && amount < DIV_ARR[d + 1])
		{
			return d;
		}
	}
	return DIV_SIZE - 1;
}

ActPasValues::ActPasValues(const std::vector<StockOrder> &stkVec)
{
    for (auto const &stk : stkVec)
    {
		ActivebuyAmtVec[stk.GetSymbol()].resize(DIV_SIZE );
		ActiveselAmtVec[stk.GetSymbol()].resize(DIV_SIZE );
		ActivebuyVolVec[stk.GetSymbol()].resize(DIV_SIZE);
		ActiveselVolVec[stk.GetSymbol()].resize(DIV_SIZE);
		//PasivebuyAmtVec[stk.GetSymbol()].resize(DIV_SIZE);
		//PasiveselAmtVec[stk.GetSymbol()].resize(DIV_SIZE);
    }

    for (auto const &stk : stkVec)
    {
        for (size_t d = 0; d < DIV_SIZE; d++)
        {
			ActivebuyAmtVec[stk.GetSymbol()].at(d) = 0.0;
			ActiveselAmtVec[stk.GetSymbol()].at(d) = 0.0;
			ActivebuyVolVec[stk.GetSymbol()].at(d) = 0.0;
			ActiveselVolVec[stk.GetSymbol()].at(d) = 0.0;
			//PasivebuyAmtVec[stk.GetSymbol()].at(d) = 0.0;
			//PasiveselAmtVec[stk.GetSymbol()].at(d) = 0.0;
        }
    }
}
EarlyMoney::EarlyMoney() {}

void EarlyMoney::OnBar(Market *mkt, int tradeDate, std::vector<StockOrder> &stkVec)
{
	ActPasValues values(stkVec);
	std::vector<Symbol> symbollist;
	symbollist.reserve(stkVec.size());
    for (auto &stk : stkVec)
    {
		symbollist.emplace_back(stk.GetSymbol());
        const vector<Trans> &transList = stk.GetTrans(); // market->GetTransList(symbol);

		OrderSeq lstBidSeq = 0, lstOfferSeq = 0;
		double lstbidAmt = 0.0, lstofferAmt = 0.0;
		double lstbidVol = 0.0, lstofferVol = 0.0;
		ActiveType lstActType = ActiveType::ActiveUnk;
		ActiveType crtActType = ActiveType::ActiveUnk;


		int bidDivSeq = -1;
		int offerDivSeq = -1;

		double price = 0.0;
		double volumn = 0.0;
		double amount = 0.0;

        for (auto const &trans : transList)
        {
            const Trans *transInfo = &trans;
			OrderSeq bidSeq = transInfo->bidSeq;
			OrderSeq offerSeq = transInfo->offerSeq;

			price = double(transInfo->price) / 100;
			volumn = transInfo->num;
			amount = price * volumn;

			bidDivSeq = -1;
			offerDivSeq = -1;
			
            if (transInfo->type == 'F')
            { // transact
				if (bidSeq > offerSeq)
				{
					if (lstBidSeq == bidSeq )
					{
						//买委托单序号相同，并且买委托单序号大于卖委托序号，认定是一个持续主动买入单
						lstbidAmt += amount;
						lstbidVol += volumn;
					}
					else
					{
						//开始了一笔新主动买，上一笔主动单结束
						if (lstbidAmt > 0)
						{
							bidDivSeq = GetAmountInterval(lstbidAmt);
							if (bidDivSeq < DIV_SIZE)
							{
								values.ActivebuyAmtVec[stk.GetSymbol()].at(bidDivSeq) += lstbidAmt;
								values.ActivebuyVolVec[stk.GetSymbol()].at(bidDivSeq) += lstbidVol;
							}
						}
						else if (lstofferAmt > 0)
						{
							offerDivSeq = GetAmountInterval(lstofferAmt);
							if (offerDivSeq < DIV_SIZE)
							{
								values.ActiveselAmtVec[stk.GetSymbol()].at(offerDivSeq) += lstofferAmt;
								values.ActiveselVolVec[stk.GetSymbol()].at(offerDivSeq) += lstofferVol;
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
						//卖委托单序号相同，并且卖委托单序号大于买委托序号，认定是一个持续主动卖出单，主动买入是否结束不明
						lstofferAmt += amount;
						lstofferVol += volumn;
					}
					else
					{
						//开始了一笔新主动买，上一笔主动单结束
						if (lstbidAmt > 0)
						{
							bidDivSeq = GetAmountInterval(lstbidAmt);
							if (bidDivSeq < DIV_SIZE)
							{
								values.ActivebuyAmtVec[stk.GetSymbol()].at(bidDivSeq) += lstbidAmt;
								values.ActivebuyVolVec[stk.GetSymbol()].at(bidDivSeq) += lstbidVol;
							}
						}
						else if (lstofferAmt > 0)
						{
							offerDivSeq = GetAmountInterval(lstofferAmt);
							if (offerDivSeq < DIV_SIZE)
							{
								values.ActiveselAmtVec[stk.GetSymbol()].at(offerDivSeq) += lstofferAmt;
								values.ActiveselVolVec[stk.GetSymbol()].at(offerDivSeq) += lstofferVol;
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
            }
        }
    }

	FactorToFile(mkt, tradeDate, symbollist,values);
    //ToSQLite(mkt, tradeDate, values);
}


bool EarlyMoney::FactorToFile(Market *mkt,int tradeDate, std::vector<Symbol>& symbollist, ActPasValues &values)
{
	// <symbol,<value>>
	std::unordered_map<int, std::vector<double>> results;

	string title = "symbol";
	for (int i = 0; i < DIV_KIND_SIZE; ++i)
		for (int j = 0; j < DIV_SIZE; ++j)
			title += "," + string(DIV_KIND[i]) + "_" + DIV_NAME[j];

	for (int stk : symbollist)
	{
		results[stk].insert(results[stk].end(), values.ActivebuyAmtVec[stk].begin(), values.ActivebuyAmtVec[stk].end());
		results[stk].insert(results[stk].end(), values.ActiveselAmtVec[stk].begin(), values.ActiveselAmtVec[stk].end());
		results[stk].insert(results[stk].end(), values.ActivebuyVolVec[stk].begin(), values.ActivebuyVolVec[stk].end());
		results[stk].insert(results[stk].end(), values.ActiveselVolVec[stk].begin(), values.ActiveselVolVec[stk].end());
	}
	mkt->FactorToFile(_factorSaveName, tradeDate, title, results);

	results.clear();

	return true;

}

}
