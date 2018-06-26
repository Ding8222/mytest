# mytest

使用到的数据库为mysql

编译时Debug和RelWithDebInfo的选择：
Debug模式下，后台有全日志输出，RelWithDebInfo输出不全，但二者都会在bin目录下生成log日志。
RelWithDebInfo模式是带调试信息的release模式，生产环境使用该模式。
一般开发都可以只使用RelWithDebInfo模式(会快一些)。

项目使用了vld检测内存泄露，如果没有安装的话，可以全局搜索，将
#define VLD_FORCE_ENABLE
#include "vld.h"
这两行代码注释，或者选择安装vld

1.使用命令git submodule update --init下载第三方库

2.双击目录tools/protobuf下的GenAllPB.bat用于生成项目使用的pb.cc等文件
GenPB.bat为差异生成（在.proto被修改后，还未提交时，使用该文件可以最小量生成pb.cc）
GenAllPB.bat为强制生成所有pb.cc文件

3.用vs2017打开server目录下的server.sln。
如果提示重定向lxnet，点击确定即可。

4.将编译模式修改为debug-x86，编译项目

6.将"tools/mysql"下的.sql导入到数据库中。

启动服务器：
双击"server/bin"目录下"start.bat"启动服务器；
双击"server/bin"目录下"stop.bat"关闭服务器；

机器人：
双击"server/bin"目录下"robot_start.bat"批量启动机器人，机器人进程根据bat中配置启动，每个进程中机器人数量由"server/bin/config/robotconfig.json"中的字段控制。
