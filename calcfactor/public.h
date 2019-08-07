#pragma once
//#include "common/AlgoLog.h"
//#include "utils/TimeUtil.h"
#include <string>
#include <vector>
#include <list>
#include <ctime>
using std::list;
using std::string;
using std::time_t;

namespace l2 {

typedef unsigned long long OrderSeq;
typedef unsigned long long TransSeq;
typedef int Symbol;
typedef unsigned int Price;
extern const OrderSeq MAX_ORDER_SEQ;
extern const OrderSeq INVALID_ORDER_SEQ;
extern const TransSeq MAX_TRANS_SEQ;
extern const TransSeq INVALID_TRANS_SEQ;
extern const Symbol INVALID_SYMBOL;
extern const Symbol MAX_SYMBOL;
extern const Price INVALID_PRICE;
extern const time_t INVALID_TIME;
extern const time_t MIN_TIME;
extern const float PRICE_SCOPE;
extern const int64_t LAUNCH_TIME;
extern const string LAUNCH_TIME_STR;
extern const unsigned int DAY_SECONDS;
extern const unsigned int HOUR_SECONDS;
extern const unsigned int MINUTE_SECONDS;


class StockOrder;
class Market;
class KBarHandler
{
public:
	// @param factorName	因子的系列名称，以此建立文件夹
	std::string _factorSaveName;
public:
	void SetFactorSaveName(const char* strName);
    virtual void OnBar(Market* mkt, int tradeDate,std::vector< StockOrder>& stk) = 0;
    virtual ~KBarHandler();
};

}
