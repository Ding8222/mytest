# mytest

使用到的数据库为mysql

需要用到的工具有：CMAKE

编译时Debug和RelWithDebInfo的选择：
Debug模式下，后台有全日志输出，RelWithDebInfo输出不全，但二者都会在bin目录下生成log日志。
RelWithDebInfo模式是带调试信息的release模式，生产环境使用该模式。
一般开发都可以只使用RelWithDebInfo模式。
一下配置需要配置debug、release、RelWithDebInfo，也可以只配置你想编译用的配置。例如只在debug模式下编译，那么其它两个的配置可以不管，不影响正常编译。

1.使用命令git submodule update --init下载第三方库

2.使用CMAKE为protobuf库生成VS项目
CMAKE目录在protobuf/cmake
生成至项目的根目录下的builds文件夹中，如果生成失败，则在CMAKE中取消勾选protobuf_BUILD_TESTS选项（额外需要其他库），重新生成

3.用vs2017打开server目录下的server.sln。
如果提示重定向lxnet，点击确定即可。

4.三方库的处理：
cryptlib:
右击属性--配置属性--常规--输出目录修改为"..\..\lib"
字符集修改为"使用Unicode字符集"
属性--配置属性--C/C++--代码生成--运行库 根据编译模式（debug、release）分别修改为（多线程调试DLL、多线程DLL）

libprotobuf：
右击属性--配置属性--常规--输出目录修改为"..\..\..\lib"
字符集修改为"使用Unicode字符集"
属性--配置属性--C/C++--代码生成--运行库 根据编译模式（debug、release、RelWithDebInfo）分别修改为（多线程调试DLL、多线程DLL、多线程DLL）

lxnet：
右击属性--配置属性--常规--输出目录修改为"..\..\..\..\lib"
字符集修改为"使用Unicode字符集"
属性--配置属性--C/C++--代码生成--运行库 根据编译模式（debug、release）分别修改为（多线程调试DLL、多线程DLL）
"配置属性--库管理器--常规--输出文件"，点击下拉按钮，选择<从父级或项目默认设置继承>。

5.双击目录tools/protobuf下的GenAllPB.bat用于生成项目使用的pb.cc等文件
GenPB.bat为差异生成（在.proto被修改后，还未提交时，使用该文件可以最小量生成pb.cc）
GenAllPB.bat为强制生成所有pb.cc文件

6.编译项目

7.将"tools/mysql"下的.sql导入到数据库中。

启动服务器：
双击"server/bin"目录下"start.bat"启动服务器；
双击"server/bin"目录下"stop.bat"关闭服务器；

机器人：
双击"server/bin"目录下"robot_start.bat"批量启动机器人，机器人进程根据bat中配置启动，每个进程中机器人数量由"server/bin/config/robotconfig.json"中的字段控制。
