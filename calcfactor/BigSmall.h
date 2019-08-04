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

struct BigSmallValues
{
    std::map<Symbol, std::vector< double>> buyVolVec; // 1.div, 2.symbol, 3.days
    std::map<Symbol, std::vector< double>> sellVolVec;
    std::map<Symbol, std::vector< double>> quitsVolVec;
    std::map<Symbol, std::vector< double>> buyAmountVec;
     std::map<Symbol, std::vector< double>> sellAmountVec;
    std::map<Symbol, std::vector< double>>  quitsAmountVec;

    BigSmallValues(const std::vector<StockOrder> &stkVec);
};

class BigSmall: public KBarHandler
{
  private:

	  //std::vector<Symbol> _symbollist;
	  // @param factorName	���ӵ�ϵ�����ƣ��Դ˽����ļ���
	  std::string _factorSaveName;


public:
    BigSmall();

	void SetFactorSaveName(const char* strName);
    //
    void OnBar(Market* mkt, int tradeDate, std::vector<StockOrder> &stkVec) override;

    bool ToSQLite(Market *mkt, int tradeDate, BigSmallValues &values);

	//����Ҫһ��һ��Ĵ棬�������������жϵ�ʱ����м�����������Ǵ�ͷ����
	/**
	@param tradeDate	��ŵ�����
	@param values		Ҫ�洢������
	**/
	bool FactorToFile(Market *mkt,int tradeDate, std::vector<Symbol>& symbollist,BigSmallValues &values);
};
}
