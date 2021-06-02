# lmock

### 对全局函数插桩

原始函数：
``` 
int global(int a, int b) {
    return a + b;
}
```
对应的桩函数：
``` 
int fake_global(int a, int b) {
    //校验参数正确性，确定被测代码传入了正确的值
    assert(a == 3);
    assert(b == 2);
    //给一个返回值，配合被测代码走特定分支
    return a - b;
}
```
 插桩示例：
``` 

assert(global(3, 2) == 5);

//通过mock调用，完成函数动态替换
assert(0 == mock(&global, &fake_global));

//调用mock后的函数，可以看到返回值变了
assert(global(3, 2) == 1);

//结束mock
reset();

//函数行为恢复
assert(global(3, 2) == 5);
```

### 对普通成员函数插桩

被测代码：
``` 
class A {
public:
    int member(int a) {return ++a;}
    static int static_member(int a) {return 200;}
    virtual int virtual_member() {return 400;}
};
```
桩函数：
``` 
int fake_member(A *pTihs, int a) {
		//由于是对成员函数插桩,这里需要这个this指针参数
    return --a;
}
```
插桩示例：
``` 

A a;
assert(a.member(100) == 101);

mock(&A::member, fake_member);
assert(a.member(100) == 99);

reset();

assert(a.member(100) == 101);
```

### 对静态成员函数插桩

桩函数：
``` 
int fake_static_member() {
	 //静态函数不需要this指针
    return 300;
}
```
插桩示例：
``` 
assert(A::static_member(200) == 200);

mock(&A::static_member, fake_static_member);
assert(A::static_member(100) == 300);

reset();

assert(A::static_member(200) == 200);
```

### 对虚函数插桩

桩函数：
``` 
int fake_virtual_member(A *pThis) {
    //虚函数同普通的成员函数由于,同样需要this指针
    return 500;
}
```
插桩示例：
``` 
A a;
assert(a.virtual_member() == 400);

//虚函数mock需要多传一个相关类的对象，任意一个对象即可，跟实际代码中的对象没有关系
A a_obj;
mock(&A::virtual_member, fake_virtual_member, &a_obj);
assert(a.virtual_member() == 500);

reset();
assert(a.virtual_member() == 400);
``` 

### 对系统及第三方库函数插桩

桩函数：
```
int fake_write(int, char*, int) {
    return 100;
}
```
插桩示例：
``` 
//直接写入一个无效的文件描述符，会失败
assert(write(5, "hello", 5) == -1);

//来一个假的wirte
mock(write, fake_write);
//模拟调用成功
assert(write(5, "hello", 5) == 100);

reset();

assert(write(5, "hello", 5) == -1);
```
可以看到，对系统函数的mock,其实跟普通的全局函数并无两样，第三方库函数也是同理。

# 使用限制&注意事项
* 目前仅支持Linux + X86_64平台，如有需求，Windows和其它硬件平台，如X86_32、ARM。
* 显然，这种方法对内联函数无效，不过对于单元测试来说，可以关闭内联，同时也建议关闭其它编译器优化。
* 可以使用-fno-access-control编译你的测试代码，可以使g++关闭c++成员的访问控制（即protected及private不再生效）。

# 项目地址
