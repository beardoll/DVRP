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
