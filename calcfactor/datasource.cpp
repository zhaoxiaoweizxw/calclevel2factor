#include "datasource.h"
//#include "common/AlgoLog.h"
//#include "public.h"
#include "../utils/utils_common.h"
//#include "utils/TimeUtil.h"
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

namespace l2
{

	static bool checkTime(const std::vector<std::pair<int, int>> &timefilter, int t)
	{
		for (auto const &v : timefilter)
		{
			if ((t >= v.first) && t < v.second)
			{
				return true;
			}
		}
		return false;
	}

	OrderSeq ChannelSeq(int channel, OrderSeq seq)
	{
		if (channel <= 6 && channel >= 1)
			return 10 * seq + channel - 1;
		else if (channel >= 2011 && channel <= 2014)
			return 10 * seq + channel - 2005;
		else
		{
			return INVALID_ORDER_SEQ;
		}
	}


	// OrderInfo ----------------------------------------------
	const string Order::ToStr() const
	{
		string str("");
		// tm *tempTm = localtime(&dt);
		str += "sym=" + std::to_string(symbol) + ",";
		// str += "dt=" + std::to_string(ptime_from_tm(*tempTm)) + ",";
		str += "ch=" + std::to_string(channel) + ",";
		str += "seq=" + std::to_string(cSeq) + ",";
		str += "p=" + std::to_string(price) + ",";
		str += "n=" + std::to_string(num) + ",";
		str += "d=" + std::to_string(direct) + ",";
		str += "t=" + std::to_string(type) + ",";
		str += "tn=" + std::to_string(transNum) + ",";
		str += "tn=" + std::to_string(transPrice) + ",";
		str += "cn=" + std::to_string(cancelNum);
		return str;
	}

	// TransInfo ----------------------------------------------
	const string Trans::ToStr() const
	{
		string str("");
		// tm *tempTm = localtime(&dt);
		str += "sym=" + std::to_string(symbol) + ",";
		// str += "dt=" + std::to_string(ptime_from_tm(*tempTm)) + ",";
		str += "ch=" + std::to_string(channel) + ",";
		str += "seq=" + std::to_string(cSeq) + ",";
		str += "bseq=" + std::to_string(cBidSeq) + ",";
		str += "oseq=" + std::to_string(cOfferSeq) + ",";
		str += "p=" + std::to_string(price) + ",";
		str += "n=" + std::to_string(num) + ",";
		str += "t=" + std::to_string(type);
		return str;
	}

	//Quota ----------------------------------------------
	Quota::Quota()
	{
		this->symbol = 0;
		this->date = 0;
		this->time = 0;
		this->preClose = 0;
		this->curKOpen = 0;
		this->curKHigh = 0;
		this->curKLow = 0;
		this->curKClose = 0;
		this->curKVol = 0;
		this->curKTurnover = 0;
		this->curKHighLimit = 0;
		this->curKLowLimit = 0;
		this->curKMeanPrice = 0;
		this->daysFromIPO = 0;
		memset(bidPrice, 0, 10);
		memset(bidVolumn, 0, 10);
		memset(offerPrice, 0, 10);
		memset(offerVolumn, 0, 10);
	}

	Quota::Quota(Symbol symbol, time_t dt, Price pClose, Price cKOpen, Price cKHigh, Price cKLow, Price cKClose, unsigned int cKVol, unsigned int cKTurnover, Price cKHighLimit,
		Price cKLowLimit, Price cKMeanPrice, Price daysFromIPO, const  Price bidPrice[], const Price bidVolumn[], const Price offerPrice[], const Price offerVolumn[])
	{
		this->symbol = symbol;
		this->time = dt;
		this->preClose = pClose;
		this->curKOpen = cKOpen;
		this->curKHigh = cKHigh;
		this->curKLow = cKLow;
		this->curKClose = cKClose;
		this->curKVol = cKVol;
		this->curKTurnover = cKTurnover;
		this->curKHighLimit = cKHighLimit;
		this->curKLowLimit = cKLowLimit;
		this->curKMeanPrice = cKMeanPrice;
		this->daysFromIPO = daysFromIPO;
		memcpy(this->bidPrice, bidPrice, 10);
		memcpy(this->bidVolumn, bidVolumn, 10);
		memcpy(this->offerPrice, offerPrice, 10);
		memcpy(this->offerVolumn, offerVolumn, 10);

	}

	Quota::~Quota() {}

	// Order --------------------------------------------------

	Order::Order()
	{
		this->channel = 0;
		this->seq = INVALID_ORDER_SEQ;
		this->cSeq = INVALID_ORDER_SEQ;
		this->price = 0;
		this->num = 0;
		this->direct = ' ';
		this->type = ' ';
		this->transNum = 0;
		this->transPrice = 0;
		this->cancelNum = 0;
		this->symbol = 0;
	}

	Order::Order(Symbol symbol, time_t dt)
	{
		this->symbol = symbol;
		this->dt = dt;
		this->channel = 0;
		this->seq = INVALID_ORDER_SEQ;
		this->cSeq = INVALID_ORDER_SEQ;
		this->price = 0;
		this->num = 0;
		this->direct = ' ';
		this->type = ' ';
		this->transNum = 0;
		this->transPrice = 0;
		this->cancelNum = 0;
	}

	Order::Order(Symbol symbol, time_t dt, int channel, OrderSeq seq, float price, unsigned int num, char direct, char type)
	{
		this->symbol = symbol;
		this->dt = dt;
		this->channel = channel;
		this->seq = ChannelSeq(channel, seq);
		this->cSeq = seq;
		this->price = int(round(price * 100)); //委托价格在内部会被乘以100并转换成整数，防止浮点数在计算中的误差
		this->num = num;
		this->direct = direct;
		this->type = type;
		this->transNum = 0;
		this->transPrice = 0;
		this->cancelNum = 0;
		if (this->seq == INVALID_ORDER_SEQ)
		{
			/*LOG_LOGIC_ERROR("unknown|"
			<< "got an order with invalid seq " << ToStr() << std::endl);*/
			std::cerr << "unknown|" << "got an order with invalid seq " << ToStr() << std::endl;
		}
	}

	Order::~Order() {}

	int Order::Cancel(unsigned int num)
	{
		unsigned int cNum = this->num > num ? num : this->num;
		this->num -= cNum;
		this->cancelNum += cNum;
		return this->num;
	}

	int Order::Trans(unsigned int num, double transPrice)
	{
		if (IsEmpty())
		{
			return 0;
		}
		else
		{
			unsigned int tNum = this->num > num ? num : this->num;
			this->num -= tNum;
			this->transPrice =
				((this->transNum * this->transPrice) + tNum * transPrice) / (this->transNum + tNum); // 成交均价
			this->transNum += tNum;
			return this->num;
		}
	}

	const Order *Order::GetInfo() const { return this; }

	bool Order::IsValid() const { return this->seq != INVALID_ORDER_SEQ; }

	// Trans --------------------------------------------------

	Trans::Trans()
	{
		this->type = '\0';
		this->seq = INVALID_TRANS_SEQ;
		this->cSeq = INVALID_TRANS_SEQ;
		this->bidSeq = INVALID_ORDER_SEQ;
		this->cBidSeq = INVALID_ORDER_SEQ;
		this->offerSeq = INVALID_ORDER_SEQ;
		this->cOfferSeq = INVALID_ORDER_SEQ;
		this->price = 0;
		this->symbol = 0;
		this->num = 0;
	}

	Trans::Trans(Symbol symbol, time_t dt)
	{
		this->symbol = symbol;
		this->dt = dt;
		this->channel = 0;
		this->seq = INVALID_TRANS_SEQ;
		this->cSeq = INVALID_TRANS_SEQ;
		this->bidSeq = INVALID_ORDER_SEQ;
		this->cBidSeq = INVALID_ORDER_SEQ;
		this->offerSeq = INVALID_ORDER_SEQ;
		this->cOfferSeq = INVALID_ORDER_SEQ;
		this->price = 0;
		this->num = 0;
		this->type = ' ';
	}

	Trans::Trans(Symbol symbol, time_t dt, int channel, TransSeq seq, OrderSeq bidSeq, OrderSeq offerSeq, float price,
		unsigned int num, char type)
	{
		this->symbol = symbol;
		this->dt = dt;
		this->channel = channel;
		this->seq = ChannelSeq(channel, seq);
		this->cSeq = seq;
		this->bidSeq = ChannelSeq(channel, bidSeq);
		this->cBidSeq = bidSeq;
		this->offerSeq = ChannelSeq(channel, offerSeq);
		this->cOfferSeq = offerSeq;
		this->price = int(round(price * 100));
		this->num = num;
		this->type = type;
		if (this->seq == INVALID_TRANS_SEQ)
		{
			//LOG_LOGIC_ERROR("unknown|"
			//                << "got an trans with invalid seq " << ToStr() << std::endl);
			std::cerr << "unknown|" << "got an trans with invalid seq " << ToStr() << std::endl;
		}
	}

	Trans::Trans(const Trans &t)
	{
		this->symbol = t.symbol;
		this->date = t.date;
		this->time = t.time;
		this->dt = t.dt;
		this->channel = t.channel;
		this->seq = t.seq;
		this->cSeq = t.cSeq;
		this->bidSeq = t.bidSeq;
		this->cBidSeq = t.cBidSeq;
		this->offerSeq = t.offerSeq;
		this->cOfferSeq = t.cOfferSeq;
		this->price = t.price;
		this->num = t.num;
		this->type = t.type;
	}

	Trans &Trans::operator=(const Trans &t)
	{
		this->symbol = t.symbol;
		this->dt = t.dt;
		this->channel = t.channel;
		this->seq = t.seq;
		this->cSeq = t.cSeq;
		this->bidSeq = t.bidSeq;
		this->cBidSeq = t.cBidSeq;
		this->offerSeq = t.offerSeq;
		this->cOfferSeq = t.cOfferSeq;
		this->price = t.price;
		this->num = t.num;
		this->type = t.type;
		return *this;
	}

	Trans::~Trans() {}

	const Trans *Trans::GetInfo() const { return this; }

	bool Trans::IsValid() const { return this->seq != INVALID_TRANS_SEQ; }

	// OrderStream --------------------------------------------
	OrderStream::OrderStream() {}

	OrderStream::~OrderStream() {}

	void OrderStream::load(Symbol symbol, const string &filename, const std::vector<std::pair<int, int>> &timefilter,
		std::vector<Order> &orderVec)
	{
		if (!TC_File::isFileExist(filename))
		{
			// LOG_DEBUG << "file is not exists: " << filename << "|" << symbol << std::endl;
			//std::cout << "file is not exists: " << filename << "|" << symbol << std::endl;
			return;
		}
		std::vector<std::string> data;
		std::string s = TC_File::load2str(filename);
		data = TC_Common::sepstr<std::string>(s, "\r\n");

		if (data.size())
		{
			orderVec.reserve(data.size());
			for (auto const &v : data)
			{
				Order order;
				order.symbol = symbol;
				if (!StrToOrder(v, order))
				{
					continue;
				}
				if (checkTime(timefilter, order.time) && order.IsValid())
				{
					orderVec.push_back(order);
				}
			}
		}
		else
		{
			//LOG_DEBUG << "load file failed or file is empty|" << filename << "|" << symbol << std::endl;
			std::cout << "load file failed or file is empty|" << filename << "|" << symbol << std::endl;
		}
	}

	bool OrderStream::StrToOrder(const std::string &str, Order &order)
	{
		// LOG_DEBUG << str << std::endl;
		char *last = nullptr;
		char *pArr[11];
		char *p = strtok_s((char *)(str.data()), ",", &last);
		int c = 0;
		while (p != NULL)
		{
			pArr[c] = p;
			p = strtok_s(NULL, ",", &last);
			c++;
			if (c == 11)
			{
				break;
			}
		}
		int swift = c > 9 ? 1 : 0;
		string dtStr;
		try
		{
			dtStr = str.substr(0, 4) + "-" + str.substr(4, 2) + "-" + str.substr(6, 2) + " " + string(pArr[1]);
			order.date = TC_Common::strto<int>(str.substr(0, 8));
			order.time = TC_Common::strto<int>(TC_Common::replace(string(pArr[1]), ":", ""));
		}
		catch (std::exception &e)
		{
			std::cerr << "exception: " << e.what() << "|" << str << std::endl;
			//LOG_ERROR << "exception: " << e.what() << "|" << str << std::endl;
			return false;
		}
		tm dtTm; // = to_tm(time_from_string(dtStr));
		TC_Common::str2tm(dtStr, "%Y-%m-%d %H:%M:%S", dtTm);
		time_t dt = mktime(&dtTm);
		int channel = atoi(pArr[2]);
		OrderSeq seq = atoi(pArr[3]);
		float price = atof(pArr[4 + swift]);
		int num = atoi(pArr[5 + swift]);
		char direct = pArr[6 + swift][0];
		char type = pArr[7 + swift][0];

		// order.symbol = symbol;
		order.dt = dt;
		order.channel = channel;
		order.seq = ChannelSeq(channel, seq);
		order.cSeq = seq;
		order.price = int(round(price * 100));
		order.num = num;
		order.direct = direct;
		order.type = type;

		order.transNum = 0;
		order.transPrice = 0;
		order.cancelNum = 0;
		return true;
	}

	bool OrderStream::GetOrder(Order &order) { return false; }

	// TransStream --------------------------------------------
	TransStream::TransStream() {}

	TransStream::~TransStream() {}

	void TransStream::load(Symbol symbol, const string &filename, const std::vector<std::pair<int, int>> &timefilter,
		std::vector<Trans> &transVec)
	{
		if (!TC_File::isFileExist(filename))
		{
			std::cout << "file is not exists: " << filename << std::endl;
			//LOG_DEBUG << "file is not exists: " << filename << std::endl;
			return;
		}
		std::vector<std::string> data;
		std::string s = TC_File::load2str(filename);
		data = TC_Common::sepstr<std::string>(s, "\r\n");

		if (data.size())
		{
			transVec.reserve(data.size());
			for (auto const &v : data)
			{
				Trans trans;
				trans.symbol = symbol;
				if (!StrToTrans(v, trans))
				{
					continue;
				}
				if (checkTime(timefilter, trans.time) && trans.IsValid())
				{
					transVec.push_back(trans);
				}
			}
		}
		else
		{
			// LOG_DEBUG << "load file failed or file is empty|" << filename << "|" << symbol << std::endl;
			std::cout << "load file failed or file is empty|" << filename << "|" << symbol << std::endl;
		}
	}

	bool TransStream::StrToTrans(const string &str, Trans &trans)
	{
		// LOG_DEBUG << str << std::endl;
		char *last = nullptr;
		char *pArr[12];
		char *p = strtok_s((char *)(str.data()), ",", &last);
		int c = 0;
		while (p != NULL)
		{
			pArr[c] = p;
			p = strtok_s(NULL, ",", &last);
			c++;
			if (c == 12)
			{
				break;
			}
		}
		int swift = c > 10 ? 1 : 0;
		string dtStr;
		try
		{
			dtStr = str.substr(0, 4) + "-" + str.substr(4, 2) + "-" + str.substr(6, 2) + " " + string(pArr[1]);
			trans.date = TC_Common::strto<int>(str.substr(0, 8));
			trans.time = TC_Common::strto<int>(TC_Common::replace(string(pArr[1]), ":", ""));
		}
		catch (std::exception &e)
		{
			std::cerr << "exception: " << e.what() << "|" << str << std::endl;
			//LOG_ERROR << "exception: " << e.what() << "|" << str << std::endl;
			return false;
		}
		tm dtTm; // = to_tm(time_from_string(dtStr));
		TC_Common::str2tm(dtStr, "%Y-%m-%d %H:%M:%S", dtTm);
		// LOG_DEBUG << "DateTime: " << dtStr << std::endl;
		time_t dt = mktime(&dtTm);
		int channel = atoi(pArr[2]);
		long seq = atoi(pArr[3]);
		long bidSeq = atoi(pArr[4 + swift]);
		long offerSeq = atoi(pArr[5 + swift]);
		float price = atof(pArr[6 + swift]);
		int num = atoi(pArr[7 + swift]);
		char type = pArr[8 + swift][0];
		if (type == 'B' || type == 'S') //此为上交所成交信息
			type = 'F';

		// trans.symbol = symbol;
		trans.dt = dt;
		trans.channel = channel;
		trans.seq = ChannelSeq(channel, seq);
		trans.cSeq = seq;
		trans.bidSeq = ChannelSeq(channel, bidSeq);
		trans.cBidSeq = bidSeq;
		trans.offerSeq = ChannelSeq(channel, offerSeq);
		trans.cOfferSeq = offerSeq;
		trans.price = int(round(price * 100));
		trans.num = num;
		trans.type = type;
		return true;
	}

	bool TransStream::GetTrans(Trans &trans) { return false; }

	StockOrder::StockOrder(Symbol symbol) : _symbol(symbol) {}

	StockOrder::StockOrder(Symbol symbol, const string &trans, const string &order) : _symbol(symbol), _transfile(trans), _orderfile(order)
	{

	}

	void StockOrder::AddOrder(Order &o) { _orderVec.push_back(o); }

	void StockOrder::AddTrans(Trans &t) { _transVec.push_back(t); }

	bool StockOrder::GetOrder(OrderSeq seq, Order &o)
	{
		auto itIndex = _orderIndexMap.find(seq);
		if (itIndex == _orderIndexMap.end())
		{
			return false;
		}
		o = _orderVec.at(itIndex->second);
		return true;

		/*auto it = std::find_if(_orderVec.begin(), _orderVec.end(), [seq](const Order &v) { return v.seq == seq; });
		if (it != _orderVec.end())
		{
			o = *it;
			return true;
		}
		return false;*/
	}

	bool StockOrder::Load(const string &transfile, const string &orderfile,
		const std::vector<std::pair<int, int>> &timefilter)
	{
		TransStream ts;
		ts.load(_symbol, transfile, timefilter, _transVec);

		OrderStream os;
		os.load(_symbol, orderfile, timefilter, _orderVec);

		return true;
	}

	void StockOrder::BuildOrderIndexMap()
	{
		for (int i = 0; i < _orderVec.size();++i)
		{
			auto const &o = _orderVec[i];
			_orderIndexMap.insert(make_pair(o.seq,i));
		}
	}

	void StockOrder::PrepareData()
	{
		BuildOrderIndexMap();
		for (auto const &t : _transVec)
		{
			if (!t.IsValid())
			{
				continue;
			}
			if (t.type == 'F')
			{ // transact
				OrderTrans(t.bidSeq, t.num, t.price);
				if (t.symbol < 60000)
				{
					/*
					LOG_LOGIC_ERROR("|attempt to transact a invalid bid order "
					<< string(",symbol=") << t.symbol << string(",orderSeq=") << t.bidSeq
					<< string(",transSeq=") << t.seq << std::endl);
					*/
				}
				OrderTrans(t.offerSeq, t.num, t.price);
				if (t.symbol < 60000)
				{
					/*
					LOG_LOGIC_INFO("|attempt to transact a invalid offer order"
					<< string(",symbol=") << t.symbol << string(",orderSeq=") << t.offerSeq
					<< string(",transSeq=") << t.seq << std::endl);
					*/
				}
			}
			else if (t.type == '4')
			{ // cancel
				if (t.cBidSeq > 0 && t.cOfferSeq == 0)
				{ // cancel bid
					OrderCancel(t.bidSeq, t.num);
					if (t.symbol < 60000)
					{
						/*
						LOG_LOGIC_ERROR("|attempt to cancel a invalid bid order"
						<< string(",symbol=") << t.symbol << string(",orderSeq=") << t.bidSeq
						<< string(",transSeq=") << t.seq << std::endl);
						*/
					}
				}
				else if (t.cBidSeq == 0 && t.cOfferSeq > 0)
				{ // cancel offer
					OrderCancel(t.offerSeq, t.num);
					if (t.symbol < 60000)
					{
						/*
						LOG_LOGIC_ERROR("|attempt to cancel a invalid offer order"
						<< string(",symbol=") << t.symbol << string(",orderSeq=") << t.offerSeq
						<< string(",transSeq=") << t.seq << std::endl);
						*/
					}
				}
				else
				{ // unknown
				  /*LOG_LOGIC_ERROR("|unknown cancel type: bid or offer?" << string(",symbol=") << t.symbol
				  << string(",transSeq=") << t.seq << std::endl);*/
					std::cerr << "|unknown cancel type: bid or offer?" << string(",symbol=") << t.symbol
						<< string(",transSeq=") << t.seq << std::endl;
				}
			}
		}
	}

	void StockOrder::OrderTrans(OrderSeq seq, unsigned int num, Price price)
	{
		auto itIndex = _orderIndexMap.find(seq);
		if (itIndex == _orderIndexMap.end())
		{
			return;
		}
		auto it = _orderVec.begin()+itIndex->second;
		

		/*auto it = std::find_if(_orderVec.begin(), _orderVec.end(), [seq](const Order &v) { return v.seq == seq; });
		if (it == _orderVec.end())
		{
			return;
		}*/
		if (it->IsValid())
		{
			it->Trans(num, price);
		}
	}

	void StockOrder::OrderCancel(OrderSeq seq, unsigned int num)
	{
		auto itIndex = _orderIndexMap.find(seq);
		if (itIndex == _orderIndexMap.end())
		{
			return;
		}
		auto it = _orderVec.begin() + itIndex->second;

		/*auto it = std::find_if(_orderVec.begin(), _orderVec.end(), [seq](const Order &v) { return v.seq == seq; });
		if (it == _orderVec.end())
		{
			return;
		}*/
		if (it->IsValid())
		{
			it->Cancel(num);
		}
	}

} // namespace l2
