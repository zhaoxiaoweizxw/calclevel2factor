#include "public.h"
//#include "util/tc_common.h"
#include <string>
using namespace std;
// using namespace boost::posix_time;
// using boost::filesystem::path;
// using boost::filesystem::directory_iterator;
namespace l2 {

/*
struct DateTime{
   unsigned short year;
   unsigned char month;
   unsigned char day;
   unsigned char seccond;
   unsigned char minute;
   unsigned char hour;
   unsigned char padding;

};
*/

const OrderSeq MAX_ORDER_SEQ = 300000000;
const OrderSeq INVALID_ORDER_SEQ = MAX_ORDER_SEQ - 1;
const TransSeq MAX_TRANS_SEQ = 300000000;
const TransSeq INVALID_TRANS_SEQ = MAX_TRANS_SEQ - 1;
const Symbol INVALID_SYMBOL = 999999;
const Symbol MAX_SYMBOL = 4000;
const Price INVALID_PRICE = 0;
const time_t INVALID_TIME = 0;
const time_t MIN_TIME = 0;
const float PRICE_SCOPE = 0.3;
// const ptime LAUNCH_TIME = microsec_clock::local_time();
//const int64_t LAUNCH_TIME = TC_Common::now2ms();        // microsec_clock::local_time();
//const string LAUNCH_TIME_STR = TC_Common::now2GMTstr(); // to_iso_string(LAUNCH_TIME);
const unsigned int DAY_SECONDS = 86400;
const unsigned int HOUR_SECONDS = 3600;
const unsigned int MINUTE_SECONDS = 60;

KBarHandler::~KBarHandler() {}


void KBarHandler::SetFactorSaveName(const char* strName)
{
	_factorSaveName = std::string(strName);
}
}
