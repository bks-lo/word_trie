/**
 * 
 * \file mytrie.h
 * \version 0.1
 * 
 * \brief sql命令控制加载和匹配头文件
 */

/* trie树结构 */
typedef struct trie_st trie_st;

/* 单词和对应设置标记结构体 */
typedef struct wordset_st
{
    char **words;   /**< 单词数组 和 sets 一一对应*/
    int *sets;      /**< 设置标记数组 和 words 一一对应 */
    int num;        /**< 数组中元素个数 */
} wordset_st;

/**
 * \brief 打印trie树
 * 
 * \param [in]	ptrie trie树节点
 */
void trie_debug(trie_st *ptrie);

/**
 * \brief 释放tire树，包括当前节点
 * 
 * \param [in]	ptrie trie树结构
 */
void trie_all_destroy(trie_st **pptrie);

/**
 * \brief 释放tire树，只递归释放子节点，当前节点保留
 * 
 * \param [in]	ptrie trie树结构
 */
void trie_allson_destroy(trie_st *ptrie);

/**
 * \brief 在字典树中匹配一个单词
 * 
 * \param [in]	ptrie 字典树结构
 * \param [in]	word  单词
 * \return	int 匹配成功，返回1；匹配失败，返回0;
 */
int trie_word_match(trie_st *ptrie, char *word);

/**
 * \brief 将一个word字符串集合，设置到trie树中
 * 
 * \param [in]	ptrie trie树结构
 * \param [in]	wordarr word字符串集合
 * \return	int 设置成功，返回0；设置失败，返回-1；
 */
int trie_wordarr_set(trie_st *ptrie, wordset_st *wordarr);
