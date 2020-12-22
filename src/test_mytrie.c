/**
 * 
 * \file test_mytrie.c
 * \version 0.1
 * 
 * \brief 测试mytrie.c中的接口
 */

#include <check.h>

/** 测试trie树单词设置接口 */
START_TEST(test_trie_word_set)
{
    trie_st root = {NULL};
    char *word = NULL;

    word = "haha";
    trie_word_set(&root, word, 1);
    ck_assert_msg(root.nson == 1, "root.nson[%d] != 1!!!", root.nson);
    trie_debug(&root);

    word = "hehe";
    trie_word_set(&root, word, 0);
    ck_assert_msg(root.nson == 1, "root.nson[%d] != 1!!!", root.nson);
    trie_debug(&root);

    word = "dada";
    trie_word_set(&root, word, 0);
    ck_assert_msg(root.nson == 2, "root.nson[%d] != 2!!!", root.nson);
    trie_debug(&root);

    word = "xiaoxiao";
    trie_word_set(&root, word, 0);
    ck_assert_msg(root.nson == 3, "root.nson[%d] != 3!!!", root.nson);
    trie_debug(&root);

    word = "xia bi bi";
    trie_word_set(&root, word, 0);
    ck_assert_msg(root.nson == 3, "root.nson[%d] != 3!!!", root.nson);
    trie_debug(&root);

    word = "xi ";
    trie_word_set(&root, word, 1);
    ck_assert_msg(root.nson == 3, "root.nson[%d] != 3!!!", root.nson);
    trie_debug(&root);

    word = "aabb";
    trie_word_set(&root, word, 1);
    ck_assert_msg(root.nson == 4, "root.nson[%d] != 4!!!", root.nson);
    trie_debug(&root);

    trie_allson_destroy(&root);
}
END_TEST

START_TEST(test_trie_word_match)
{
    trie_st root = {NULL};
    trie_st *tnode = NULL;
    char *word = NULL;
    char *mstr = NULL;
    int ret = 0;

    word = "haha";
    trie_word_set(&root, word, 1);
    word = "hehe";
    trie_word_set(&root, word, 0);
    word = "dada";
    trie_word_set(&root, word, 0);
    word = "xiaoxiao";
    trie_word_set(&root, word, 0);
    word = "xia bi bi";
    trie_word_set(&root, word, 1);
    word = "xi ";
    trie_word_set(&root, word, 1);
    word = "aabb";
    trie_word_set(&root, word, 1);

    trie_debug(&root);

    tnode = trie_chr_find(&root, 'b');
    ck_assert_msg(tnode == NULL, "tnode node not expect");

    tnode = trie_chr_find(&root, 'a');
    ck_assert_msg(tnode != NULL, "tnode node not expect");
    ck_assert_msg(tnode->is_set == 1, "tnode is_set node not expect");

    tnode = trie_chr_find(&root, 'd');
    ck_assert_msg(tnode != NULL, "tnode node not expect");
    ck_assert_msg(tnode->is_set == 0, "tnode is_set node not expect");

    tnode = trie_chr_find(&root, 'f');
    ck_assert_msg(tnode == NULL, "tnode node not expect");
    
    tnode = trie_chr_find(&root, 'h');
    ck_assert_msg(tnode != NULL, "tnode node not expect");
    ck_assert_msg(tnode->is_set == 1, "tnode is_set node not expect");

    tnode = trie_chr_find(&root, 'x');
    ck_assert_msg(tnode != NULL, "tnode node not expect");
    ck_assert_msg(tnode->is_set == 1, "tnode is_set node not expect");

    tnode = trie_chr_find(&root, 'z');
    ck_assert_msg(tnode == NULL, "tnode node not expect");

    mstr = "xia";
    ret = trie_word_match(&root, mstr);
    ck_assert_msg(ret == 0, "ret[%d] invalid", ret);

    mstr = "xi ";
    ret = trie_word_match(&root, mstr);
    ck_assert_msg(ret == 1, "ret[%d] invalid", ret);

    trie_allson_destroy(&root);
}
END_TEST

Suite *make_suite(void)
{
    Suite *s = suite_create("xiaoxiao");
    TCase *tc = tcase_create("mytrie_c_test");

    debug_level_set(LOG_MAX);
    tcase_add_test(tc, test_trie_word_set);
    tcase_add_test(tc, test_trie_word_match);

    
    suite_add_tcase(s, tc);
    return s;
}

int main()
{
    int nf;
    Suite *s = make_suite();
    SRunner *sr = srunner_create(s);
    srunner_set_fork_status(sr, CK_NOFORK);

    srunner_run_all(sr, CK_VERBOSE);
    nf = srunner_ntests_failed(sr);
    
    srunner_free(sr);
    return nf;
}