#include "DailyFactors.h"
#include "../utils/utils_common.h"
#include <algorithm>
#include <cmath>
#include <iostream>
using std::cout;
using std::endl;
using std::fixed;
using std::max;
using std::min;
using std::range_error;
namespace l2
{

static const double AMOUNT_LIMIT = 9999999999999999;

static const double DIV_ARR[] = {0,      5000,   10000,   20000,   50000,   100000,
                                 200000, 500000, 1000000, 2000000, 5000000, 10000000};

static const char *DIV_NAME[] = {"0_5Q",  "5Q_1",   "1_2",     "2_5",     "5_10",     "10_20",
                                 "20_50", "50_100", "100_200", "200_500", "500_1000", "1000_INF"};

// const unsigned int DIV_SIZE = length(DIV_ARR)-1;
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

DailyFactorValues::DailyFactorValues(const std::vector<StockOrder> &stkVec)
{

    for (auto const &stk : stkVec)
    {

        order_buyVolVec[stk.GetSymbol()].resize(DIV_SIZE);
        order_sellVolVec[stk.GetSymbol()].resize(DIV_SIZE);
        trader_VolVec[stk.GetSymbol()].resize(DIV_SIZE);
        order_buyAmountVec[stk.GetSymbol()].resize(DIV_SIZE);
        order_sellAmountVec[stk.GetSymbol()].resize(DIV_SIZE);
        trader_AmountVec[stk.GetSymbol()].resize(DIV_SIZE);
        order_buyFreqVec[stk.GetSymbol()].resize(DIV_SIZE);
        order_sellFreqVec[stk.GetSymbol()].resize(DIV_SIZE);
        trader_FreqVec[stk.GetSymbol()].resize(DIV_SIZE);
    }

    for (auto const &stk : stkVec)
    {

        for (size_t d = 0; d < DIV_SIZE; d++)
        {
            order_buyVolVec[stk.GetSymbol()].at(d) = 0.0;
            order_sellVolVec[stk.GetSymbol()].at(d) = 0.0;
            trader_VolVec[stk.GetSymbol()].at(d) = 0.0;
            order_buyAmountVec[stk.GetSymbol()].at(d) = 0.0;
            order_sellAmountVec[stk.GetSymbol()].at(d) = 0.0;
            trader_AmountVec[stk.GetSymbol()].at(d) = 0.0;
            order_buyFreqVec[stk.GetSymbol()].at(d) = 0.0;
            order_sellFreqVec[stk.GetSymbol()].at(d) = 0.0;
            trader_FreqVec[stk.GetSymbol()].at(d) = 0.0;
        }
    }
}
DailyFactors::DailyFactors() {}

void DailyFactors::OnBar(Market *mkt, int tradeDate, std::vector<StockOrder> &stkVec)
{
    DailyFactorValues values(stkVec);

    for (auto &stk : stkVec)
    {

        const vector<Trans> &transList = stk.GetTrans(); // market->GetTransList(symbol);
        const vector<Order> &orderList = stk.GetOrder();
        // LOG_DEBUG << "trans|" << stk.GetSymbolStr() << "|" << transList.size() << "\torder|" << stk.GetSymbolStr()
        //          << "|" << orderList.size() << std::endl;
        for (auto const &order : orderList)
        {
            if (order.IsEmpty() || (!order.IsValid()))
            {
                //LOG_DEBUG << "ignore this order " << order.seq << "|" << order.symbol << std::endl;
				std::cerr << "ignore this order " << order.seq << "|" << order.symbol << std::endl;
                continue;
            }
            const Order *info = &order;
            long long orderVolume = info->num + info->transNum + info->cancelNum;
            double bidAmount;
            double offerAmount;
            if (info->direct == '1')
            {
                if (info->type == '2')
                {
                    bidAmount = orderVolume * info->price / 100;
                }
                else
                {
                    bidAmount = orderVolume * info->transPrice / 100;
                }
                unsigned int bidDivSeq = GetAmountInterval(bidAmount);
                if (bidDivSeq < DIV_SIZE)
                {
                    // if (100 * hour + minute <= 100 * cutOffHour + cutOffMinute)
                    {
                        values.order_buyVolVec[stk.GetSymbol()].at(bidDivSeq) += orderVolume;
                        values.order_buyAmountVec[stk.GetSymbol()].at(bidDivSeq) += bidAmount;
                        values.order_buyFreqVec[stk.GetSymbol()].at(bidDivSeq) += 1.0;
                    }
                }
            }
            else if (info->direct == '2')
            {
                if (info->type == '2')
                {
                    offerAmount = orderVolume * info->price / 100;
                }
                else
                {
                    offerAmount = orderVolume * info->transPrice / 100;
                }
                unsigned int offerDivSeq = GetAmountInterval(offerAmount);
                if (offerDivSeq < DIV_SIZE)
                {
                    values.order_sellVolVec[stk.GetSymbol()].at(offerDivSeq) += orderVolume;
                    values.order_sellAmountVec[stk.GetSymbol()].at(offerDivSeq) += offerAmount;
                    values.order_sellFreqVec[stk.GetSymbol()].at(offerDivSeq) += 1.0;
                }
            }
        }

        for (auto const &trans : transList)

            if (trans.type == 'F')
            { // transact
                double price = double(trans.price) / 100;
                double volumn = trans.num;
                double amount = price * volumn;
                unsigned int traderDivSeq = GetAmountInterval(amount);

                if (traderDivSeq < DIV_SIZE)
                {
                    values.trader_VolVec[stk.GetSymbol()].at(traderDivSeq) += volumn;
                    values.trader_AmountVec[stk.GetSymbol()].at(traderDivSeq) += amount;
                    values.trader_FreqVec[stk.GetSymbol()].at(traderDivSeq) += 1.0;
                }
            }
    }

    //ToSQLite(mkt, tradeDate, values);
}
}
