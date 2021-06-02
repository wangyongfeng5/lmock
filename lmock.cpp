#include "lmock.h"
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

size_t PAGE_SIZE = getpagesize();

char const PREFIX[] = {'\x48', '\xb8'};
char const POSTFIX[] = {'\xff', '\xe0'};

const int INST_LEN = sizeof(PREFIX) + sizeof(void *) + sizeof(POSTFIX);

const int MAX_MOCK_NUM = 10240;

struct MockInfo {
    void *addr;
    char origin[INST_LEN];
};

static MockInfo g_mocks[MAX_MOCK_NUM];
static size_t g_mock_num = 0;

static int mock_func(void *old_func, void *new_func) {
    if (g_mock_num == MAX_MOCK_NUM) {
        return -1;
    }

    g_mocks[g_mock_num].addr = old_func;
    memcpy(g_mocks[g_mock_num].origin, (char *)old_func, INST_LEN);
    g_mock_num++;

    char *align_point = (char *)old_func - ((uint64_t)old_func % PAGE_SIZE);
    if (0 != mprotect(align_point, (char *)old_func - align_point + INST_LEN, PROT_READ | PROT_WRITE | PROT_EXEC)) {
        return -2;
    }

    memcpy(old_func, PREFIX, sizeof(PREFIX));
    memcpy((char *)old_func + sizeof(PREFIX), &new_func, sizeof(void *));
    memcpy((char *)old_func + sizeof(PREFIX) + sizeof(void *), POSTFIX, sizeof(POSTFIX));

    if (0 != mprotect(align_point, (char *)old_func - align_point + INST_LEN, PROT_READ | PROT_EXEC)) {
        return -3;
    }

    return 0;
}

static int restore_func(void *func_addr, char *origin) {
    char *align_point = (char *)func_addr - ((uint64_t)func_addr % PAGE_SIZE);
    if (0 != mprotect(align_point, (char *)func_addr - align_point + INST_LEN, PROT_READ | PROT_WRITE | PROT_EXEC)) {
        return -2;
    }

    memcpy(func_addr, origin, sizeof(MockInfo::origin));
    
    if (0 != mprotect(align_point, (char *)func_addr - align_point + INST_LEN, PROT_READ | PROT_EXEC)) {
        return -3;
    }

    return 0;
}

int mock(void *old_func, void *new_func, void *obj) {
    if (obj) {
        void **virtual_table = *(void ***)obj;
        old_func = virtual_table[(uint64_t)old_func - 1];
    }
    return mock_func(old_func, new_func);
}

void reset() {
    for (int i = g_mock_num - 1; i >= 0; i--) {
        restore_func(g_mocks[i].addr, g_mocks[i].origin);
    }
    g_mock_num = 0;
}
