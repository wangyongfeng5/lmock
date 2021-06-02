#pragma once

#include <stdlib.h>
#include <stdio.h>

int mock(void *old_func, void *new_func, void *obj);

//替换一个函数，修改机器指令，用新函数替换旧函数，支持全局函数（包括第三方和系统函数）、成员函数（包括静态和虚函数）
//对于虚函数，需要额外传入一个对应类的实例，注意是随便一个实例，与被测代码没有关系
template <typename Origin, typename Mock>
int mock(Origin old_func, Mock new_func, void *obj = nullptr) {
    char addr[64];
    sprintf(addr, "%lu", old_func);
    void *old_addr = (void *)atoll(addr);
    sprintf(addr, "%lu", new_func);
    void *new_addr = (void *)atoll(addr);
    return mock(old_addr, new_addr, obj);
}

//重置所有替换过的操作，恢复原函数功能
void reset();
