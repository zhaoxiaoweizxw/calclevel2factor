
#include <iostream>
#include <string>
#include <map>
//#include "../utils/utils_common.h"

#include "level2.h"

using namespace std;

void main(int argc, char **argv)
{
	string helpInfo = "parameter example: -dataRoot e:/level2/ -fastrunpath c:/fastrun954/ -targetPath ../summary/ -end 20161015 -begin 20161013 -factorlist earlymoney";
	string dataRoot = "", targetPath = "./summary/",fastPath = "";
	string begin = "", end = "";
	string saveDays = "0", backDays = "1";
	string reservedSize = "90";
	string cutOff = "1500";
	string freq = "day";
	string factorlist = "";
	string threadnumb = "3";

	map<string, string> mp_args;
	mp_args.insert(make_pair("threadnum", threadnumb));
	for (int p = 1; p < argc; p += 2) {
		if (strcmp(argv[p], "-dataRoot") == 0) {//存放原始level2压缩文件的文件夹
			dataRoot = argv[p + 1];
			mp_args.insert(make_pair("dataRoot", dataRoot));
		}
		else if (strcmp(argv[p], "-targetPath") == 0) {
			targetPath = argv[p + 1];
			mp_args.insert(make_pair("targetPath", targetPath));
		}
		else if (strcmp(argv[p], "-begin") == 0) {
			begin = argv[p + 1];
			mp_args.insert(make_pair("begindate", begin));
		}
		else if (strcmp(argv[p], "-end") == 0) {
			end = argv[p + 1];
			mp_args.insert(make_pair("enddate", end));
		}
		else if (strcmp(argv[p], "-saveDays") == 0) {
			saveDays = argv[p + 1];
			mp_args.insert(make_pair("saveDays", saveDays));
		}
		else if (strcmp(argv[p], "-backDays") == 0) {
			backDays = argv[p + 1];
			mp_args.insert(make_pair("backDays", backDays));
		}
		else if (strcmp(argv[p], "-reservedSize") == 0) {
			reservedSize = argv[p + 1];
			mp_args.insert(make_pair("reservedSize", reservedSize));
		}
		else if (strcmp(argv[p], "-cutOff") == 0) {
			cutOff = argv[p + 1];
			mp_args.insert(make_pair("cutOff", cutOff));
		}
		else if (strcmp(argv[p], "-freq") == 0) {
			freq = argv[p + 1];
			mp_args.insert(make_pair("freq", freq));
		}
		else if (strcmp(argv[p], "-factorlist") == 0) {
			factorlist = argv[p + 1];
			mp_args.insert(make_pair("factorlist", factorlist));
		}
		else if (strcmp(argv[p], "-fastrunpath") == 0) {//固态硬盘的临时工作文件夹，解压缩，io更快
			fastPath = argv[p + 1];
			mp_args.insert(make_pair("fastrunPath", fastPath));
		}
		else if (strcmp(argv[p], "-threadnum") == 0) {//配置线程数
			threadnumb = argv[p + 1];
			mp_args["threadnum"] = threadnumb;
		}
	}

	std::cout << "======== parameter ========" << std::endl;
	std::cout << "dataRoot    : " << dataRoot << std::endl;
	std::cout << "targetPath  : " << targetPath << std::endl;
	std::cout << "begin       : " << begin << std::endl;
	std::cout << "end         : " << end << std::endl;
	std::cout << "saveDays    : " << saveDays << std::endl;
	std::cout << "backDays    : " << backDays << std::endl;
	std::cout << "reservedSize: " << reservedSize << std::endl;
	std::cout << "cutOff      : " << cutOff << std::endl;
	std::cout << "freq        : " << freq << std::endl;
	std::cout << "factorlist  : " << factorlist << std::endl;
	std::cout << "===========================" << std::endl;


	factorLevel2(mp_args);
}