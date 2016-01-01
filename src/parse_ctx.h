#ifndef PARSE_CTX_GUARD
#define PARSE_CTX_GUARD

#include <stdlib.h>
#include "../third-party-src/tree.h"
#include "common.h"

/**
 * Parse_ctx provides a context for parsing of cron file. For now
 * the context contains only a dictionary of variable values, 
 * relized by a red black tree. Beside that there are some macros 
 * which seem to have been forgotten in the original tree library.
 * These construct a complete node type out of the entry macros 
 * consisting of the data structure and a constructor.
 * */

#define RB_NODE_T_GEN(name, typek, typev) \
	typedef struct name { \
		typek key; \
		typev value; \
		RB_ENTRY(name) entry; \
	} name; 

#define RB_NODE_T_GENC(name, typek, typev) \
	name* name##_RB_NODE_CONSTRUCTOR(typek d, typev e) \
	{ \
		name* node = (name*)malloc(sizeof(name)); \
		node->key = d; \
		node->value = e; \
		return node; \
	}

#define RB_TREE_T(name, typek, typev) \
	RB_NODE_T_GEN(name##_node_t,	typek, typev) \
	typedef RB_HEAD(name, name##_node_t) name;

#define RB_TREE_T_GEN(name, typek, typev) \
	RB_NODE_T_GENC(name##_node_t,	typek, typev) 

#define RB_NODE_T(name) \
	name##_node_t

#define RB_NODE(name, key, value) \
	name##_node_t_RB_NODE_CONSTRUCTOR(key, value)

RB_TREE_T(string_tree_t, char*, char*)

typedef struct	parse_ctx_t
{
	string_tree_t dict; 
} parse_ctx_t;

#endif
