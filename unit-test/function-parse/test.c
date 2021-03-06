#include <string.h>
#include "shell.h"
#include "lex.h"
#include "astree.h"
#include "libft.h"
#include "logging.h"

void print_token(void *tok)
{
	t_tok *token;

	token = (t_tok *)tok;
	printf("[%3d] "YELLOW"%s"END"\n", token->type, token->data);
}

void	print_tree(t_astree *node, int sp_num)
{
	char	*str;

	if (sp_num != 0)
		printf(" ");
	if (node == NULL)
	{
		printf(BLACK"(null)\n"END);
		return ;
	}
	if (node->type & NODE_PIPE)
		str = "PIPE";
	else if (node->type & NODE_BCKGRND)
		str = "BCKGRND";
	else if (node->type & NODE_SEQ)
		str = "SEQ";
	else if (node->type & NODE_REDIRECT_IN)
		str = "REDIRECT_IN";
	else if (node->type & NODE_REDIRECT_OUT)
		str = "REDIRECT_OUT";
	else if (node->type & NODE_REDIRECT_IN2)
		str = "REDIRECT_IN2";
	else if (node->type & NODE_REDIRECT_OUT2)
		str = "REDIRECT_OUT2";
	else if (node->type & NODE_CMDPATH)
		str = "CMDPATH";
	else if (node->type & NODE_ARGUMENT)
		str = "ARGUMENT";
	else if (node->type & NODE_REDIRECT_LIST)
		str = "REDIRECT_LIST";
	else if (node->type & NODE_REDIRECTION)
		str = "REDIRECTION";
	else
		str = BOLD RED"NOT FOUND"END;
	if (node->type & NODE_DATA)
		printf(YELLOW"%s"END"("BLUE"%s"END")\n", node->data, str);
	else
		printf(BOLD BLUE"%s\n"END, str);
	printf("%*s", sp_num, "");
	printf("├");
	print_tree(node->left, sp_num + 2);
	printf("%*s", sp_num, "");
	printf("└");
	print_tree(node->right, sp_num + 2);
}

bool	compare(t_astree *ex, t_astree *ac)
{
	if (ex == NULL && ac == NULL)
		return (true);
	if ((ex == NULL && ac != NULL)
		|| (ex != NULL && ac == NULL))
	{
		fprintf(stderr, RED"One is NULL.\n"END);
		fprintf(stderr, "expect: %p, actualy: %p\n", ex, ac);
		return (false);
	}
	if (ex->type != ac->type)
	{
		fprintf(stderr, RED"type is different.\n"END);
		fprintf(stderr, "expect: %d, actualy: %d\n", ex->type, ac->type);
		return (false);
	}
	if (!(ex->data == NULL && ac->data == NULL))
	{
		if (strcmp(ex->data, ac->data))
		{
			fprintf(stderr, RED"data is different.\n"END);
			fprintf(stderr, "expect: %s, actualy: %s\n", ex->data, ac->data);
			return (false);
		}
	}
	if (!compare(ex->left, ac->left))
		return (false);
	return (compare(ex->right, ac->right));
}

void	test(bool varbose, char *input, t_astree *expect_tree)
{
	t_lexer		*lex;
	t_astree	*res_tree;
	bool		res_flg;
	bool		is_ok;

	if (varbose)
		printf(BOLD"%s\n"END, input);
	else
		printf("%s", input);
	minishell_lexer(input, &lex);
	if (varbose)
		ft_lstiter(lex->listtok, print_token);
	res_tree = NULL;
	res_flg = parse_v2(lex, &res_tree);
	if (varbose)
	{
		printf("----- expect -----\n");
		print_tree(expect_tree, 0);
		printf("----- actualy -----\n");
		print_tree(res_tree, 0);
	}
	is_ok = compare(expect_tree, res_tree);
	lexer_free(&lex);
	res_tree = astree_delete_node(res_tree);
	expect_tree = astree_delete_node(expect_tree);
	if (res_flg && is_ok)
		printf(GREEN" ✓\n"END);
	else
	{
		printf(RED" ×\n"END);
		if (!res_flg)
			fprintf(stderr, RED"return val is different.\n"END);
	}
}

int main(void) {
	bool	varbose;

	varbose = false;
	// バッファリング無効
	setvbuf(stdout, 0, _IONBF, 0);
	setvbuf(stderr, 0, _IONBF, 0);
	/*
	 * <token list> ::= (EMPTY)
	 */
	printf("(no-length string)");
	test(varbose, "", NULL);
	printf("(white space only)");
	test(varbose, "    ", NULL);
	/*
	 * <token list>		::= <token> <token list>
	 * <simple command>	::= <pathname> <token list>
	 * <command>		::= <simple command>
	 */
	// <pathname> (EMPTY)
	test(varbose, "pwd",
		astree_create_node(NODE_CMDPATH | NODE_DATA, strdup("pwd"),
			NULL, // LEFT
			NULL)); // RIGHT
	// <pathname> <token>
	test(varbose, "echo a",
		astree_create_node(NODE_CMDPATH | NODE_DATA, strdup("echo"),
			NULL,
			astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("a"),
				NULL,
				NULL)));
	// <pathname> <token list>
	test(varbose, "token1 token2 token3 token4",
		astree_create_node(NODE_CMDPATH | NODE_DATA, strdup("token1"),
			NULL,
			astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("token2"),
				NULL,
				astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("token3"),
					NULL,
					astree_create_node(NODE_ARGUMENT | NODE_DATA,
						strdup("token4"),
						NULL,
						NULL)))));
	/*
	 * <command>			::= <simple command> <redirection list>
	 * <redirection list>	::= <redirection> <redirection list>
	 * <redirection>		::= '<' <filename> <token list>
	 */
	// <simple command> <redirection>
	test(varbose, "cat < in1",
		astree_create_node(NODE_REDIRECTION, NULL,
			astree_create_node(NODE_CMDPATH | NODE_DATA,
				strdup("cat"),
				NULL,
				NULL),
			astree_create_node(NODE_REDIRECT_LIST, NULL,
				astree_create_node(NODE_REDIRECT_IN | NODE_DATA, strdup("in1"),
					NULL,
					NULL),
				NULL)));
	test(varbose, "cat > out1",
		astree_create_node(NODE_REDIRECTION, NULL,
			astree_create_node(NODE_CMDPATH | NODE_DATA, strdup("cat"),
				NULL,
				NULL),
			astree_create_node(NODE_REDIRECT_LIST, NULL,
				astree_create_node(NODE_REDIRECT_OUT | NODE_DATA, strdup("out1"),
					NULL,
					NULL),
				NULL)));
	test(varbose, "cat >> out1",
		astree_create_node(NODE_REDIRECTION, NULL,
			astree_create_node(NODE_CMDPATH | NODE_DATA,
				strdup("cat"),
				NULL,
				NULL),
			astree_create_node(NODE_REDIRECT_LIST, NULL,
				astree_create_node(NODE_REDIRECT_OUT2 | NODE_DATA, strdup("out1"),
					NULL,
					NULL),
				NULL)));
	// // <simple command> <redirection list>
	test(varbose, "cat < in1 < in2",
		astree_create_node(NODE_REDIRECTION, NULL,
			astree_create_node(NODE_CMDPATH | NODE_DATA, strdup("cat"),
				NULL,
				NULL),
			astree_create_node(NODE_REDIRECT_LIST, NULL,
				astree_create_node(NODE_REDIRECT_IN | NODE_DATA, strdup("in1"),
					NULL,
					NULL),
				astree_create_node(NODE_REDIRECT_LIST, NULL,
						astree_create_node(NODE_REDIRECT_IN | NODE_DATA, strdup("in2"),
							NULL,
							NULL),
						NULL))));
	test(varbose, "cat > out1 > out2",
		astree_create_node(NODE_REDIRECTION, NULL,
			astree_create_node(NODE_CMDPATH | NODE_DATA, strdup("cat"),
				NULL,
				NULL),
			astree_create_node(NODE_REDIRECT_LIST, NULL,
				astree_create_node(NODE_REDIRECT_OUT | NODE_DATA, strdup("out1"),
					NULL,
					NULL),
				astree_create_node(NODE_REDIRECT_LIST, NULL,
						astree_create_node(NODE_REDIRECT_OUT | NODE_DATA, strdup("out2"),
							NULL,
							NULL),
						NULL))));
	test(varbose, "cat >> out1 >> out2",
		astree_create_node(NODE_REDIRECTION, NULL,
			astree_create_node(NODE_CMDPATH | NODE_DATA, strdup("cat"),
				NULL,
				NULL),
			astree_create_node(NODE_REDIRECT_LIST, NULL,
				astree_create_node(NODE_REDIRECT_OUT2 | NODE_DATA, strdup("out1"),
					NULL,
					NULL),
				astree_create_node(NODE_REDIRECT_LIST, NULL,
						astree_create_node(NODE_REDIRECT_OUT2 | NODE_DATA, strdup("out2"),
							NULL,
							NULL),
						NULL))));
	// // <simple command> <redirection> <token>
	test(varbose, "cat < in1 arg1",
		astree_create_node(NODE_REDIRECTION, NULL,
			astree_create_node(NODE_CMDPATH | NODE_DATA, strdup("cat"),
				NULL,
				astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg1"),
					NULL,
					NULL)),
			astree_create_node(NODE_REDIRECT_LIST, NULL,
				astree_create_node(NODE_REDIRECT_IN | NODE_DATA, strdup("in1"),
					NULL,
					NULL),
				NULL)));
	test(varbose, "cat arg1 < in1 arg2",
		astree_create_node(NODE_REDIRECTION, NULL,
			astree_create_node(NODE_CMDPATH | NODE_DATA, strdup("cat"),
				NULL,
				astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg1"),
					NULL,
					astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg2"),
						NULL,
						NULL))),
			astree_create_node(NODE_REDIRECT_LIST, NULL,
				astree_create_node(NODE_REDIRECT_IN | NODE_DATA, strdup("in1"),
					NULL,
					NULL),
				NULL)));
	// <simple command> <redirection> <token list>
	test(varbose, "cat < in1 arg1 arg2",
		astree_create_node(NODE_REDIRECTION, NULL,
			astree_create_node(NODE_CMDPATH | NODE_DATA, strdup("cat"),
				NULL,
				astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg1"),
					NULL,
					astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg2"),
						NULL,
						NULL))),
			astree_create_node(NODE_REDIRECT_LIST, NULL,
				astree_create_node(NODE_REDIRECT_IN | NODE_DATA, strdup("in1"),
					NULL,
					NULL),
				NULL)));
	test(varbose, "cat arg1 < in1 arg2 arg3",
		astree_create_node(NODE_REDIRECTION, NULL,
			astree_create_node(NODE_CMDPATH | NODE_DATA, strdup("cat"),
				NULL,
				astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg1"),
					NULL,
					astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg2"),
						NULL,
						astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg3"),
							NULL,
							NULL)))),
			astree_create_node(NODE_REDIRECT_LIST, NULL,
				astree_create_node(NODE_REDIRECT_IN | NODE_DATA, strdup("in1"),
					NULL,
					NULL),
				NULL)));
	// <simple command> <redirection list> <token>
	test(varbose, "cat < in1 < in2 arg1",
		astree_create_node(NODE_REDIRECTION, NULL,
			astree_create_node(NODE_CMDPATH | NODE_DATA, strdup("cat"),
				NULL,
				astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg1"),
					NULL,
					NULL)),
			astree_create_node(NODE_REDIRECT_LIST, NULL,
				astree_create_node(NODE_REDIRECT_IN | NODE_DATA, strdup("in1"),
					NULL,
					NULL),
				astree_create_node(NODE_REDIRECT_LIST, NULL,
						astree_create_node(NODE_REDIRECT_IN | NODE_DATA, strdup("in2"),
							NULL,
							NULL),
						NULL))));
	test(varbose, "cat arg1 < in1 < in2 arg2",
		astree_create_node(NODE_REDIRECTION, NULL,
			astree_create_node(NODE_CMDPATH | NODE_DATA, strdup("cat"),
				NULL,
				astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg1"),
					NULL,
					astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg2"),
						NULL,
						NULL))),
			astree_create_node(NODE_REDIRECT_LIST, NULL,
				astree_create_node(NODE_REDIRECT_IN | NODE_DATA, strdup("in1"),
					NULL,
					NULL),
				astree_create_node(NODE_REDIRECT_LIST, NULL,
						astree_create_node(NODE_REDIRECT_IN | NODE_DATA, strdup("in2"),
							NULL,
							NULL),
						NULL))));
	test(varbose, "cat < in1 arg1 < in2",
		astree_create_node(NODE_REDIRECTION, NULL,
			astree_create_node(NODE_CMDPATH | NODE_DATA, strdup("cat"),
				NULL,
				astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg1"),
					NULL,
					NULL)),
			astree_create_node(NODE_REDIRECT_LIST, NULL,
				astree_create_node(NODE_REDIRECT_IN | NODE_DATA, strdup("in1"),
					NULL,
					NULL),
				astree_create_node(NODE_REDIRECT_LIST, NULL,
						astree_create_node(NODE_REDIRECT_IN | NODE_DATA, strdup("in2"),
							NULL,
							NULL),
						NULL))));
	test(varbose, "cat arg1 < in1 arg2 < in2 arg3",
		astree_create_node(NODE_REDIRECTION, NULL,
			astree_create_node(NODE_CMDPATH | NODE_DATA, strdup("cat"),
				NULL,
				astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg1"),
					NULL,
					astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg2"),
						NULL,
						astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg3"),
							NULL,
							NULL)))),
			astree_create_node(NODE_REDIRECT_LIST, NULL,
				astree_create_node(NODE_REDIRECT_IN | NODE_DATA, strdup("in1"),
					NULL,
					NULL),
				astree_create_node(NODE_REDIRECT_LIST, NULL,
						astree_create_node(NODE_REDIRECT_IN | NODE_DATA, strdup("in2"),
							NULL,
							NULL),
						NULL))));
	// <simple command> <redirection list> <token list>
	test(varbose, "cat < in1 arg1 arg2 < in2",
		astree_create_node(NODE_REDIRECTION, NULL,
			astree_create_node(NODE_CMDPATH | NODE_DATA, strdup("cat"),
				NULL,
				astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg1"),
					NULL,
					astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg2"),
						NULL,
						NULL))),
			astree_create_node(NODE_REDIRECT_LIST, NULL,
				astree_create_node(NODE_REDIRECT_IN | NODE_DATA, strdup("in1"),
					NULL,
					NULL),
				astree_create_node(NODE_REDIRECT_LIST, NULL,
						astree_create_node(NODE_REDIRECT_IN | NODE_DATA, strdup("in2"),
							NULL,
							NULL),
						NULL))));
	test(varbose, "cat arg1 arg2 < in1 arg3 arg4 < in2 arg5 arg6",
		astree_create_node(NODE_REDIRECTION, NULL,
			astree_create_node(NODE_CMDPATH | NODE_DATA, strdup("cat"),
				NULL,
				astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg1"),
					NULL,
					astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg2"),
						NULL,
						astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg3"),
							NULL,
							astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg4"),
								NULL,
								astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg5"),
									NULL,
									astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg6"),
										NULL,
										NULL))))))),
			astree_create_node(NODE_REDIRECT_LIST, NULL,
				astree_create_node(NODE_REDIRECT_IN | NODE_DATA, strdup("in1"),
					NULL,
					NULL),
				astree_create_node(NODE_REDIRECT_LIST, NULL,
						astree_create_node(NODE_REDIRECT_IN | NODE_DATA, strdup("in2"),
							NULL,
							NULL),
						NULL))));
	test(varbose, "cat arg1 arg2 > out1 arg3 arg4 > out2 arg5 arg6",
		astree_create_node(NODE_REDIRECTION, NULL,
			astree_create_node(NODE_CMDPATH | NODE_DATA, strdup("cat"),
				NULL,
				astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg1"),
					NULL,
					astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg2"),
						NULL,
						astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg3"),
							NULL,
							astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg4"),
								NULL,
								astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg5"),
									NULL,
									astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg6"),
										NULL,
										NULL))))))),
			astree_create_node(NODE_REDIRECT_LIST, NULL,
				astree_create_node(NODE_REDIRECT_OUT | NODE_DATA, strdup("out1"),
					NULL,
					NULL),
				astree_create_node(NODE_REDIRECT_LIST, NULL,
						astree_create_node(NODE_REDIRECT_OUT | NODE_DATA, strdup("out2"),
							NULL,
							NULL),
						NULL))));
	test(varbose, "cat arg1 arg2 >> out1 arg3 arg4 >> out2 arg5 arg6",
		astree_create_node(NODE_REDIRECTION, NULL,
			astree_create_node(NODE_CMDPATH | NODE_DATA, strdup("cat"),
				NULL,
				astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg1"),
					NULL,
					astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg2"),
						NULL,
						astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg3"),
							NULL,
							astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg4"),
								NULL,
								astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg5"),
									NULL,
									astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg6"),
										NULL,
										NULL))))))),
			astree_create_node(NODE_REDIRECT_LIST, NULL,
				astree_create_node(NODE_REDIRECT_OUT2 | NODE_DATA, strdup("out1"),
					NULL,
					NULL),
				astree_create_node(NODE_REDIRECT_LIST, NULL,
						astree_create_node(NODE_REDIRECT_OUT2 | NODE_DATA, strdup("out2"),
							NULL,
							NULL),
						NULL))));
	// error
	// error_test(varbose, "cat < <", "syntax error near unexpected token `<'");
	// 	test("cat < out test;;"); // ';;'
	// 	test("cat < ;"); // ';'
	// 	// error
	// 	test("echo >"); // 'newline';
	// /*
	//  * <redirection> ::= '<<' <token> <token list>
	//  */
	// // test("cat << test");
	// 	// error
	// 	test("echo >>"); // 'newline';

	/*
	 * <command line> ::= <job>
	 * <job> ::= <command>
	 * <job> ::= <command> '|' <job>
	 */
	test(varbose, "echo arg1 | tr a A",
		astree_create_node(NODE_PIPE, NULL,
			astree_create_node(NODE_CMDPATH | NODE_DATA, strdup("echo"),
				NULL,
				astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("arg1"),
					NULL,
					NULL)),
			astree_create_node(NODE_CMDPATH | NODE_DATA, strdup("tr"),
					NULL,
					astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("a"),
						NULL,
						astree_create_node(NODE_ARGUMENT | NODE_DATA, strdup("A"),
							NULL,
							NULL)))));
	test(varbose, "a | b | c",
		astree_create_node(NODE_PIPE, NULL,
			astree_create_node(NODE_CMDPATH | NODE_DATA, strdup("a"),
				NULL,
				NULL),
			astree_create_node(NODE_PIPE, NULL,
				astree_create_node(NODE_CMDPATH | NODE_DATA, strdup("b"),
					NULL,
					NULL),
				astree_create_node(NODE_CMDPATH | NODE_DATA, strdup("c"),
					NULL,
					NULL))));
	// 	//error
	// 	test("a |"); // '|'
	// /*
	//  * <command line> ::= <job> ;
	//  * <command line> ::= <job> ; <command line>
	//  */
	// test(true, "a;", NULL);
	// test(true, "a; b; c" NULL);
	// 	//error
	// 	test("echo a;;"); // ';;'
	// 	test("echo a;; ;"); // ';;'
	// /*
	//  * <command line> ::= <job> & // not make
	//  * <command line> ::= <job> & <command line> // not make
	//  */
}
