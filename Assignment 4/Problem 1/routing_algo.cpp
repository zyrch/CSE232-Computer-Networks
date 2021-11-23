#include "node.h"
#include <iostream>

using namespace std;

void printRT(vector<RoutingNode*> nd){
/*Print routing table entries*/
	for (int i = 0; i < nd.size(); i++) {
	  nd[i]->printTable();
	}
}

void routingAlgo(vector<RoutingNode*> nd){
  //Your code here

  int numNodes = nd.size();

  while(1) {

    bool updated = false;
    for (auto &node : nd) {
      node->sendMsg();
    }
    for (auto &node : nd) {
      updated |= node->processQueue();
    }
    if (!updated) {
      break;
    }
  }

  cout << "\n \n Routing Tables after convergence:" << '\n';
  printRT(nd);

}


void RoutingNode::recvMsg(RouteMsg msg) {

  msgQueue.push(msg);

}

bool RoutingNode::processQueue() {

  bool isTableUpdated = false;
  while(!msgQueue.empty()) {
    RouteMsg msg = msgQueue.front();
    msgQueue.pop();

    for (RoutingEntry &msgRouterEntry : msg.mytbl.tbl) {
      bool entryExists = false;
      for (RoutingEntry &myRouterEntry : mytbl.tbl) {
        if (myRouterEntry.dstip == msgRouterEntry.dstip) {
          if (myRouterEntry.cost > msgRouterEntry.cost + 1) {
            myRouterEntry.cost = msgRouterEntry.cost + 1;
            myRouterEntry.nexthop = msg.from;
            myRouterEntry.ip_interface = msg.recvip;
            isTableUpdated = true;
          }else if (myRouterEntry.cost < msgRouterEntry.cost + 1) {
            // cost is increased, only update if msg is from nexthop
            if (msg.from == myRouterEntry.nexthop) {
              myRouterEntry.cost = msgRouterEntry.cost + 1;
              isTableUpdated = true;
            }
          }
          entryExists = true;
        }
      }
      if (!entryExists) {
        RoutingEntry newEntry;
        newEntry.dstip = msgRouterEntry.dstip;
        newEntry.cost = msgRouterEntry.cost + 1;
        newEntry.nexthop = msg.from;
        newEntry.ip_interface = msg.recvip;
        mytbl.tbl.push_back(newEntry);
        isTableUpdated = true;
      }
    }
  }
  return isTableUpdated;
  //your code here
}




