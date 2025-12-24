# Update_logs/更新日志 请以code打开或者下载
当前对于1.5KB的文件
电脑压力 CPU进程占用380+ 线程占用~9000 I9-13900KF
因为我需要多开5~8个vs2022,副屏幕开着虚拟机进行测试在另一个电脑的运行参数以及ollama DeepSeek-R1:32B
并没有在纯实验状态测试性能,以现实生产环境测试

*AStruct Format:*
```AStruct
loaddata() ~60us
第一次getvalue() ~10us
缓存命中的getvalue ~400纳秒
所有存储类操作 ~15us
可读性展示:
《Astruct版本》
{
[title]:玩家数据
  {
  [header]:武器.剑01.基础属性
    {
    id=sword_01
    damage.physical=100
    damage.fire=50
    }
  }[]
  {
  [header]:武器.剑01.特效.暴击
    {
    type=critical
    value=0.2
    conditions.target.race=demon||目标种族为恶魔时触发
    conditions.target.health.below=0.3||目标血量低于30%时触发
    }
  }[]
};

《Json版本》
{
	"player": {
		"inventory": {
			"weapons": [
				{
					"id": "sword_01",
					"stats": {
						"damage": {
							"physical": 100,
							"fire": 50,
							"modifiers": [
								{
									"type": "critical",
									"value": 0.2,
									"conditions": {
										"target": {
											"race": "demon",
											"health": { "below": 0.3 }
										}
									}
								}
							]
						}
					}
				}
			]
		}
	}
}
```
英文名Aleactional故全名是AleacStruct简称AStruct

# "v1.0-2025Year:9Month:30Day"
第一代实现基础功能getvalue() loaddata() :::/或:::\路径转化完整路径 内联||注释

# "v2.0-2025Year:10Month:5Day",
第二代基础算法通过DeepSeek的代码优化与MIT和美科的同志们帮助协同优化达到极致性能，加入新功能getvalue()索引查找，修改file读取以更快的二进制读取，将singlesaved()使用一次性保存优化性能
并且无DOM，写入的数据在无拷贝是就已经彻底交给本地成员变量的同时被释放。而后续任何操作只针对本地变量操作与存储实现半步解耦
加入std::map cache缓存哈希算法最快可400纳秒瞬间读取
加入初步的异常处理

# "v3.0-2025Year:10Month:9Day"
第三代添加特殊存储方案，通过queue future condition_variable std::pair stomic thread实现写入整列排队进行原子写入!前台操作内存后台进行无IO锁排队写入保证数据不
混乱，通过析构函数使用future保证写入任务不会因为关闭主进程而导致存档剩余任务写入不会销毁因为他们都在另一个进程写入,存储和主进程完全彻底解耦
它是 消费者生产者模型的进化/魔改版本
加入LRU缓存策略使缓存不会过长，当长度超过指定的长度将会自动erase(cache.begin());
加入超级数组解析,进入元存储级别的DSL级存储格式,可通过强抽象存储复杂内容如虚幻5蓝图
蓝图key=@array@[@array@[父类,@array@[摄像头,x=0,y=120,z=0,r=120,距离=102],@array@[胶囊体,mesh=:::/player.one]],atk=100,血量=100/最大血量=150,@array@[hp.type=int,name.type=Fstring/string]]

# "v3.3-2025Year:10Month:11Day"
强化第三代，加入DelKey DelHeader Delkey DelTitle AppendHeader AppendTitle AddKey Fixformat CreateAStruct AStruct::parseArray AStruct::parseKey
等完整的解析方案。增加了全面的异常处理。

# "v4.0-2025Year:10Month:13Day"
加入了新的拓展库 AleaCook 它需要自行#include<AleaCook>即可
由于本质采用继承,AleaCook引用了OpenSSL而且完全解耦,AStruct主库是无依赖的,只有手动include才会加入加密库
用法AStruct as;
AleaCook cook(&as);会将as的所有权赋予cook
cook.Cook();自动将.Astruct的明文内容进行烹饪 main.Astruct(纯文本) ->(烹饪)->main.alcst
会将alcst的内容提取到内存进行内存操作
cook.UnCook()会将alcst的内容提取到内存进行内存操作
cook.Purity();会将alcst还原成.Astruct
烹饪过程是将.Astruct进行AES256加密后成为二进制的.alcst 使用现代化的OpenSSL-AES256

# "v4.2-2025Year:10Month:13Day"
加入了存档开关，在被烹饪过的二进制状态下将会强制关闭存档，当触发Cook开关就会强行将源文本替换成新的，因此破解了存档是无效的
只有析构函数活着主动烹饪才会修改新存档这期间只使用内存与存档完全解耦
在UnCook()时会通过std::move()将解密的明文存储在this->structdata字符串中,由于开关被打开,所以写入类的存档能力全部关闭,内存修改性能均为L1 L2 L3级。
前台只操作this->structdata这段内存只有析构函数会自动Cook关闭存档限制
加入了close();当as.close();触发时会自动彻底销毁内存包括缓存和其他设置！

# "v5.0-2025Year:10Month:16Day"
加入超级数组变量AList通过"<<"叠加数组然后.toAstruct()就可以转化为@array@的结构直接存储
AList同时也可以直接对接到AStruct::getArray();直接获取更规整的数组使用如AList list=AStruct::getArray(); 
同时通过list[0].go()将@array@[@array@[a,b,c]]第一个元素也就是@array@[a,b,c]而go的意思就是再次解析将这个内容总结方便使用
如"@array@[@array@[父类,@array@[摄像头,x=0,y=120,z=0,r=120,距离=102],@array@[胶囊体,mesh=:::/player.one]],atk=100,血量=100/最大血量=150,@array@[hp.type=int,name.type=Fstring/string]]"
AList list;
list = AStruct::getArray(as.getvalue());
list[0].Go()[1];在这里解析是->list[0] = @array@[父类,@array@[摄像头,x=0,y=120,z=0,r=120,距离=102]而Go指的是解析指定的@array@[摄像头,x=0,y=120,z=0,r=120,距离=102]GO()[0]则是"摄像头"
可直接使用list来创建超级数组哦list<<"a"<<"b"<<"c"; list.toArray()会自动组装@array@[a,b,c]支持嵌套
AList lists; lists<<"d"<<list<<"e";则组装的是@array@[d,@array@[a,b,c],e];
增加了全面的错误参数，会返回数字，用户可按照报错数字进行查找指定的详细报错代码

# "v5.5-2025Year:10Month:20Day"
@array@[@array@[父类,@array@[摄像头,x=0,y=120,z=0,r=120,距离=102],@array@[胶囊体,mesh=:::/player.one]],atk=100,血量=100/最大血量=150,@array@[hp.type=int,name.type=Fstring/string]]
AList支持Go的递归嵌套如 list[0].Go()[0].Go()[1].Go()[1]可直接获取mesh=:::/player.one
新增函数list[0].Parse();可以直接解析key=value这种类型。

# "v6.0-2025Year:10Month:21Day"
报错参数改成返回数值在库中的errorlist.Astruct有详细的参数建议常驻如
AStruct why;
why.loaddata("your/lib/path/errorlist.Astruct");
返回参数如 01001 可以使用 why.getvalue("error","01","01001"); 01的输入是指定报错区域快,001是代号，如果返回的是int无01这种而是1
则支持1001查询即 why.getvalue("error",1,"01001");注意需自己多加一个0如果是两位数如10则why.getvalue("error",10,"01001");
他们会返回这样的内容是以字符串输出 "输入为空请检查/cannot input empty content!"

errorlist.Astruct可通过Aleactional账户的Github获取最新的错误库或者获取他人翻译的语言进行下载,我只提供两大国际语言 中文+英文
而有可用返回值的则会返回五位数的错误代码
当返回值为空的时候无法返回则会生成名为log的文件在软件运行的目录下内容通常为
[2025:10:21-17:39:26]  //具体时间
[Error function]:main  //报错函数名
[File Path]:K:\Cplusplus_pro\AStruct\AStruct.cpp //报错文件位置
[Error line]:2285 //代码错误的行列
[Error code]:01001 //可直接查询的why.getvalue("error","01","01001");或者why.getvalue("error",1,"01001");

# "v6.2-2025Year:10Month:22Day"
将v4.0到v6.0的功能封装成为lib且强化部分能力
AList list << "a" << "b" << "c"; list.toArray()->@array@[a,b,c]此处不变
AList lists << "d" <<(list<<"a"<<"b"<<"c")<<"e"; lists.toArray->@array@[d,@array@[a,b,c],e]
用法更为抽象和强大简洁,且性能优化完成。左移运算符的加强本质上就是符合C++工程师常用的运算符比如 流 类都需要左移运算符oss file write cout等
将AES256key和密钥采用动态变化,且性能提升并且写入时采用AStruct本体的消费者生产者进化版模型进行,开放32个API
API包含了CRUD-C:as.Appendtitle();as.Appendheader();as.addkey();R:Getvalue autoparse等 U:changevalue/title/header D:delvale/key/title/header

# "v7.0beta-Startfor:2025-11-8 let it truth in 2025-11-9"
将AleaCook库解耦，AleaCook语法融入AStruct主体，加密依赖处理为.dll插件版本仍然使用AES256，dll几乎不可能被还原更安全
当AleaCook未识别到dll地址将会自动提示并且自动降级不抛出异常而是return将AList加入template版本可以AList<<常见任何类型
加密语法不变，但加密方案固定为 采用原文的总大小 - 注释内容长度 = 需加密长度+iv长度=总长度，想要获取加密原文必须得知注释内容
而想要知道注释内容就必须知道原文想要知道原文就必须破解形成死循环。用户只需输入key，且key会基于iv长度打乱
由于模板转化能力是利用的ostringstream oss因此假如你ostream& operator<<(ostream& os, const CustomType& obj) {
    return os << "Custom{" << obj.id << "," << obj.name << "}";
}也能让AList直接支持list<< CustomType{1, "test"}; 故只要你的编译器oss支持的任何数据Alist都能完成也能通过自己重载实现序列化如虚幻UOBJECT的操作(已成功)

# "v7.2:2025-11-10"
开放快捷工具
std::vector<std::string> AStruct::static_splittext(文本,"分割符");
AStruct::staitc_searchtext(原文本,目标内容);返回bool,严格筛查法包括大小写
char*/string/uit8 AStruct::static_dir();返回当前运行库的目录

# "v8.0beta:2025-11-28|{start in [2025-11-27] end in [2025-11-28] use about 30mins}"
加密功能完成,自带AES-NI,提升了抗信道攻击能力,功率计算,偏移计算的各属性能力。性能大幅提升，用法改善
采用大量的防御机制,优化了重复加密重复解密的BUG,通常建议用指针创建AleaCook实例化手动释放保证其周期!当然简单加密也可以简单使用
as.loaddata("xxx/main.Astruct");//此处必须以正常的路径为主
AleaCook();//构造函数请勿手动调用,会启动两级路由[1.项目路径,2.当前运行目录,自动赋值
AleaCook cooks(&as);//将as的使用权交给加密功能
cooks.CheckDllInfo();//会输出当前存档状态判断是否正确加载dll,可直接通过dll的cmd检测目标exe是否具备合法能力
cooks.Cook();//手动加密此时开启 仅内存操控能力 禁止存储能力
cooks.UnCook();//读取的加密内存解密后存入内存后可直接正常使用astruct的CRUD
cooks.Purity();//将当前文件还原出AStruct明文
~AleaCook();//请勿手动调用,虽然有兜底但并非所有引擎可用,析构本身就会调用,会自动保存一次加密存档!并且解除加密锁定允许编辑因此请注意周期哦

# "v9.0alpha:2025-12-19|{start in [2025-12-18] end in [2025-12-19] use about an hour}"
加入了新CRUD 两种允许用index访问的取值
getvalue("title","header",int index);
beta_getvalue("title","header",int index);
加入了[超级路径]概念，并入:::/的自动补充前缀路径，升级为::::/
如::::/config/conf.ini时会自动利用内存映射读取指向的文件拼接到缓存中以及内存中
加入了自动存储超级路径的功能，加入了重载changevalue，当使用以下函数
changevalue("title","header","key","datas","path");
changevalue("title","header",int index,"datas","path");
会将key的value设置为path并且自动附带::::/ 也就是说路径应该以 your/path/txt.txt
或者changevalue("title","header",int index/"key","datas","path",false);
加入false后将不会自动推导路径，需要你自行写入完整路径

# "v9.0:2025-12-21"
架构完整，无BUG出现，以及发现的一些小特性
通过实战，发现由于内存形态和AStruct内容一模一样，但因为CPU或者Windows10+的优化导致内存膨胀很可能远低于1.0%
通过实战，修复了一部分小逻辑

模型原理 标准修改一次
as.loaddata("xxx.Astruct");
as.changevalue("title","header","key","value");
         
		 一次内存映射->纯内存->内存内容被修改(已完成) <-结束-> 触发异步写入将需要存入的内容任务加入后台队列(无感知/无需管理) 
		 
		 总共0次拷贝，1次原子修改，1次触发异步原子后台写入

//该产品为我个人开发的第一个库，不喜勿喷！
