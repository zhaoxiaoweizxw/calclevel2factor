#include "BigSmall.h"
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
static const double DIV_ARR[] = {0, 50000, 100000, 200000, 500000, 1000000, 2000000, 5000000, 10000000};
static const char *DIV_NAME[] = {"0_5",     "5_10",    "10_20",    "20_50",   "50_100",
                                 "100_200", "200_500", "500_1000", "1000_INF"};
static const unsigned int DIV_SIZE = sizeof(DIV_ARR) / sizeof(DIV_ARR[0]);

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

BigSmallValues::BigSmallValues(const std::vector<StockOrder> &stkVec)
{
    for (auto const &stk : stkVec)
    {
        buyVolVec[stk.GetSymbol()].resize(DIV_SIZE);
        sellVolVec[stk.GetSymbol()].resize(DIV_SIZE);
        quitsVolVec[stk.GetSymbol()].resize(DIV_SIZE);
        buyAmountVec[stk.GetSymbol()].resize(DIV_SIZE);
        sellAmountVec[stk.GetSymbol()].resize(DIV_SIZE);
        quitsAmountVec[stk.GetSymbol()].resize(DIV_SIZE);
    }

    for (auto const &stk : stkVec)
    {
        for (size_t d = 0; d < DIV_SIZE; d++)
        {
            buyVolVec[stk.GetSymbol()].at(d) = 0.0;
            sellVolVec[stk.GetSymbol()].at(d) = 0.0;
            quitsVolVec[stk.GetSymbol()].at(d) = 0.0;
            buyAmountVec[stk.GetSymbol()].at(d) = 0.0;
            sellAmountVec[stk.GetSymbol()].at(d) = 0.0;
            quitsAmountVec[stk.GetSymbol()].at(d) = 0.0;
        }
    }
}
BigSmall::BigSmall() {}

void BigSmall::OnBar(Market *mkt, int tradeDate, std::vector<StockOrder> &stkVec)
{
    BigSmallValues values(stkVec);
	std::vector<Symbol> symbollist;
	symbollist.reserve(stkVec.size());
    for (auto &stk : stkVec)
    {
		symbollist.emplace_back(stk.GetSymbol());
        const vector<Trans> &transList = stk.GetTrans(); // market->GetTransList(symbol);
        for (auto const &trans : transList)
        {
            const Trans *transInfo = &trans;
            Order bidOrder;
            bool ok = stk.GetOrder(transInfo->bidSeq, bidOrder);
            if (!ok)
            {
                continue;
            }
            Order offerOrder;
            ok = stk.GetOrder(transInfo->offerSeq, offerOrder);
            if (!ok)
            {
                continue;
            }

            if (transInfo->type == 'F' && bidOrder.IsValid() && offerOrder.IsValid())
            { // transact
                const Order *bidOrderInfo = bidOrder.GetInfo();
                const Order *offerOrderInfo = offerOrder.GetInfo();
                double price = double(transInfo->price) / 100;
                double volumn = transInfo->num;
                double amount = price * volumn;
                double bidPrice = double(bidOrderInfo->price) / 100;
                double offerPrice = double(offerOrderInfo->price) / 100;
                double bidVol = bidOrderInfo->num + bidOrderInfo->transNum + bidOrderInfo->cancelNum;
                double offerVol = offerOrderInfo->num + offerOrderInfo->transNum + offerOrderInfo->cancelNum;
                double bidAmount = bidPrice * bidVol;
                double offerAmount = offerPrice * offerVol;
                unsigned int bidDivSeq = GetAmountInterval(bidAmount);
                unsigned int offerDivSeq = GetAmountInterval(offerAmount);

                // if (100 * hour + minute <= 100 * cutOffHour + cutOffMinute)
                {
                    if (bidDivSeq < DIV_SIZE)
                    {
                        values.buyVolVec[stk.GetSymbol()].at(bidDivSeq) += volumn;
                        values.buyAmountVec[stk.GetSymbol()].at(bidDivSeq) += amount;
                    }
                    if (offerDivSeq < DIV_SIZE)
                    {
                        values.sellVolVec[stk.GetSymbol()].at(offerDivSeq) += volumn;
                        values.sellAmountVec[stk.GetSymbol()].at(offerDivSeq) += amount;
                    }
                    if (bidDivSeq == offerDivSeq && bidDivSeq < DIV_SIZE)
                    {
                        values.quitsVolVec[stk.GetSymbol()].at(bidDivSeq) += volumn;
                        values.quitsAmountVec[stk.GetSymbol()].at(bidDivSeq) += amount;
                    }
                }
            }
        }
    }

	FactorToFile(mkt, tradeDate, symbollist,values);
    //ToSQLite(mkt, tradeDate, values);
}

void BigSmall::SetFactorSaveName(const char* strName)
{
	_factorSaveName = std::string(strName);
}
bool BigSmall::FactorToFile(Market *mkt,int tradeDate, std::vector<Symbol>& symbollist, BigSmallValues &values)
{
	// <symbol,<value>>
	std::unordered_map<int, std::vector<double>> results;

	string title = "symbol";
	const char *Factor_Name[] = {"buyVol","sellVol","quitsVol","buyAmt","sellAmt","quitsAmt" };

	int nFactors = sizeof(Factor_Name) / sizeof(Factor_Name[0]);

	for (int j = 0; j < nFactors; j++)
	{
		for (size_t i = 0; i < DIV_SIZE; i++)
			title += "," + std::string(Factor_Name[j]) + "_" + std::string(DIV_NAME[i]);
	}
	for (int stk : symbollist)
	{
		results[stk].insert(results[stk].end(),values.buyVolVec[stk].begin(), values.buyVolVec[stk].end());
		results[stk].insert(results[stk].end(), values.sellVolVec[stk].begin(), values.sellVolVec[stk].end());
		results[stk].insert(results[stk].end(), values.quitsVolVec[stk].begin(), values.quitsVolVec[stk].end());
		results[stk].insert(results[stk].end(), values.buyAmountVec[stk].begin(), values.buyAmountVec[stk].end());
		results[stk].insert(results[stk].end(), values.sellAmountVec[stk].begin(), values.sellAmountVec[stk].end());
		results[stk].insert(results[stk].end(), values.quitsAmountVec[stk].begin(), values.quitsAmountVec[stk].end());
	}
	mkt->FactorToFile(_factorSaveName, tradeDate, title, results);

	results.clear();

	return true;

}
bool BigSmall::ToSQLite(Market *mkt, int tradeDate, BigSmallValues &values)
{

    // <factorName, <symbol,<<tradeDate,value>>>>
    std::unordered_map<std::string, std::map<std::string, std::vector<std::pair<int, double>>>> results;
    // results[GetSymbolStr(v.first)].emplace_back(std::make_pair(tradeDate, values));
    for (auto const &v : values.buyVolVec)
    {
        size_t sz = v.second.size();
        for (size_t i = 0; i < sz; i++)
        {
            // LOG_DEBUG << "DIV " << i << "|" << it->first << "|" << it->second << std::endl;

            results["BUY_AMOUNT_" + std::string(DIV_NAME[i])][mkt->GetSymbolStr(v.first)].emplace_back(
                std::make_pair(tradeDate, values.buyAmountVec[v.first].at(i)));
            results["BUY_VOL_" + std::string(DIV_NAME[i])][mkt->GetSymbolStr(v.first)].emplace_back(
                std::make_pair(tradeDate, values.buyVolVec[v.first].at(i)));

            results["SELL_AMOUNT_" + std::string(DIV_NAME[i])][mkt->GetSymbolStr(v.first)].emplace_back(
                std::make_pair(tradeDate, values.sellAmountVec[v.first].at(i)));
            results["SELL_VOL_" + std::string(DIV_NAME[i])][mkt->GetSymbolStr(v.first)].emplace_back(
                std::make_pair(tradeDate, values.sellVolVec[v.first].at(i)));

            results["QUITS_AMOUNT_" + std::string(DIV_NAME[i])][mkt->GetSymbolStr(v.first)].emplace_back(
                std::make_pair(tradeDate, values.quitsAmountVec[v.first].at(i)));
            results["QUITS_VOL_" + std::string(DIV_NAME[i])][mkt->GetSymbolStr(v.first)].emplace_back(
                std::make_pair(tradeDate, values.quitsVolVec[v.first].at(i)));
        }
    }
    //mkt->ToSQLite(tradeDate, results);
    return true;
}
}
