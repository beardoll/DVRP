使用了cmake来生成makefile

cmake中使用了“外部构建”的方法，其中

所有的中间文件和最后的可执行文件都放在VRPTW/build目录下

在VRPTW/下构建了一个CMakeLists

而在VPRTW/src下也构建了一个CMakeLists

同时在VRPTW/src下，包含了所有的.h和.cpp文件

VRPTW/src下的ALNS_Result, outputtxt以及solomon-1987-rc1都是输入输出文件存放的地方，暂时应该先不用管

在main.cpp下，定义了读入的xml文件地址，xml文件就存放在solomon-1987-rc1下

在TxtRecorder.cpp中，定义了输出的txt文档地址，也就是outputtxt文件夹

代码在VS下调试通过并且成功运行，没有逻辑上的问题。