# DVRP
Master branch is dynamic vehicle routing problem ...
Some important notes:
* route.size: (P-D)对的数量
* route.getAllCustomer: 返回的是路径中原有的顾客(D)节点，注意不是复制
* route.removeInvalidCustomer: 里面的posVec仅是按顺序保存顾客(D)的id，不包括商店(P)
* route中关于节点的搜索都是以**指针值**为依据，因此千万慎用赋值节点的操作，否则在原路径中会找不到。
