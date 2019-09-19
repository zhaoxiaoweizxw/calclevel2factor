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


	//����Ϊ��λ��ͳ��ÿ������ĳɽ���
	struct PerBuckValues
	{
		std::map<Symbol, std::vector< double>> ActiveBuyAmtVec;// 1.div, 2.symbol
		std::map<Symbol, std::vector< double>> ActiveSelAmtVec;// 1.div, 2.symbol
		std::map<Symbol, std::vector< double>> PassivBuyAmtVec;// 1.div, 2.symbol
		std::map<Symbol, std::vector< double>> PassivSelAmtVec;// 1.div, 2.symbol

		std::map<Symbol, std::vector< double>> BreakBuyAmtVec;// 1.div, 2.symbol
		std::map<Symbol, std::vector< double>> BreakSelAmtVec;// 1.div, 2.symbol

		void initBuck(const std::vector<StockOrder> &stkVec, int DIV_SIZE);

		~PerBuckValues();
	};

	class SoftbigAmall : public KBarHandler
	{
	private:

		//std::vector<Symbol> _symbollist;

		const double AMOUNT_LIMIT = 9999999999999999;
		vector<double> Buck_DIV_ARR;
		vector<string> Buck_DIV_NAME;

		vector<string> Buck_DIV_KIND;
		unsigned int DIV_SIZE;

		bool bCalcBreakPrice;

	public:
		//SoftbigAmall();
		SoftbigAmall(int UpLimit, int nSpan=1);
		SoftbigAmall::~SoftbigAmall();

		unsigned int GetAmountInterval(double amount);
		//
		void OnBar(Market* mkt, int tradeDate, std::vector<StockOrder> &stkVec) override;

		void setBreak(bool bBreak);

		void insertToBuck(Symbol netsymbol, long lastBidAmo, long lstOfferAmo, PerBuckValues& values);


		//����Ҫһ��һ��Ĵ棬�������������жϵ�ʱ����м�����������Ǵ�ͷ����
		/**
		@param tradeDate	��ŵ�����
		@param values		Ҫ�洢������
		**/
		bool FactorToFile(Market *mkt, int tradeDate, std::vector<Symbol>& symbollist, PerBuckValues &values);
	};
}
