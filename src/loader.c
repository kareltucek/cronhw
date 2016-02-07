#include <stdlib.h>
#include "loader.h"
#include "parse_ctx.h"
#include "task.h"
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"
#include <string.h>

/*
 * Loader contains set of functions used to load commands from a file and store them into a
 * list of task_t structures.
 */

/*
 * tree node comparator
 */
bool
rb_str_cmp(RB_NODE_T(string_tree_t)* n1, RB_NODE_T(string_tree_t)* n2)
{
	return (strcmp(n1->key, n2->key));
}

	RB_GENERATE(string_tree_t, RB_NODE_T(string_tree_t), entry, rb_str_cmp)
	RB_TREE_T_GEN(string_tree_t, char *, char *)
	SLIST_LIST_T_GEN(tasklist_t, task_t)
SLIST_LIST_T_GEN(intlist_t, int)

	/*
	 * Retrieves one line from given file descriptor and stores it into a provided pointer.
	 * This works by first seeking for a newline and then by allocating the memory and
	 * making a copy of the data
	 */
bool
getline(int fd, char ** lineptr)
{
	int fp = lseek(fd, 0, SEEK_CUR);
	char buff[BUFFSIZE];
	int nlpos = 0;
	int bytes_read;
	while (true)
	{
		bytes_read = read(fd, buff, BUFFSIZE);
		int pos = 0;
		while (pos < bytes_read && buff[pos] != '\n')
			pos++;
		nlpos += pos;
		if (bytes_read == 0 || buff[pos] == '\n')
			break;
	}
	char * line = (char *)malloc((nlpos+1)*sizeof (char)); CHECKP(line, "alloc error")
	lseek(fd, fp, SEEK_SET);
	if (read(fd, line, nlpos+1) < 0)
		stderror(STD_ERR);
	line[nlpos] = '\0';
	*lineptr = line;
	return (bytes_read > 0);
}


void
task_init(task_t * task)
{
	for (int i = 0; i < 5; i++)
	{
		SLIST_INIT(&task->times[i]);
	}
}

void
task_destroy(task_t * task)
{
	SLIST_NODE_T(intlist_t)* ptr;
	for (int i = 0; i < 5; i++)
	{
		while (!SLIST_EMPTY(&task->times[i])) {
			ptr = SLIST_FIRST(&task->times[i]);
			SLIST_REMOVE_HEAD(&task->times[i], entry);
			free(ptr);
		}
	}
	free(task->cmdline);
}

void
tasklist_clear(tasklist_t tasklist)
{
	SLIST_NODE_T(tasklist_t)* ptr;
	for (int i = 0; i < 5; i++)
	{
		while (!SLIST_EMPTY(&tasklist)) {
			ptr = SLIST_FIRST(&tasklist);
			SLIST_REMOVE_HEAD(&tasklist, entry);
			free(ptr);
		}
	}
}

/*
 * Sets a key in parser context's dictionary to value
 */
void
parse_ctx_set(parse_ctx_t * pctx, char * key, char * value)
{
	RB_NODE_T(string_tree_t) find, *res;
	find.key = key;
	res = RB_FIND(string_tree_t, &pctx->dict, &find);
	if (res != NULL)
                {
		RB_REMOVE(string_tree_t, &pctx->dict, res);
                                free(res);
                }
	RB_INSERT(string_tree_t, &pctx->dict, RB_NODE(string_tree_t, key, value));
}

/*
 * Initializes a parser context.
 */
void
init_parse_ctx(parse_ctx_t * pctx)
{
	RB_INIT(&pctx->dict);
	parse_ctx_set(pctx, strdup("$"), strdup("$"));
	parse_ctx_set(pctx, strdup("SHELL"), strdup("/bin/sh"));
	parse_ctx_set(pctx, strdup("PATH"), strdup("/usr/bin:/bin"));
	parse_ctx_set(pctx, strdup("LOGNAME"), strdup(getenv("USER")));
	parse_ctx_set(pctx, strdup("HOME"), strdup(getenv("HOME")));
}

void
clear_parse_ctx(parse_ctx_t * pctx)
{
	RB_NODE_T(string_tree_t)* ptr;
	while (!RB_EMPTY(&pctx->dict)) {
		ptr = RB_MIN(string_tree_t, &pctx->dict);
		free(ptr->key);
		free(ptr->value);
		RB_REMOVE(string_tree_t, &pctx->dict, ptr);
		free(ptr);
	}
}

bool
is_identifier_char(char c)
{
	return (c == '_' || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9'));
}

/*
 * finds a value associated to varname in pctx
 */
char *
eval(parse_ctx_t * pctx, char * varname)
{
	RB_NODE_T(string_tree_t) find, *res;
	find.key = varname;
	res = RB_FIND(string_tree_t, &pctx->dict, &find);
	if (res == NULL)
		return ("");
	else
		return (res->value);
}


/*
 * Returns length of an identifier of a variable beginning at ptr.
 * E.g. for cat returns 3
 */
int
get_inner_len(char * ptr)
{
	int len = 0;
	char * ptr2 = ptr;
	if (*ptr2 == '$')
	{
		return (1);
	}
	while (is_identifier_char(*ptr2))
	{
		len++;
		ptr2++;
	}
	return (len);
}

/*
 * Returns a copy of name of variable under ptr. E.g. for "cat" returns "cat"
 */
char *
get_inner_name(char * ptr)
{
	int len = get_inner_len(ptr);
	char * varname = malloc((len+1)*sizeof (char)); CHECKP(varname, "alloc err")
	varname[len] = '\0';
	for (int i = 0; i < len; i++)
		varname[i] = ptr[i];
	return (varname);
}

/*
 * Returns varname. E.g. for "$cat" returns "cat"
 */
char *
get_var_name(char * ptr)
{
	return (get_inner_name(ptr+1));
}

/*
 * calculates length of a line after substitutions are applied
 */
int
preprocess_line_length(parse_ctx_t * pctx, char * line)
{
	int length = strlen(line);
	for (char * ptr = line; *ptr != '\0'; ptr++)
	{
		if (*ptr == '$')
		{
			char * varname = get_var_name(ptr);
			char * varvalue = eval(pctx, varname);
			length = length - strlen(varname) + strlen(varvalue)-1;
			ptr += strlen(varname);
			free(varname);
		}
	}
	return (length);
}

/*
 * Substitutes variables for their values
 */
char *
preprocess_line(parse_ctx_t * pctx, char * line)
{
	int newlen = preprocess_line_length(pctx, line);
	char * newline = (char *)malloc((newlen+1)*sizeof (char)); CHECKP(newline, "alloc err")
	newline[newlen] = '\0';

	char * ptr = line;
	char * newptr = newline;
	while (*ptr != '\0')
	{
		if (*ptr == '$')
		{
			char * varname = get_var_name(ptr);
			char * evaluated = eval(pctx, varname);
			while (*evaluated != '\0')
			{
				*newptr = *evaluated;
				newptr++;
				evaluated++;
			}
			char * ptr2 = varname;
			while (*ptr2 != '\0')
			{
				ptr2++;
				ptr++;
			}
			free(varname);
			ptr++;
		}
		else
		{
			*newptr = *ptr;
			newptr++;
			ptr++;
		}
	}

	free(line);
	return (newline);
}

/*
 * This function parses a coma separated list of range specifiers and pushes all
 * corresponding numbers to the supplemented list. E.g. 1, 2, 5-7 will result in 1, 2, 5, 6, 7
 * being put into the list. '*' is resolved to -1
 */
void
parse_range(char ** line, intlist_t * list, char * origline)
{
	char * ptr = *line;
	while (*ptr == ' ')
		ptr++;
	int cnum = 0;
	int lnum = 0;
	bool range = false;
	while (true)
	{
		switch (*ptr)
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				cnum = cnum*10+(*ptr - 48);
				break;
			case ',':
			case ' ':
				if (range)
				{
					if (lnum > cnum)
					{
						error("warning: start of range is bigger than its end:", false, STD_ERR);
						error(origline, false, STD_ERR);
					}
					for (int i = lnum; i <= cnum; i++)
					{
						SLIST_NODE_T(intlist_t) *n = SLIST_NODE(intlist_t, i);
						SLIST_INSERT_HEAD(list, n, entry);
					}
				}
				else
				{
					SLIST_NODE_T(intlist_t) *n = SLIST_NODE(intlist_t, cnum);
					SLIST_INSERT_HEAD(list, n, entry);
				}
				range = false;
				cnum = 0;
				break;
			case '-':
				lnum = cnum;
				cnum = 0;
				range = true;
				break;
			case '*':
				cnum = -1;
				break;
			default:
				error("crontab load failed - expected *, number or a range specification on line:", false, STD_ERR);
				error(origline, true, STD_ERR);
				return;
				break;
		}
		if (*ptr == ' ')
			break;
		ptr++;
	}
	*line = ptr;
}

/*
 * Parses an entry of form "* * * * * command" into a task_t structure which is then put into the
 * supplemented list.
 */
void
parse_command(char * line, tasklist_t * list)
{
	task_t task;
	task_init(&task);
	task.cmdline = line;
	char * origline = line;
	for (int i = 0; i < 5; i++)
	{
		parse_range(&line, &task.times[i], origline);
	}
	task.cmd = line;
	SLIST_NODE_T(tasklist_t)* n = SLIST_NODE(tasklist_t, task);
	SLIST_INSERT_HEAD(list, n, entry);
}

/*
 * Reads a variable assignment and updates the parse context dictionary.
 */
void parse_assignment(parse_ctx_t * pctx, char * line) {
	int len = get_inner_len(line);
	if (line[len] != '=')
	{
		error("error: expected '=' after identifier in line: ", false, STD_ERR);
		error(line, true, STD_ERR);
		return;
	}
	char * varname = get_inner_name(line);
	char * value = get_inner_name(line+len+1);
	parse_ctx_set(pctx, varname, value);
                setenv(varname, value, true);
	free(line);
}

/*
 * Takes a line, evaluates and decides what should be done with it.
 */
void
parse_line(parse_ctx_t * pctx, char * line, tasklist_t * list)
{
	char * ptr = line;
	while (*ptr != '\0')
	{
		if (*ptr == '#')
			*ptr = '\0';
		else
			ptr++;
	}

	char * newline = preprocess_line(pctx, line);

	ptr = newline;
	while (*ptr == ' ')
		ptr++;

	if (('0' <= *ptr && *ptr <= '9') || *ptr == '*')
		parse_command(newline, list);
	else if (*ptr != '\0')
		parse_assignment(pctx, newline);
	else
		free(newline);
}





/*
 * Loads file and feeds the lines into the parse_line above.
 */
tasklist_t
load_from_file(char * filename)
{
	PRINT("loading config file\n", STD_OUT, STD_ERR)
		tasklist_t list = SLIST_HEAD_INITIALIZER(head);
	int fd = open(filename, O_RDONLY);
	if (fd < 0)
	{
		stderror(STD_ERR);
		return (list);
	}
	char * line;
	parse_ctx_t pctx;
	init_parse_ctx(&pctx);
	while (getline(fd, &line))
	{
		parse_line(&pctx, line, &list);
	}
	free(line);
	clear_parse_ctx(&pctx);
	return (list);
}
