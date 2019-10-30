#pragma once
#include "datacheck.h"
#include "../utils/utils_common.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <numeric>
using std::cout;
using std::endl;
using std::fixed;
using std::max;
using std::min;
using std::range_error;
namespace l2
{
	static const char *FACTOR_NAME[] = { "transVolSum"};
	static const unsigned int FACTOR_SIZE = sizeof(FACTOR_NAME) / sizeof(FACTOR_NAME[0]);

	DataCheckValues::DataCheckValues(const std::vector<StockOrder> &stkVec)
	{
		for (auto const &stk : stkVec)
		{
			transVolSumVec[stk.GetSymbol()].resize(1);
		}

		for (auto const &stk : stkVec)
		{
			for (size_t d = 0; d < 1; d++)
			{
				transVolSumVec[stk.GetSymbol()].at(d) = 0.0;
			}
		}
	}

	dataCheckL2::dataCheckL2() {}



	void dataCheckL2::OnBar(Market *mkt, int tradeDate, std::vector<StockOrder> &stkVec)
	{

		DataCheckValues values(stkVec);
		std::vector<Symbol> symbollist;
		symbollist.reserve(stkVec.size());


		for (auto &stk : stkVec)
		{
			Symbol curSymbol = stk.GetSymbol();
			symbollist.emplace_back(curSymbol);
			const vector<Trans> &transList = stk.GetTrans(); // market->GetTransList(symbol);

			int nVolday = accumulate(transList.begin(), transList.end(), 0, [](int a, Trans b) {return a + (b.type == '4'?0:b.num*b.price/100.0); });
			values.transVolSumVec[curSymbol][0] = nVolday;

		}
		FactorToFile(mkt, tradeDate, symbollist, values);
	}

	bool dataCheckL2::FactorToFile(Market *mkt, int tradeDate, std::vector<Symbol>& symbollist, DataCheckValues &values)
	{
		// <symbol,<value>>
		std::unordered_map<int, std::vector<double>> results;

		string title = "symbol";
		for (int i = 0; i < FACTOR_SIZE; ++i)
			title += "," + string(FACTOR_NAME[i]);

		for (int stk : symbollist)
		{
			results[stk].insert(results[stk].end(), values.transVolSumVec[stk].begin(), values.transVolSumVec[stk].end());
		}
		mkt->FactorToFile(_factorSaveName, tradeDate, title, results);

		results.clear();

		return true;

	}
}