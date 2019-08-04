#pragma once
#include "public.h"
#include <ctime>
#include <fstream>
#include <list>
#include <vector>
#include <map>
using std::list;
using std::string;
using std::vector;
namespace l2
{

	OrderSeq ChannelSeq(int channel, OrderSeq seq);

	class Quota 
	{
	public:
		//����һ���տ���
		Quota();
		//����һ��ί��
		Quota(Symbol symbol, time_t dt, Price pClose,Price cKOpen,Price cKHigh,Price cKLow,Price cKClose,unsigned int cKVol,unsigned int cKTurnover,Price cKHighLimit,
			Price cKLowLimit,Price cKMeanPrice,Price daysFromIPO, const Price bidPrice[], const Price bidVolumn[], const Price offerPrice[],const Price offerVolumn[]);

		~Quota();

		Symbol symbol;
		int date;
		int time;
		Price preClose;
		Price curKOpen;
		Price curKHigh;
		Price curKLow;
		Price curKClose;
		unsigned int curKVol;
		unsigned int curKTurnover;
		Price curKHighLimit;
		Price curKLowLimit;
		Price curKMeanPrice;
		Price daysFromIPO;
		Price bidPrice[10];
		Price bidVolumn[10];
		Price offerPrice[10];
		Price offerVolumn[10];

	};
	class Order
	{
	public:
		//����һ����ί��
		Order();
		//����һ����Чί��
		Order(Symbol symbol, time_t dt);
		//����һ��ί��
		Order(Symbol symbol, time_t dt, int channel, OrderSeq seq, float price, unsigned int num, char direct, char type);

		~Order();

		//������numΪ�������������س�����ʣ������
		int Cancel(unsigned int num);
		//�ɽ���numΪ�ɽ����������سɽ���ʣ������
		int Trans(unsigned int num, double transPrice);
		//ת��Ϊ�ַ���
		const string ToStr() const;

		const Order *GetInfo() const;

		bool IsValid() const;
		bool IsEmpty() const { return symbol == 0; }

		Symbol symbol;
		time_t dt; //ί��ʱ��
		int date;
		int time;
		int channel;            //ͨ����
		OrderSeq seq;           //ί�����(ȫ��)
		OrderSeq cSeq;          //ί�����(ͨ����)
		Price price;            //ί�м۸��100������
		unsigned int num;       //ί������
		char direct;            //ί�з���1��2����G���룬F����
		char type;              //ί�����ͣ�1�мۣ�2�޼ۣ�U��������
		unsigned int transNum;  //�ѳɽ�����
		double transPrice;      // �ɽ�����
		unsigned int cancelNum; //�ѳ�������
		unsigned int refCount;  //���ü���
	};

	class Trans
	{
	public:
		Symbol symbol;
		time_t dt; //�ɽ�ʱ��
		int date;
		int time;
		int channel;        //ͨ����
		TransSeq seq;       //�ɽ����(ȫ��)
		TransSeq cSeq;      //�ɽ����(ͨ����)
		OrderSeq bidSeq;    //ί�����(ȫ��)
		OrderSeq cBidSeq;   //ί�����(ͨ����)
		OrderSeq offerSeq;  //ί�����(ȫ��)
		OrderSeq cOfferSeq; //ί�����(ͨ����)
		Price price;        //�ɽ��۸�
		unsigned int num;   //�ɽ�����
		char type;          //ִ������ F�ɽ���4����
							//����һ���ճɽ���¼
		Trans();
		//����һ����Ч�ɽ���¼
		Trans(Symbol symbol, time_t dt);
		//����һ���ɽ���¼
		Trans(Symbol symbol, time_t dt, int channel, TransSeq seq, OrderSeq bidSeq, OrderSeq offerSeq, float price,
			unsigned int num, char type);
		Trans(const Trans &trans);
		Trans &operator=(const Trans &trans);

		~Trans();

		const string ToStr() const;

		const Trans *GetInfo() const;

		bool IsValid() const;
		bool IsEmpty() const { return symbol == 0; }
	};

	class OrderStream
	{
	public:
		OrderStream();
		~OrderStream();
		void load(Symbol symbol, const string &filename, const std::vector<std::pair<int, int>> &timefilter,
			std::vector<Order> &orderVec);
		bool StrToOrder(const string &str, Order &order);
		bool GetOrder(Order &order);
	};

	class TransStream
	{
	public:
		TransStream();
		~TransStream();
		void load(Symbol symbol, const string &filename, const std::vector<std::pair<int, int>> &timefilter,
			std::vector<Trans> &transVec);
		bool StrToTrans(const string &str, Trans &trans);
		bool GetTrans(Trans &trans);
	};

	class StockOrder
	{
	private:
		// ���
		Symbol _symbol;

		// bid ��
		// std::vector<Order> _bidOrder;
		// offer ���۵�
		// std::vector<Order> _offerOrder;
		std::vector<Order> _orderVec;
		std::vector<Trans> _transVec;
		// ��¼order��λ�ã��ÿռ任ʱ��
		std::map<OrderSeq, int> _orderIndexMap;
		std::string _transfile;
		std::string _orderfile;

	public:
		StockOrder(Symbol symbol = 0);
		StockOrder(Symbol symbol, const std::string &trans, const std::string &order);
		void AddOrder(Order &o);
		void AddTrans(Trans &t);
		const std::vector<Order> &GetOrder() { return _orderVec; }
		const std::vector<Trans> &GetTrans() { return _transVec; }
		Symbol GetSymbol() const { return _symbol; }
		bool GetOrder(OrderSeq seq, Order &o);
		bool Load(const std::string &transfile, const std::string &orderfile,
			const std::vector<std::pair<int, int>> &timefilter);
		bool Load(const std::vector<std::pair<int, int>> &timefilter) { return Load(_transfile, _orderfile, timefilter); }
		void PrepareData();
		void OrderTrans(OrderSeq seq, unsigned int num, Price price);
		void OrderCancel(OrderSeq seq, unsigned int num);
		const string GetSymbolStr() const
		{
			char buf[16] = { 0x00 };
			std::snprintf(buf, sizeof(buf), "%06d.CS", _symbol);
			return std::string(buf);
		}

		void BuildOrderIndexMap();

		void Release()
		{
			_orderVec.clear();
			_transVec.clear();
			_orderIndexMap.clear();
		}
	};
}
