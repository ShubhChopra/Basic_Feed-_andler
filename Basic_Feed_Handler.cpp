//============================================================================
// Author      : Shubh
// Version     :
// Copyright   : Your copyright notice
//============================================================================

#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <time.h>
#include <unordered_map>
#include "pitch"

using namespace std;
template<typename A, typename B>
std::pair<B, A> flip_pair(const std::pair<A, B> &map_OrderIdToExecutions) {
	return std::pair<B, A>(map_OrderIdToExecutions.second, map_OrderIdToExecutions.first);
}

template<typename A, typename B>
std::multimap<B, A> flip_map(const std::map<A, B> &map_OrderIdToExecutions) {
	std::multimap<B, A> multimap_OrderIdToExecutions_flip;
	std::transform(map_OrderIdToExecutions.begin(),
			map_OrderIdToExecutions.end(),
			std::inserter(multimap_OrderIdToExecutions_flip,
					multimap_OrderIdToExecutions_flip.begin()),
			flip_pair<A, B>);
	return multimap_OrderIdToExecutions_flip;
}

void sortAndPrint(map<string, long> map_OrderIdToExecutions) {
	//exchanging the keys and values for map to sort them
	std::multimap<long, string> multimap_OrderIdToExecutions_flip = flip_map(
			map_OrderIdToExecutions);
	cout << "The 10 most frequent stock symbol by volume executed are :" << endl;
	short count = 0;
	map<long, string>::reverse_iterator it;
	for (it = multimap_OrderIdToExecutions_flip.rbegin();
			it != multimap_OrderIdToExecutions_flip.rend() && count <= 9; ++it) {
		std::cout << "(" << ++count << ") " << it->second << " " << it->first << endl;
	}
}

int main(int argc, char*argv[]) {
	string strPitchLine;
	unordered_map<string, pair<string, long> > map_OrderIdToSymbol; // hashmap of order id to symbol
	map<string, long> map_OrderIdToExecutions; // map of order id to executed shares
	while (getline(cin, strPitchLine)) {
		if (strPitchLine == "")
			break;

		const char* buf = strPitchLine.c_str();

		switch (buf[9]) {
		case 'A':
			try {
				addorder *ao;
				ao = (addorder*) buf;
				string strOrderId(((addorder*) buf)->szOrderId,
						sizeof(((addorder*) buf)->szOrderId));
				string strSymbol(((addorder*) buf)->szSymbol, sizeof(((addorder*) buf)->szSymbol));
				string strShares(((addorder*) buf)->szShares, sizeof(((addorder*) buf)->szShares));
				map_OrderIdToSymbol[strOrderId] = make_pair(strSymbol, stol(strShares));
			} catch (exception &e) {
				cout << "Caught exception when processing AddOrder message : " << e.what() << endl;
			}
			break;

		case 'E':
			try {
				execorder *eo;
				eo = (execorder*) buf;
				string strOrderId(((execorder*) buf)->szOrderId,
						sizeof(((execorder*) buf)->szOrderId));
				string strExecutedShares(((execorder*) buf)->szExecShares,
						sizeof(((execorder*) buf)->szExecShares));

				unordered_map<string, pair<string, long> >::iterator it_OrderIdToSymbol =
						map_OrderIdToSymbol.find(strOrderId);
				if (it_OrderIdToSymbol == map_OrderIdToSymbol.end())
					continue;
				string strSymbol = (it_OrderIdToSymbol->second).first;
				long lOrderedShares = (it_OrderIdToSymbol->second).second;
				long lExecutedShared = stol(strExecutedShares);
				map_OrderIdToExecutions[strSymbol] += lExecutedShared;
				long lBalanceShares = lOrderedShares - lExecutedShared;
				if (lBalanceShares == 0)
					map_OrderIdToSymbol.erase(strOrderId);
				else
					it_OrderIdToSymbol->second = make_pair(strSymbol, lBalanceShares);
			} catch (exception &e) {
				cout << "Caught exception when processesing ExecOrder message : " << e.what()
						<< endl;
			}
			break;

		case 'P':
			try {
				tradeorder *to;
				to = (tradeorder*) buf;
				string strSymbol(((tradeorder*) buf)->szSymbol,
						sizeof(((tradeorder*) buf)->szSymbol));
				string strTradedShares(((tradeorder*) buf)->szTradeShares,
						sizeof(((tradeorder*) buf)->szTradeShares));
				long lTradedShares = stol(strTradedShares);
				map_OrderIdToExecutions[strSymbol] += lTradedShares;
			} catch (exception &e) {
				cout << "Caught exception when processing TradeOrder message : " << e.what()
						<< endl;
			}
			break;

		case 'X':
			try {
				cancelorder *co;
				co = (cancelorder*) buf;
				string strOrderId(((cancelorder*) buf)->szOrderId,
						sizeof(((cancelorder*) buf)->szOrderId));
				string strCanceledShares(((cancelorder*) buf)->szCxlShares,
						sizeof(((cancelorder*) buf)->szCxlShares));

				unordered_map<string, pair<string, long> >::iterator it_OrderIdToSymbol =
						map_OrderIdToSymbol.find(strOrderId);

				if (it_OrderIdToSymbol == map_OrderIdToSymbol.end())
					continue;

				long lOrderedShares = (it_OrderIdToSymbol->second).second;
				long lCanceledShares = stol(strCanceledShares);
				long lBalanceShares = lOrderedShares - lCanceledShares;
				if (lBalanceShares == 0)
					map_OrderIdToSymbol.erase(strOrderId);
				else
					it_OrderIdToSymbol->second = make_pair((it_OrderIdToSymbol->second).first,
							lBalanceShares);
			} catch (exception &e) {
				cout << "Caught exception when processing CancelOrder message : " << e.what()
						<< endl;
			}
			break;

		case 'H':
			//TODO - process Trading Status msg (not required for result)
			break;
		case 'I':
			//TODO - process Auction Update msg (not required for result)
			break;
		case 'J':
			//TODO - process Auction summary msg (not required for result)
			break;
		case 'R':
			//TODO - process Retail Price Improvement msg (not required for result)
			break;
		default:
			break;
		}
	}
	sortAndPrint(map_OrderIdToExecutions);

	return 0;
}
