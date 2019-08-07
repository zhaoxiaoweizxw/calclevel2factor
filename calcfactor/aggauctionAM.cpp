#pragma once
#include "aggauctionAM.h"
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
	static const double NaNValue = 0.000001;
	static const double MaxValue = 99999999;

	static const char *FACTOR_NAME[] = { "crsDayPct",   "stage1Pct",    "stage2Pct",  "aggAucAmAmout" ,"stage1High","stage1Low","stepUpDownVec","Step2Num","highlimit","lowlimit"};
	static const unsigned int FACTOR_SIZE = sizeof(FACTOR_NAME) / sizeof(FACTOR_NAME[0]);

	double calcCorr(vector<double> &A, vector<double> &B, long Length) {
		double sumA(0.0), sumB(0.0), aveA(0.0), aveB(0.0);

		//求和
		sumA = std::accumulate(A.begin(), A.end(), 0.0);
		sumB = std::accumulate(B.begin(), B.end(), 0.0);

		//求平均值
		aveA = sumA / double(Length);
		aveB = sumB / double(Length);

		//计算相关系数
		double R1(0), R2(0), R3(0);
		for (long i = 0; i < Length; i++)
		{
			R1 += (A[i] - aveA) * (B[i] - aveB);
			R2 += pow((A[i] - aveA), 2);
			R3 += pow((B[i] - aveB), 2);
		}
		if (R3 < NaNValue)
			return 0.0;

		return (R1 / sqrt(R2*R3));
	}
	
	AggAuctionValues::AggAuctionValues(const std::vector<StockOrder> &stkVec)
	{
		for (auto const &stk : stkVec)
		{
			crsDayPctVec[stk.GetSymbol()].resize(1);
			stage1PctVec[stk.GetSymbol()].resize(1);
			stage2PctVec[stk.GetSymbol()].resize(1);
			aggAucAmAmoutVec[stk.GetSymbol()].resize(1);
			stage1HighVec[stk.GetSymbol()].resize(1);
			stage1LowVec[stk.GetSymbol()].resize(1);
			stepUpDownVec[stk.GetSymbol()].resize(1);
			step2DnumVec[stk.GetSymbol()].resize(1);
			highLimitVec[stk.GetSymbol()].resize(1);
			lowLimitVec[stk.GetSymbol()].resize(1);
		}

		for (auto const &stk : stkVec)
		{
			for (size_t d = 0; d < 1; d++)
			{
				crsDayPctVec[stk.GetSymbol()].at(d) = 0.0;
				stage1PctVec[stk.GetSymbol()].at(d) = 0.0;
				stage2PctVec[stk.GetSymbol()].at(d) = 0.0;
				aggAucAmAmoutVec[stk.GetSymbol()].at(d) = 0.0;
				stage1HighVec[stk.GetSymbol()].at(d) = 0.0;
				stage1LowVec[stk.GetSymbol()].at(d) = 0.0;
				stepUpDownVec[stk.GetSymbol()].at(d) = 0.0;
				step2DnumVec[stk.GetSymbol()].at(d) = 0.0;
				highLimitVec[stk.GetSymbol()].at(d) = 0.0;
				lowLimitVec[stk.GetSymbol()].at(d) = 0.0;
			}
		}
	}

	aggauctionAM::aggauctionAM(){}



	void aggauctionAM::OnBar(Market *mkt, int tradeDate, std::vector<StockOrder> &stkVec)
	{

		AggAuctionValues values(stkVec);
		std::vector<Symbol> symbollist;
		symbollist.reserve(stkVec.size());

		time_t lstTime;
		time_t curTime;
		double preClose;
		double stage1High;
		double stage1Low;
		double stage1Close;
		double stage2Close;
		double curOpen;
		unsigned int aggAucAmo;
		vector<double> stage2CloseVec;
		time_t stage1endtime = 92001;
		time_t stage2endtime = 92501;
		double highLimit = 0.0;
		double lowLimit = 0.0;

		for (auto &stk : stkVec)
		{
			Symbol curSymbol;
			lstTime = 0;
			curTime = 0;
			stage1High = 0.0;
			stage1Low = 0.0;
			stage1Close = 0.0;
			stage2Close = 0.0;
			aggAucAmo = 0;
			curOpen = 0.0;
			preClose = 0.0;
			stage2CloseVec.clear();

			curSymbol = stk.GetSymbol();
			symbollist.emplace_back(curSymbol);
			const vector<Quote> &quoteList = stk.GetQuote(); // market->GetTransList(symbol);

			for (auto const &quote : quoteList)
			{
				curTime = quote.time;

				//单独判断，以防中间有虚拟撮合价，最终因为撤单却没有撮合价了
				if (lstTime < stage1endtime && curTime >= stage1endtime)
				{
					if (quote.curKClose>NaNValue)
						stage1Close = quote.curKClose;
				}

				if (lstTime < stage2endtime && curTime >= stage2endtime)
				{
					if(quote.curKClose>NaNValue)
						stage2Close = quote.curKClose;
				}

				if (curTime <= stage1endtime && quote.curKClose>NaNValue)
				{
					if (stage1High < quote.curKClose)
					{
						stage1High = quote.curKClose;
					}

					if (stage1Low < NaNValue) stage1Low = MaxValue;
					if (stage1Low > quote.curKClose)
					{
						stage1Low = quote.curKClose;
					}
				}
				if (curTime > stage1endtime && curTime <= stage2endtime && quote.curKClose>NaNValue)
				{
					stage2CloseVec.push_back(quote.curKClose);
				}

				//保留未形成开盘价的信息
				if (quote.curKOpen > NaNValue)
				{
					curOpen = quote.curKOpen;
					preClose = quote.preClose;
					aggAucAmo = quote.curKTurnover;
					highLimit = quote.curKHighLimit;
					lowLimit = quote.curKLowLimit;
					break;
				}
				lstTime = curTime;
			}
			values.crsDayPctVec[curSymbol][0] = curOpen<NaNValue? MaxValue : (curOpen / preClose-1)*100;
			values.stage1PctVec[curSymbol][0] = stage1Close<NaNValue ? MaxValue : (stage1Close / preClose - 1) * 100;

			double dStage2chgPrice = 0.0;
			if (stage2Close > NaNValue && stage1Close > NaNValue)
				dStage2chgPrice = stage2Close - stage1Close;
			else if (stage2Close > NaNValue && stage1Close < NaNValue)
				dStage2chgPrice = stage2Close - preClose;
			else if (stage2Close < NaNValue && stage1Close < NaNValue)
				dStage2chgPrice = NaNValue ;
			else
			{
				dStage2chgPrice = NaNValue ;
				std::cerr << "Error: " << curSymbol << " stage1 have close,but stage2 did not have" << endl;
			}
			values.stage2PctVec[curSymbol][0] = stage2Close > NaNValue?(dStage2chgPrice / preClose) * 100 : MaxValue;
			values.aggAucAmAmoutVec[curSymbol][0] = aggAucAmo;
			values.stage1HighVec[curSymbol][0] =  stage1High;
			values.stage1LowVec[curSymbol][0] = stage1Low;
			values.highLimitVec[curSymbol][0] = highLimit;
			values.lowLimitVec[curSymbol][0] = lowLimit;

			int nStage2PriceLen = stage2CloseVec.size();
			double dUpDncorr = 0.0;
			if (nStage2PriceLen <2) 
			{
				dUpDncorr = 0.0;
			}
			else
			{
				std::vector<double> ranktime;
				for (int i = 0; i < nStage2PriceLen; ++i)
				{
					ranktime.push_back(i);
				}
				dUpDncorr = calcCorr(ranktime, stage2CloseVec, nStage2PriceLen);
			}
			values.stepUpDownVec[curSymbol][0] = dUpDncorr;
			values.step2DnumVec[curSymbol][0] = nStage2PriceLen;
		}
		FactorToFile(mkt, tradeDate, symbollist, values);
	}

	bool aggauctionAM::FactorToFile(Market *mkt, int tradeDate, std::vector<Symbol>& symbollist, AggAuctionValues &values)
	{
		// <symbol,<value>>
		std::unordered_map<int, std::vector<double>> results;

		string title = "symbol";
		for (int i = 0; i < FACTOR_SIZE; ++i)
			title += "," + string(FACTOR_NAME[i]);

		for (int stk : symbollist)
		{
			results[stk].insert(results[stk].end(), values.crsDayPctVec[stk].begin(), values.crsDayPctVec[stk].end());
			results[stk].insert(results[stk].end(), values.stage1PctVec[stk].begin(), values.stage1PctVec[stk].end());
			results[stk].insert(results[stk].end(), values.stage2PctVec[stk].begin(), values.stage2PctVec[stk].end());
			results[stk].insert(results[stk].end(), values.aggAucAmAmoutVec[stk].begin(), values.aggAucAmAmoutVec[stk].end());
			results[stk].insert(results[stk].end(), values.stage1HighVec[stk].begin(), values.stage1HighVec[stk].end());
			results[stk].insert(results[stk].end(), values.stage1LowVec[stk].begin(), values.stage1LowVec[stk].end());
			results[stk].insert(results[stk].end(), values.stepUpDownVec[stk].begin(), values.stepUpDownVec[stk].end());
			results[stk].insert(results[stk].end(), values.step2DnumVec[stk].begin(), values.step2DnumVec[stk].end());
			results[stk].insert(results[stk].end(), values.highLimitVec[stk].begin(), values.highLimitVec[stk].end());
			results[stk].insert(results[stk].end(), values.lowLimitVec[stk].begin(), values.lowLimitVec[stk].end());
		}
		mkt->FactorToFile(_factorSaveName, tradeDate, title, results);

		results.clear();

		return true;

	}
}