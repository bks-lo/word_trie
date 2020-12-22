/**
 * 
 * \file mytrie.c
 * \version 0.1
 * 
 * \brief sql命令控制加载和匹配
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include "mytrie.h"
#include "debug.h"

/**
 *  加载模版
 *  生成模版匹配树
 *  设置匹配树标记
 */

/** 字典树结构 */
struct trie_st {
    struct trie_st **sons;      /**< 下一层trie树结构指针 */
    int nson;                   /**< 下一层trie树个数 */
    char val;                   /**< 节点对应的字符 */
    uint8_t is_word:1;          /**< 是否为一个单词 */
    uint8_t is_set:1;           /**< 是否被策略勾选 */
    uint8_t reserve:6;          /**< 保留字段 */
};

#define STR_DBGPREFIX   "|---"          /* 打印trie树节点的前缀 */
#define STR_WORDSET     " [word]"       /* 打印trie节点时 增加一个打印 */
#define MAX_DEBUG_LINE  1024            /* 打印时一行最多缓存1024字符 */

/** 根据size开辟空间，开辟失败返回ret */
#define MALLOC_RETURN(result, size, ret) do { \
    result = malloc(size); \
    if (result == NULL) { \
        write_log(LOG_ERROR, "malloc size %d failed !!!", (size));   \
        return ret; \
    }   \
    memset(result, 0, size); \
} while (0)

/**
 * \brief 递归打印trie数节点
 * 
 * \param [in]	ptrie   trie树节点
 * \param [in]	prefix  打印的前缀字符串
 * \param [in]	nbro    打印的节点是否还有下一个兄弟节点
 */
static void trie_debug_rec(trie_st *ptrie, char *prefix, int nbro)
{
    assert(prefix != NULL);
    if (ptrie == NULL) {
        return;
    }

    /* 打印当前节点 */
    char buf[MAX_DEBUG_LINE] = {0};
    int ret = 0;
    if (ptrie->val == 0)        /* 根节点 */
        ret = snprintf(buf, MAX_DEBUG_LINE, "%s", "ROOT");
    else
        ret = snprintf(buf, MAX_DEBUG_LINE, "%s%c [%d]", prefix, ptrie->val, ptrie->is_set);
    
    /* 当前节点是一个单词 */
    if (ptrie->is_word) {
        if (ret + sizeof(STR_WORDSET) < MAX_DEBUG_LINE)
            snprintf(buf+ret, MAX_DEBUG_LINE-ret, STR_WORDSET);
    }
    write_log(LOG_DEBUG, "%s", buf);

    /* 构造子节点前缀 */
    int len = strlen(prefix);
    char *tmp = NULL;
    MALLOC_RETURN(tmp, len + sizeof(STR_DBGPREFIX), );
    if (len == 0) {             /* 没有前缀直接使用宏STR_DBGPREFIX */
        snprintf(tmp, sizeof(STR_DBGPREFIX), STR_DBGPREFIX);
    } else {
        strncpy(tmp, prefix, len);
        tmp[len-1] = ' ';       /* 需要将之前的前缀最后三个字符'---'修改为空格 */
        tmp[len-2] = ' ';
        tmp[len-3] = ' ';
        if (!nbro) {
            tmp[len-4] = ' ';   /* 如果当前节点没有下一个兄弟，多回退一个字符 */
        }
        snprintf(tmp+len, sizeof(STR_DBGPREFIX), STR_DBGPREFIX);
    }
    
    /* 遍历当前节点的所有子节点，深度优先递归 */
    int i = 0;
    trie_st **tarr = ptrie->sons;
    for (; i< ptrie->nson; ++i) {
        trie_debug_rec(tarr[i], tmp, (i + 1) < ptrie->nson);
    }
    free(tmp);
}

/**
 * \brief 打印trie树
 * 
 * \param [in]	ptrie trie树节点
 */
void trie_debug(trie_st *ptrie)
{
    if (ptrie == NULL) {
        write_log(LOG_INFO, "This is a empty trie");
        return;
    }

    trie_debug_rec(ptrie, "", 0);
}

/**
 * \brief alloc一个trie节点
 * 
 * \return	trie_st* 
 */
static trie_st *trie_alloc()
{
    trie_st *tmp = NULL;
    MALLOC_RETURN(tmp, sizeof(trie_st), NULL);
    memset(tmp, sizeof(trie_st), 0);

    return tmp;
}

/**
 * \brief 扩充节点中sons字段的大小
 * 
 * \param [in]	ptrie trie树节点
 * \return	int 扩充成功，返回0；扩充失败，返回-1；
 */
static int trie_sons_realloc(trie_st *ptrie)
{
    trie_st **tmp = NULL;
    MALLOC_RETURN(tmp, (ptrie->nson + 1) * sizeof(trie_st *), -1);
    memcpy(tmp, ptrie->sons, (ptrie->nson) * sizeof(trie_st *));
    
    free(ptrie->sons);
    ptrie->sons = tmp;
    return 0;
}

/**
 * \brief 递归删除trie树节点
 * 
 * \param [int]	ptrie 
 */
static void trie_destroy_rec(trie_st **pptrie)
{
    if (*pptrie == NULL) {
        return;
    }
    
    int i = 0;
    trie_st **tarr = (*pptrie)->sons;
    if (tarr == NULL) {
        goto tdr_ret;
    }

    /* 遍历当前节点的所有子节点，深度优先递归 */
    for (i = 0; i < (*pptrie)->nson; ++i) {
        trie_destroy_rec(&(tarr[i]));
    }

tdr_ret:
    /* 释放子节点数组 */
    (*pptrie)->nson = 0;
    free((*pptrie)->sons);
    (*pptrie)->sons = NULL;

    /* 释放当前节点 */
    free(*pptrie);
    *pptrie = NULL;
    return;
}

/**
 * \brief 释放tire树，包括当前节点
 * 
 * \param [in]	ptrie trie树结构
 */
void trie_all_destroy(trie_st **pptrie)
{
    trie_destroy_rec(pptrie);
}

/**
 * \brief 释放tire树，只递归释放子节点，当前节点保留
 * 
 * \param [in]	ptrie trie树结构
 */
void trie_allson_destroy(trie_st *ptrie)
{
    if (ptrie == NULL)
        return ;
    
    int i = 0;
    trie_st **tarr = ptrie->sons;
    if (tarr == NULL) {
        goto tdr_ret;
    }

    /* 遍历当前节点的所有子节点，深度优先递归 */
    for (i = 0; i < ptrie->nson; ++i) {
        trie_destroy_rec(&(tarr[i]));
    }

tdr_ret:
    free(ptrie->sons);
    ptrie->sons = NULL;
    ptrie->nson = 0;
    return;
}

/**
 * \brief 通过二分查找，获取当前子节点中能命中chr的节点
 * 
 * \param [in]	ptrie trie树中的父节点
 * \param [in]	chr   待匹配的字符
 * \return	trie_st* 查找成功，返回子节点指针；查找失败，返回NULL；
 */
static trie_st *trie_chr_find(trie_st *ptrie, char chr)
{
    int s = 0;
    int e = ptrie->nson - 1;
    int mid = 0;
    char cmp = 0;
    trie_st **arr = ptrie->sons;
    
    /* 二分查找到对应的节点 */
    while (s <= e) {
        mid = (s + e) >> 1;
        cmp = chr - arr[mid]->val;
        if (cmp == 0)
            return arr[mid];
        

        if (cmp < 0)
            e = mid - 1; 
        else
            s = mid + 1;
    }

    return NULL;
}

/**
 * \brief 在字典树中匹配一个单词
 * 
 * \param [in]	ptrie 字典树结构
 * \param [in]	word  单词
 * \return	int 匹配成功，返回1；匹配失败，返回0;
 */
int trie_word_match(trie_st *ptrie, char *word)
{
    if (*word == '\0') {
        if (ptrie->is_set && ptrie->is_word)
            return 1;
        else
            return 0;
    }

    trie_st *son = trie_chr_find(ptrie, *word);
    if (son != NULL && son->is_set == 1) {
        return trie_word_match(son, word + 1);
    }

    return 0;
}

/**
 * \brief 给节点插入一个新的子节点，子节点的值按从小到大排队
 * 
 * \param [in]	ptrie trie树结构
 * \param [in]	son   带插入的子节点
 * \return	int 插入成功，返回0；插入失败，返回-1；
 */
static int trie_son_insert(trie_st *ptrie, trie_st *son)
{
    /* 扩充ptrie的子节点大小 */
    trie_sons_realloc(ptrie);
    
    /* 查找合适的位置 */
    int i = ptrie->nson - 1;
    trie_st **arr = ptrie->sons;
    while (i >= 0) {
        if (arr[i]->val <= son->val)
            break;
        
        arr[i + 1] = arr[i];
        --i;
    }

    /* 将子节点插入 */
    arr[i + 1] = son;
    ptrie->nson += 1;
    return 0;
}

/**
 * \brief 向trie树中设置一个单词
 * 
 * \param [in]	ptrie  trie树
 * \param [in]	word   待设置的单词
 * \param [in]	is_set 单词是否被勾选
 * \return	int 设置成功，返回0；设置失败，返回-1；
 */
static int trie_word_set(trie_st *ptrie, char *word, int is_set)
{
    if (*word == '\0') {
        ptrie->is_word = 1;
        return 0;
    }

    /* 查找有没有相同元素的子节点 */
    trie_st *son = trie_chr_find(ptrie, *word);
    if (son != NULL) {      /* 存在这样的子节点的话，递归查找下一个元素 */
        son->is_set |= is_set;
        return trie_word_set(son, word + 1, is_set);
    }

    /* 不存在这样的子节点，则构造子节点，插入到当前元素的子节点列表中 */
    son = trie_alloc();
    son->val = *word;
    son->is_set = is_set;
    trie_son_insert(ptrie, son);

    return trie_word_set(son, word + 1, is_set);
}

/**
 * \brief 将一个word字符串集合，设置到trie树中
 * 
 * \param [in]	ptrie trie树结构
 * \param [in]	wordarr word字符串集合
 * \return	int 设置成功，返回0；设置失败，返回-1；
 */
int trie_wordarr_set(trie_st *ptrie, wordset_st *wordarr)
{
    int ret = 0;
    int i = 0;
    for (; i < wordarr->num; ++i) {
        ret = trie_word_set(ptrie, wordarr->words[i], wordarr->sets[i]);
        if (ret != 0) {
            write_log(LOG_WARNING, "set word[%d][%s] failed", wordarr->sets[i],
                      wordarr->words[i]);
            return -1;
        }
    }

    return 0;
}

/* 放置在文件最后，用于单元测试 */
#ifdef UNITTEST_MYTRIE
#include "test_mytrie.c"
#endif