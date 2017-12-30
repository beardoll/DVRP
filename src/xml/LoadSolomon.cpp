#include "LoadSolomon.h"
#include "tinystr.h"
#include "tinyxml.h"
#include<iostream>

void getFloatFromChildNode(TiXmlHandle parent, string childName, float &value) {
    // 获取XML文件中parent节点下名为childName的信息，赋值给value
    TiXmlElement* elem = parent.FirstChild(childName.c_str()).Element();
    value = (float)atof(elem->GetText());
}

bool getData(string filename, vector<Customer*> &allCustomer, 
        Customer &depot, float &capacity){
    // 读取xml内容于allCustomers, depot, capacity中
    TiXmlDocument doc(filename.c_str());     // 读入XML文件
    if(!doc.LoadFile()) return false;    // 如果无法读取文件，则返回
    TiXmlHandle hDoc(&doc);         // hDoc是&doc指向的对象
    TiXmlElement* pElem;            // 指向元素的指针
    pElem = hDoc.FirstChildElement().Element(); //指向根节点
    TiXmlHandle hRoot(pElem);       // hRoot是根节点
    Customer* customer;
    int tempINT;
    float tempFLOAT;

    // 读取x,y，它们放在network->nodes->node节点中
    TiXmlElement* nodeElem = hRoot.FirstChild("network").FirstChild("nodes").
        FirstChild("node").Element(); //当前指向了node节点
    for(nodeElem; nodeElem; nodeElem = nodeElem->NextSiblingElement()) { // 挨个读取node节点的信息
        customer = new Customer;
        TiXmlHandle node(nodeElem);  // nodeElem所指向的节点
        nodeElem->QueryIntAttribute("id", &tempINT);  //把id放到temp1中，属性值读法
        if(tempINT == 0){  // depot节点
            depot.id = tempINT;
            depot.prop = 0;
            getFloatFromChildNode(node, "cx", depot.x);
            getFloatFromChildNode(node, "cy", depot.y);
            depot.type = 'D';
            depot.serviceTime = 0;
            depot.arrivedTime = 0;
            depot.startTime = 0;
            depot.priority = 0;
        } else {  // 取货点
            customer = new Customer;
            customer->id = tempINT;  
            nodeElem->QueryIntAttribute("property", 0);  //先设所有的顾客都是static的
            customer->prop = tempINT;
            getFloatFromChildNode(node, "cx", depot.x);
            getFloatFromChildNode(node, "cy", depot.y);
            customer->type = 'P';
            customer->priority = 0;
            allCustomer.push_back(customer);
        }
    }

    // 读取其余信息
    TiXmlElement* requestElem = hRoot.FirstChild("requests").
        FirstChild("request").Element();   // 指向了request节点
    int count = 0;
    for(requestElem; requestElem; requestElem = requestElem->NextSiblingElement()) {
        // 当前顾客节点，注意不能赋值给一个新的对象，否则会调用复制构造函数
        customer = allCustomer[count];     		
        TiXmlHandle request(requestElem);  // 指针指向的对象
        TiXmlHandle twNode = request.FirstChild("tw");
        getFloatFromChildNode(twNode, "start", customer->startTime);
        getFloatFromChildNode(twNode, "end", customer->endTime);
        // quantity
        getFloatFromChildNode(request, "quantity", customer->quantity);
        // service time
        getFloatFromChildNode(request, "service_time", customer->serviceTime);
        TiXmlElement* serviceTimeElem = request.FirstChild("service_time").Element();
        count++;
    }

    // 读取capacity
    TiXmlElement* capacityElem = hRoot.FirstChild("fleet").FirstChild("vehicle_profile").
        FirstChild("capacity").Element();
    tempFLOAT = (float)atof(capacityElem->GetText());
    capacity = tempFLOAT;
    return true;
}

