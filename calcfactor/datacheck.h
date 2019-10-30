#pragma once
#include "market.h"
#include "public.h"
#include <list>
#include <string>
#include <vector>
using std::list;
using std::string;
using std::vector;
namespace l2 {

	/**
	����1����ʳɽ�����͡�
	**/
	struct DataCheckValues
	{
		std::map<Symbol, std::vector<double> > transVolSumVec;// 1.div, 2.symbol

		DataCheckValues(const std::vector<StockOrder> &stkVec);
	};

	class dataCheckL2 : public KBarHandler
	{
	private:

		//std::vector<Symbol> _symbollist;

	public:
		dataCheckL2();

		//
		void OnBar(Market* mkt, int tradeDate, std::vector<StockOrder> &stkVec) override;


		//����Ҫһ��һ��Ĵ棬�������������жϵ�ʱ����м�����������Ǵ�ͷ����
		/**
		@param tradeDate	��ŵ�����
		@param values		Ҫ�洢������
		**/
		bool FactorToFile(Market *mkt, int tradeDate, std::vector<Symbol>& symbollist, DataCheckValues &values);
	};
}
