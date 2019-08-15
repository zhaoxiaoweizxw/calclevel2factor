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

	class clsQuoteIndex
	{
	public:
		clsQuoteIndex()
		{
			this->startIndex = -1;
			this->endIndex = -1;
		}
		clsQuoteIndex(int nStart,int nEnd)
		{
			this->startIndex = nStart;
			this->endIndex = nEnd;
		}
		~clsQuoteIndex() {};
		int startIndex;
		int endIndex;
	};

	class Quote 
	{
	public:
		//构造一个空快照
		Quote();
		//构造一个委托
		Quote(Symbol symbol, time_t dt, float pClose, float cKOpen, float cKHigh, float cKLow, float cKClose,unsigned int cKVol,unsigned int cKTurnover, float cKHighLimit,
			float cKLowLimit, float cKMeanPrice, unsigned int daysFromIPO, const float bidPrice[], const unsigned int  bidVolumn[], const float offerPrice[],const unsigned int  offerVolumn[]);

		Quote &operator=(const Quote &t);
		~Quote();

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
		unsigned int daysFromIPO;
		Price bidPrice[10];
		unsigned int  bidVolumn[10];
		Price offerPrice[10];
		unsigned int  offerVolumn[10];
		bool operator<( const Quote m2)const {
			return this->time<m2.time;
		}

	};
	class Order
	{
	public:
		//构造一个空委托
		Order();
		//构造一个无效委托
		Order(Symbol symbol, time_t dt);
		//构造一个委托
		Order(Symbol symbol, time_t dt, int channel, OrderSeq seq, float price, unsigned int num, char direct, char type);

		~Order();

		//撤单，num为撤单数量，返回撤单后剩余数量
		int Cancel(unsigned int num);
		//成交，num为成交数量，返回成交后剩余数量
		int Trans(unsigned int num, double transPrice);
		//转换为字符串
		const string ToStr() const;

		const Order *GetInfo() const;

		bool IsValid() const;
		bool IsEmpty() const { return symbol == 0; }

		Symbol symbol;
		time_t dt; //委托时间
		int date;
		int time;
		int channel;            //通道号
		OrderSeq seq;           //委托序号(全局)
		OrderSeq cSeq;          //委托序号(通道内)
		Price price;            //委托价格×100，整数
		unsigned int num;       //委托数量
		char direct;            //委托方向，1买，2卖，G借入，F出借
		char type;              //委托类型，1市价，2限价，U本方最优
		unsigned int transNum;  //已成交数量
		double transPrice;      // 成交均价
		unsigned int cancelNum; //已撤单数量
		unsigned int refCount;  //引用计数
	};

	class Trans
	{
	public:
		Symbol symbol;
		time_t dt; //成交时间
		int date;
		int time;
		int channel;        //通道号
		TransSeq seq;       //成交序号(全局)
		TransSeq cSeq;      //成交序号(通道内)
		OrderSeq bidSeq;    //委买序号(全局)
		OrderSeq cBidSeq;   //委买序号(通道内)
		OrderSeq offerSeq;  //委卖序号(全局)
		OrderSeq cOfferSeq; //委卖序号(通道内)
		Price price;        //成交价格
		unsigned int num;   //成交数量
		char type;          //执行类型 F成交，4撤单
							//构造一个空成交记录
		Trans();
		//构造一个无效成交记录
		Trans(Symbol symbol, time_t dt);
		//构造一个成交记录
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

	class QuoteStream
	{
	public:
		QuoteStream();
		~QuoteStream();
		void load(Symbol symbol, const string &filename, const std::vector<std::pair<int, int>> &timefilter,
			std::vector<Quote> &quotaVec);
		bool StrToQuote(const string &str, Quote &quota);
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
		// 标的
		Symbol _symbol;

		// bid 买单
		// std::vector<Order> _bidOrder;
		// offer 报价单
		// std::vector<Order> _offerOrder;
		std::vector<Order> _orderVec;
		std::vector<Trans> _transVec;
		std::vector<Quote> _quoteVec;
		// 记录order的位置，用空间换时间
		std::map<OrderSeq, int> _orderIndexMap;

		// 记录quota分钟开始的位置
		std::map<int, clsQuoteIndex*> _quoteIndexMap;

		std::string _transfile;
		std::string _orderfile;
		std::string _quotefile;

	public:
		StockOrder(Symbol symbol = 0);
		StockOrder(Symbol symbol, const std::string &trans, const std::string &order,const std::string &quota);
		void AddOrder(Order &o);
		void AddTrans(Trans &t);
		void AddQuote(Quote &q);
		const std::vector<Order> &GetOrder() { return _orderVec; }
		const std::vector<Trans> &GetTrans() { return _transVec; }
		const std::vector<Quote> &GetQuote() { return _quoteVec; }
		Symbol GetSymbol() const { return _symbol; }
		bool GetOrder(OrderSeq seq, Order &o);
		bool GetPreTimeQuote(int time, Quote &q);
		bool GetTimeRangeQuotes(int starttime, int endtime, std::vector<Quote> & qVec);
		int calcQuoteIndex(int time);
		bool Load(const std::string &transfile, const std::string &orderfile,const std::string &quotafile,
			const std::vector<std::pair<int, int>> &timefilter);
		bool Load(const std::vector<std::pair<int, int>> &timefilter) { return Load(_transfile, _orderfile,_quotefile ,timefilter); }
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
		void BuildQuoteIndexMap();

		void Release()
		{
			_orderVec.clear();
			std::vector<Order>().swap(_orderVec);
			_transVec.clear();
			std::vector<Trans>().swap(_transVec);
			_quoteVec.clear();
			std::vector<Quote>().swap(_quoteVec);
			_orderIndexMap.clear();
			_quoteIndexMap.clear();
		}

		
	};
}
