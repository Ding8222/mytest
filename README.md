# mytest

使用到的数据库为mysql

需要用到的工具有：CMAKE

1.用vs2017打开server目录下的server.sln。

2.三方库的处理：
使用CMAKE为zlib和fmtlib生成.sln文件至各自根目录下的build文件

fmt：
"C/C++--常规--附加包含目录" 去掉fmt\

zlib：
将build目录下的zconf.h文件拷贝至zlib根目录下

protobuf：
右击生成protoc项目，得到protoc.exe，将该文件放至"server/lib"目录下

将libprotobuf的属性中
"C/C++--代码生成--运行库"修改为"多线程DLL"或者"多线程调试DLL"(分别对应release和debug模式)。

lxnet：
将属性中
"C/C++--代码生成--运行库"修改为"多线程DLL"或者"多线程调试DLL"(分别对应release和debug模式)。
"库管理器--常规--输出文件"清空。

4.在解决方案中3rd目录下，将所有的三方库的输出目录修改至目录./server/lib下（"..\..\..\..\lib\"）。

5.双击GenAllPB.bat用于生成项目使用的pb.cc等文件
GenPB.bat为差异生成（在.proto被修改后，还未提交时，使用该文件可以最小量生成pb.cc）
GenAllPB.bat为强制生成所有pb.cc文件

6.编译项目

7.将"tools/mysql"下的.sql导入到数据库中。

启动服务器：
双击"server/bin"目录下"start.bat"启动服务器；
双击"server/bin"目录下"stop.bat"关闭服务器；

机器人：
双击"server/bin"目录下"robot_start.bat"批量启动机器人，机器人进程根据bat中配置启动，每个进程中机器人数量由"server/bin/config/robotconfig.json"中的字段控制。

