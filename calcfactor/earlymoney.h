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
ͳ���������������׵Ĵ�С��
0~4��ΪС����4~20��Ϊ�е���20��100��Ϊ��
**/
struct ActPasValues
{
    std::map<Symbol, std::vector< double>> ActivebuyAmtVec;// 1.div, 2.symbol
    std::map<Symbol, std::vector< double>> ActiveselAmtVec;
	std::map<Symbol, std::vector< double>> ActivebuyVolVec;// 1.div, 2.symbol
	std::map<Symbol, std::vector< double>> ActiveselVolVec;
	//std::map<Symbol, std::vector< double>> PasivebuyAmtVec;// 1.div, 2.symbol
	//std::map<Symbol, std::vector< double>> PasiveselAmtVec;

	ActPasValues(const std::vector<StockOrder> &stkVec);
};

class EarlyMoney: public KBarHandler
{
  private:

	  //std::vector<Symbol> _symbollist;

public:
	EarlyMoney();

    //
    void OnBar(Market* mkt, int tradeDate, std::vector<StockOrder> &stkVec) override;


	//����Ҫһ��һ��Ĵ棬�������������жϵ�ʱ����м�����������Ǵ�ͷ����
	/**
	@param tradeDate	��ŵ�����
	@param values		Ҫ�洢������
	**/
	bool FactorToFile(Market *mkt,int tradeDate, std::vector<Symbol>& symbollist, ActPasValues &values);
};
}
