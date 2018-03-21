# DVRP
Master branch is dynamic vehicle routing problem ...
Some important notes:
* route.size: (P-D)对的数量
* route.getAllCustomer: 返回的是路径中原有的顾客(D)节点，注意不是复制
* route.removeInvalidCustomer: 里面的posVec仅是按顺序保存顾客(D)的id，不包括商店(P)
* route中关于节点的搜索都是以**指针值**为依据，因此千万慎用赋值节点的操作，否则在原路径中会找不到。
* 顾客的arrivedTime仅会在Car的状态改变时被重新设定，在algorithm进行search时不会改变
* 在replan时，对于choice为depot的顾客节点，其不可以被remove
* 现在refresh Arrived time时，对于current指针后面的节点，会覆盖其arriveTime。
* 规定执行algorithm时所有的车子必须是停车等待状态（如果是运行中，可以通过capture方法提取未走过的路径，并生成一辆处于等待状态的虚拟车）
* Route中增加了stand节点，用来记录最新驻点（可能不是当前驻点，但是必须和前一个驻点，当前驻点形成一条直线，换言之，当目的地变更后，驻点必须随之变更），统一将驻点变更的逻辑写在了Car.cpp的updateState中（主要的）。
* Car中增加了带currentTime的insertAfter函数，用于插入新顾客时，更新当前stand节点（可以想象一下路线变更后由直线变成折线）。
* 设置了`LATEST_SERVICE_TIME`，规定顾客必须在这之前接受服务，过了该时间后骑手下班，则不会再提供服务。如果骑手服务完最后一个顾客仍未到下班时间，则在原地待命。会在事件集中存储其在下班后departure的事件。
