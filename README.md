# mytest

使用到的数据库为mysql
需要用到的工具有：CMAKE

编译时Debug和RelWithDebInfo的选择：
Debug模式下，后台有全日志输出，RelWithDebInfo输出不全，但二者都会在bin目录下生成log日志。
RelWithDebInfo模式是带调试信息的release模式，生产环境使用该模式。
一般开发都可以只使用RelWithDebInfo模式。

1.使用命令git submodule update --init下载第三方库

2.使用CMAKE为protobuf库生成VS项目
CMAKE目录在protobuf/cmake
生成至项目的根目录下的builds文件夹中，如果生成失败，则在CMAKE中取消勾选protobuf_BUILD_TESTS选项（额外需要其他库），重新生成

3.用vs2017打开server目录下的server.sln。
如果提示重定向lxnet，点击确定即可。

4.将编译模式修改为debug-x86

5.修改目标编译模式下的运行库：
cryptlib:
属性--配置属性--C/C++--代码生成--运行库 根据编译模式（debug、release）分别修改为（多线程调试DLL、多线程DLL）

lxnet：
属性--配置属性--C/C++--代码生成--运行库 根据编译模式（debug、release）分别修改为（多线程调试DLL、多线程DLL）

6.双击目录tools/protobuf下的GenAllPB.bat用于生成项目使用的pb.cc等文件
GenPB.bat为差异生成（在.proto被修改后，还未提交时，使用该文件可以最小量生成pb.cc）
GenAllPB.bat为强制生成所有pb.cc文件

7.编译项目

8.将"tools/mysql"下的.sql导入到数据库中。

启动服务器：
双击"server/bin"目录下"start.bat"启动服务器；
双击"server/bin"目录下"stop.bat"关闭服务器；

机器人：
双击"server/bin"目录下"robot_start.bat"批量启动机器人，机器人进程根据bat中配置启动，每个进程中机器人数量由"server/bin/config/robotconfig.json"中的字段控制。
