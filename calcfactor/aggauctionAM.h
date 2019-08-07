#pragma once
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
	����1����ҹ�Ƿ��������̼����ǰ���̼۵��Ƿ���
	����2���׶�1��9:15-9:20���ý׶ο��Գ������Ƿ���
	����3���׶�2��9:20-9:25���ý׶��޷��������Ƿ���
	����4�����Ͼ������ȣ�240�����Ͼ��۳ɽ���/��ȥ�����վ��ɽ����#����ɽ����
	����5���׶�1�Ƿ���ͣ��# ������߼�
	����6���׶�1�Ƿ��ͣ��
	����7���׶�2�۸��Ƿ�ƽ��������
	����8������7�ĸ�������¼�׶�2�۸������
	����9������5�ĸ�������¼�ù�Ʊ����ͣ�ۡ�
	����10������6�ĸ�������¼�ù�Ʊ�ĵ�ͣ�ۡ�
	**/
	struct AggAuctionValues
	{
		std::map<Symbol, std::vector<double> > crsDayPctVec;// 1.div, 2.symbol
		std::map<Symbol, std::vector<double> > stage1PctVec;
		std::map<Symbol, std::vector<double> > stage2PctVec;
		std::map<Symbol, std::vector<double> > aggAucAmAmoutVec;
		std::map<Symbol, std::vector<double> > stage1HighVec;
		std::map<Symbol, std::vector<double> > stage1LowVec;
		std::map<Symbol, std::vector<double> > stepUpDownVec;
		std::map<Symbol, std::vector<double> > step2DnumVec;
		std::map<Symbol, std::vector<double> > highLimitVec;
		std::map<Symbol, std::vector<double> > lowLimitVec;

		AggAuctionValues(const std::vector<StockOrder> &stkVec);
	};

	class aggauctionAM : public KBarHandler
	{
	private:

		//std::vector<Symbol> _symbollist;

	public:
		aggauctionAM();

		//
		void OnBar(Market* mkt, int tradeDate, std::vector<StockOrder> &stkVec) override;


		//����Ҫһ��һ��Ĵ棬�������������жϵ�ʱ����м�����������Ǵ�ͷ����
		/**
		@param tradeDate	��ŵ�����
		@param values		Ҫ�洢������
		**/
		bool FactorToFile(Market *mkt, int tradeDate, std::vector<Symbol>& symbollist, AggAuctionValues &values);
	};
}
