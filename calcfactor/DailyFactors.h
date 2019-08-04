#pragma once
#include "market.h"
#include "public.h"
#include <list>
#include <string>
#include <vector>
#include "public.h"
using std::list;
using std::string;
using std::vector;
namespace l2 {

struct DailyFactorValues
{
       // 因子名设置--order_ 表示基于委托序号；trader_ 表示基于成交序号。
    std::map<Symbol, std::vector< double>>  order_buyVolVec; // 1.bin_index, 2.symbol_index, 3.value
     std::map<Symbol, std::vector< double>> order_sellVolVec;
    std::map<Symbol, std::vector< double>> trader_VolVec;
     std::map<Symbol, std::vector< double>> order_buyAmountVec;
    std::map<Symbol, std::vector< double>> order_sellAmountVec;
     std::map<Symbol, std::vector< double>>  trader_AmountVec;
     std::map<Symbol, std::vector< double>> order_buyFreqVec;
    std::map<Symbol, std::vector< double>> order_sellFreqVec;
     std::map<Symbol, std::vector< double>>  trader_FreqVec;

    DailyFactorValues(const std::vector<StockOrder> &stkVec);
};
class DailyFactors: public KBarHandler
{
  private:




public:
    DailyFactors();
    //
    void OnBar(Market* mkt, int tradeDate, std::vector<StockOrder> &stkVec) override;

};

}
