#include <stdio.h>

#include "../../std/log/aether_log.h"

void print_arr(char* pargv[], int n) {
    for (short i = 0; i < n; i++) {
        log_info("argv%d:%s", i, pargv[i]);
    }
}

/**
 * 增强版join函数
 * 支持分隔符为字符串
 *
 * @param strs 字符串数组
 * @param sep 分隔字符串（可以为NULL表示不添加分隔符）
 * @param count 字符串数量（如果为-1，则自动检测NULL结尾）
 * @return 连接后的字符串
 */
char* join_ex(char** strs, const char* sep, int count) {
    if (strs == NULL) {
        return strdup("");
    }

    // 自动检测字符串数量
    if (count == -1) {
        count = 0;
        while (strs[count] != NULL) {
            count++;
        }
    }

    if (count == 0) {
        return strdup("");
    }

    // 计算总长度
    size_t total_length = 0;
    size_t sep_len = (sep != NULL) ? strlen(sep) : 0;

    for (int i = 0; i < count; i++) {
        total_length += strlen(strs[i]);
    }

    // 添加分隔符长度
    if (count > 1 && sep_len > 0) {
        total_length += (count - 1) * sep_len;
    }

    // 分配内存
    char* result = (char*)malloc(total_length + 1);
    if (result == NULL) {
        return NULL;
    }

    // 连接字符串
    char* ptr = result;
    for (int i = 0; i < count; i++) {
        // 复制当前字符串
        size_t len = strlen(strs[i]);
        memcpy(ptr, strs[i], len);
        ptr += len;

        // 添加分隔符
        if (i < count - 1 && sep != NULL && sep_len > 0) {
            memcpy(ptr, sep, sep_len);
            ptr += sep_len;
        }
    }

    *ptr = '\0';
    return result;
}
