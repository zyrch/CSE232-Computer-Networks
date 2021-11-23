#include "node.h"
#include <iostream>

using namespace std;

void printRT(vector<RoutingNode*> nd){
/*Print routing table entries*/
	for (int i = 0; i < nd.size(); i++) {
	  nd[i]->printTable();
	}
}

void routingAlgo(vector<RoutingNode*> nd, bool changeTables=false){

  if (changeTables) {
    for (RoutingEntry &entry: nd[0]->getTable()->tbl) {
      nd[0]->updateTblEntry(entry.dstip, MAX_HOP_COUNT);
    }

    for (RoutingEntry &entry: nd[1]->getTable()->tbl) {
      if (entry.dstip == "10.0.0.1") {
        nd[1]->updateTblEntry(entry.dstip, MAX_HOP_COUNT);
      }
    }
  }

  int numNodes = nd.size();

  while(1) {
    /*Print routing table entries after each iteration */
    printRT(nd);

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
            if (msgRouterEntry.cost + 1 >= MAX_HOP_COUNT) {
              myRouterEntry.cost = MAX_HOP_COUNT;
              goto x;
            }
            myRouterEntry.cost = msgRouterEntry.cost + 1;
            myRouterEntry.nexthop = msg.from;
            myRouterEntry.ip_interface = msg.recvip;
            isTableUpdated = true;
          }else if (myRouterEntry.cost < msgRouterEntry.cost + 1) {
            // cost is increased, only update if msg is from nexthop
            if (msg.from == myRouterEntry.nexthop) {
              if (msgRouterEntry.cost + 1 >= MAX_HOP_COUNT) {
                myRouterEntry.cost = MAX_HOP_COUNT;
                goto x;
              }
              myRouterEntry.cost = msgRouterEntry.cost + 1;
              isTableUpdated = true;
            }
          }
x:
          entryExists = true;
        }
      }
      if (!entryExists) {
        RoutingEntry newEntry;
        newEntry.dstip = msgRouterEntry.dstip;
        newEntry.cost = min(msgRouterEntry.cost + 1, MAX_HOP_COUNT);
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




