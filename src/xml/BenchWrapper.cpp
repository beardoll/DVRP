#include "BenchWrapper.h"
#include "../run/Config.h"
#include<sstream>
#include<stdexcept>

void BenchWrapper::insertFloatToNode(TiXmlElement *element, float data) {
    // 将浮点型数据装入xml节点"element"中
    ostringstream buffer;
    string ss;
    buffer << data;
    ss = buffer.str();
    element->LinkEndChild(new TiXmlText(ss.c_str()));
    buffer.str("");
}

TiXmlElement* BenchWrapper::createNode(string name, float data){
    // 创建XML节点，并且写入data
    TiXmlElement *elem = new TiXmlElement(name.c_str());
    insertFloatToNode(elem, data);
    return elem;
}

void BenchWrapper::saveSpotInfo(vector<Spot*> customers, TiXmlElement *root) {
    // 将customers数据存为XML格式
    // Args:
    //   customers: Spot类型的指针数组
    //   root: 上级节点，比如<Customer>
    vector<Spot*>::iterator custIter;
    TiXmlElement *elem;

    for (custIter=customers.begin(); custIter<customers.end(); custIter++) {
        // 依次创建顾客节点
        elem = new TiXmlElement("Node");
        elem->SetAttribute("id", (*custIter)->id);
        TiXmlElement* cx = createNode("cx", (*custIter)->x);
        TiXmlElement* cy = createNode("cy", (*custIter)->y);
        elem->LinkEndChild(cx);
        elem->LinkEndChild(cy);
        switch((*custIter)->type) { // 属性值是string类型而非ASCII码类型
            case 'D':
                elem->SetAttribute("type", "D");
                break;
            case 'S':
                elem->SetAttribute("type", "S");
                TiXmlElement* serviceTimeElem = createNode("serviceTime", (*custIter)->serviceTime);
                elem->LinkEndChild(serviceTimeElem);
                if(elem->choice != NULL) {
                    TiXmlElement* choiceElem = createNode("choice", (*custIter)->choice->id);
                    elem->LinkEndChild(choiceElem);
                }
                break;
            case 'C':
                elem->SetAttribute("type", "C");
                TiXmlElement* quantityElem = createNode("quantity", (*custIter)->quantity);
                TiXmlElement* startTimeElem = createNode("startTime", (*custIter)->startTime);
                TiXmlElement* endTimeElem = createNode("endTime", (*custIter)->endTime);
                TiXmlElement* arrivedTimeElem = createNode("arrivedTime", (*custIter)->arrivedTime);
                TiXmlElement* tolerantTimeElem = createNode("tolerantTime", (*custIter)->tolerantTime);
                TiXmlElement* serviceTimeElem = createNode("serviceTime", (*custIter)->serviceTime);
                TiXmlElement* choiceElem = createNode("choice", (*custIter)->choice->id);
                // 将所有节点都挂载到单个顾客节点node上
                elem->LinkEndChild(quantityElem);
                elem->LinkEndChild(startTimeElem);
                elem->LinkEndChild(endTimeElem);
                elem->LinkEndChild(arrivedTimeElem);
                elem->LinkEndChild(tolerantTimeElem);
                elem->LinkEndChild(serviceTimeElem);
                elem->LinkEndChild(choiceElem);
                break;
        }
        // 将elem节点挂在到root节点上
        root->LinkEndChild(elem);
    }
}

void BenchWrapper::saveBench(string path, vector<Spot*> staticCustomer, 
        vector<Spot*> dynamicCustomer, vector<Spot*> store, Spot depot, 
        float capacity) {
    // 将生成的数据集存放于xml文件中
    // Args:
    //   path: 相对路径，生成的文件名
    //   staticCustomer: 静态顾客集合（指针数组）
    //   dynamicCustomer: 动态顾客集合（指针数组）
    //   Store: 商铺
    //   depot: 仓库
    //   capacity: 车载量
    TiXmlDocument doc;
    TiXmlComment *comment;
    TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "", "");
    doc.LinkEndChild(decl);

    // 建立根节点
    TiXmlElement *root = new TiXmlElement("Result");
    doc.LinkEndChild(root);

    TiXmlElement *staticElem = new TiXmlElement("staticCustomer");
    saveSpotInfo(staticCustomer, staticElem);
    TiXmlElement *dynamicElem = new TiXmlElement("dynamicCustomer");
    saveSpotInfo(dynamicCustomer, dynamicElem);
    root->LinkEndChild(staticElem);
    root->LinkEndChild(dynamicElem);

    TiXmlElement *storeElem = new TiXmlElement("store");
    saveSpotInfo(store, storeElem);
    root->LinkEndChild(storeElem);

    TiXmlElement *depotElem = new TiXmlElement("depot");
    depotElem->SetAttribute("id", depot.id);
    depotElem->SetAttribute("type", "D");
    TiXmlElement *cx = createNode("cx", depot.x);
    TiXmlElement *cy = createNode("cy", depot.y);
    depotElem->LinkEndChild(cx);
    depotElem->LinkEndChild(cy);
    root->LinkEndChild(depotElem);

    TiXmlElement *vehicleNode = new TiXmlElement("vehicle");
    TiXmlElement *capacityNode = createNode("capacity", capacity);
    vehicleNode->LinkEndChild(capacityNode);
    root->LinkEndChild(vehicleNode);

    doc.SaveFile(path.c_str());
    cout << "Saved new benchmark to " << path << endl;
}

void BenchWrapper::saveResult(string fileName, vector<Car*> carSet, vector<Spot*> rejectCustomers, 
        vector<Spot*> dynamicCustomers, Spot depot, float travelLen, float extra) {
    // 将实验结果存放于fileName为名的XML文件中
    TiXmlDocument doc;
    TiXmlComment *comment;
    TiXmlDeclaration *dec1 = new TiXmlDeclaration("1.0", "", "");
    doc.LinkEndChild(dec1);

    // 建立根节点
    TiXmlElement *root = new TiXmlElement("Result");
    doc.LinkEndChild(root);
    
    // 添加注释
    comment = new TiXmlComment();
    string s = "ALNS Result for dataset " + fileName;
    comment->SetValue(s.c_str());
    root->LinkEndChild(comment);

    // RouteSet节点    
    TiXmlElement *routeSetElem = new TiXmlElement("RouteSet");
    root->LinkEndChild(routeSetElem);
    TiXmlElement *routeElem;
    vector<Car*>::iterator carIter;
    for(carIter=carSet.begin(); carIter<carSet.end(); carIter++) {
        // 写入Route节点，为RouteSetElem的子节点
        routeElem = new TiXmlElement("Route");
        routeElem->SetAttribute("index", (*carIter)->getCarIndex());
        vector<Spot*> tempCust = (*carIter)->getRoute().getAllCustomer();
        // tempCust首尾均不含depot节点，因此在下面添加depot节点
        // 这里考虑到销毁tempCust时会对里面所有指针的实例进行销毁，因此
        // 添加两个depot节点的复制品，他们拥有不同的地址
        Spot* newdepot1 = new Customer(depot);
        Spot* newdepot2 = new Customer(depot);
        vector<Spot*>::iterator custIter = tempCust.begin();
        tempCust.insert(custIter, newdepot1);
        tempCust.push_back(newdepot2);
        // 然后依次将tempCust中的节点信息写入XML文件中
        saveSpotInfo(tempCust, routeElem);
        routeSetElem->LinkEndChild(routeElem);
    }

    // routeLen节点
    TiXmlElement *travelLenElem = createNode("travelLen", travelLen);
    root->LinkEndChild(travelLenElem);

    // extra数据
    TiXmlElement *addAveDistanceElem = createNode("addAveDistance", extra);
    root->LinkEndChild(addAveDistanceElem);

    // 未能接受服务的顾客
    TiXmlElement *rejectCustomerElem = new TiXmlElement("rejectCustomer");
    saveSpotInfo(rejectCustomers, rejectCustomerElem);
    root->LinkEndChild(rejectCustomerElem);

    // 动态顾客
    TiXmlElement *dynamicCustomerElem = new TiXmlElement("dynamicCustomer");
    saveSpotInfo(dynamicCustomers, dynamicCustomerElem);
    root->LinkEndChild(dynamicCustomerElem);

    doc.SaveFile(fileName.c_str());
}

void BenchWrapper::getFloatFromChildNode(TiXmlHandle parent, string childName, float &value) {
    // 获取XML文件中parent节点下名为childName的信息，赋值给value
    TiXmlElement* elem = parent.FirstChild(childName.c_str()).Element();
    value = (float)atof(elem->GetText());
} 

void BenchWrapper::getFloatArrayFromChildNode(TiXmlHandle parent, string childName, float *array) {
    // 获取XML文件中parent节点下名为childName（多个）节点存放的float数组信息
    TiXmlElement *childElem = parent.FirstChild(childName.c_str()).Element();
    int c = 0;
    for (childElem; childElem; childElem = childElem->NextSiblingElement()) {
        array[c++] = (float)atof(childElem->GetText());
    }
}

void BenchWrapper::loadStoreInfo(vector<Spot*> &stores, TiXmlElement *nodeElem) {
    // nodeElem指向第一个存放store信息的XML节点
    for(nodeElem; nodeElem; nodeElem=nodeElem->NextSiblingElement()) {
        Spot *store = new Spot();
        // 读取属性值
        nodeElem->QueryIntAttribute("id", &store->id);
        nodeElem->QueryIntAttribute("property", &store->prop);
        store->type = 'S';
        store->priority = 0;
        // 读取nodeElem下的各个节点值
        TiXmlHandle node(nodeElem);
        getFloatFromChildNode(node, "cx", store->x);
        getFloatFromChildNode(node, "cy", store->y);
        getFloatFromChildNode(node, "quantity", store->quantity);
        getFloatFromChildNode(node, "startTime", store->startTime);
        getFloatFromChildNode(node, "endTime", store->endTime);
        getFloatFromChildNode(node, "arrivedTime", store->arrivedTime);
        getFloatFromChildNode(node, "tolerantTime", store->tolerantTime);
        getFloatFromChildNode(node, "serviceTime", store->serviceTime);
        stores.push_back(store);
    }
}

void BenchWrapper::loadCustomerInfo(vector<Spot*> &customers, vector<Spot*> stores,
        TiXmlElement *nodeElem) {
    // 读取XML中的customer信息
    // nodeElem指向第一个存放顾客信息的XML节点
    // 先对stores进行排序(按id)
    sort(stores.begin(), stores.end());
    int beginID = stores[0]->id;  // stores ID的最小值
    for(nodeElem; nodeElem; nodeElem=nodeElem->NextSiblingElement()) {
        // 挨个读取节点信息，并且存入到Customer结构体中
        Spot *customer = new Spot();
        // 读取nodeElem属性值
        nodeElem->QueryIntAttribute("id", &customer->id);
        nodeElem->QueryIntAttribute("property", &customer->prop);
        customer->type = 'C';
        customer->priority = 0;
        // 读取nodeElem下辖的各个节点值
        TiXmlHandle node(nodeElem); // nodeElem所指向的节点
        getFloatFromChildNode(node, "cx", customer->x);
        getFloatFromChildNode(node, "cy", customer->y);
        getFloatFromChildNode(node, "quantity", customer->quantity);
        getFloatFromChildNode(node, "startTime", customer->startTime);
        getFloatFromChildNode(node, "endTime", customer->endTime);
        getFloatFromChildNode(node, "arrivedTime", customer->arrivedTime);
        getFloatFromChildNode(node, "tolerantTime", customer->tolerantTime);
        getFloatFromChildNode(node, "serviceTime", customer->serviceTime);
        float temp;
        getFloatFromChildNode(node, "choice", temp);
        int choice = (int)temp;
        Spot *store = new Spot(**stores[choice-beginID]);
        customer->choice = store;
        store->choice = customer;
        customers.push_back(customer);
    }
}

void BenchWrapper::loadBench(string fileName, vector<Spot*> &staticCustomers, vector<Spot*> &dynamicCustomers,
        vector<Spot*> stores, Spot &depot, float &capacity) {
    // 读取数据集中的数据到相应的实体中
    // 首先，加载XML文件
    TiXmlDocument doc(fileName.c_str());
    if (!doc.LoadFile()) { 
        // 如果无法读取文件，则抛出异常
        throw out_of_range("Cannot load the benchmark!!");
    }
    TiXmlHandle hDoc(&doc);      // hDoc是&doc指向的对象
    TiXmlElement* pElem;         // 指向根节点的指针
    pElem = hDoc.FirstChildElement().Element();
    TiXmlHandle hRoot(pElem);    // hRoot是根节点
    
    // 获取商店信息
    TiXmlElement* storeElem = hRoot.FirstChild("store").FirstChild("Node").Element();
    loadStoreInfo(stores, storeElem);

    // 获取存放顾客信息的第一个XML节点（一般是"cx"）
    TiXmlElement* staticCustomerElem = hRoot.FirstChild("staticCustomer").FirstChild("Node").Element();
    loadCustomerInfo(staticCustomers, stores, staticCustomerElem);

    TiXmlElement* dynamicCustomerElem = hRoot.FirstChild("dynamicCustomer").FirstChild("Node").Element();
    loadCustomerInfo(dynamicCustomers, stores, dynamicCustomerElem);

    // 仓库信息
    TiXmlElement* depotElem = hRoot.FirstChild("depot").Element();
    depotElem->QueryIntAttribute("id", &depot.id);
    TiXmlHandle depotNode(depotElem);
    getFloatFromChildNode(depotNode, "cx", depot.x);
    getFloatFromChildNode(depotNode, "cy", depot.y);
    depot.type = 'D';
    depot.priority = 0;
    depot.startTime = 0;
    depot.arrivedTime = 0;
    depot.prop = 0;
    
    // 车辆信息
    TiXmlElement* vehicleElem = hRoot.FirstChild("vehicle").Element();
    TiXmlHandle vehicleNode(vehicleElem);
    getFloatFromChildNode(vehicleNode, "capacity", capacity);
}


