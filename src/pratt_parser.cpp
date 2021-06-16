#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// NOTE: See `https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html`.

typedef uint8_t  u8;
typedef uint32_t u32;
typedef size_t   usize;

typedef int32_t i32;

#define null nullptr

#define CAP_NODES (1 << 5)
#define CAP_LISTS (1 << 5)

struct String {
    const char* buffer;
    u32         len;
};

template <typename T>
struct List {
    T*    item;
    List* next;
};

enum TokenTag {
    TOKEN_IDENT = 0,
    TOKEN_STR,
    TOKEN_U32,
    TOKEN_ADD,
    TOKEN_SUB,
    TOKEN_MUL,
    TOKEN_DIV,
    TOKEN_COMMA,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
};

union TokenBody {
    String as_str;
    u32    as_u32;
};

struct Token {
    TokenBody body;
    TokenTag  tag;
};

enum NodeTag {
    NODE_IDENT = 0,
    NODE_STR,
    NODE_U32,
    NODE_CALL,
    NODE_UNOP,
    NODE_BINOP,
};

enum Op {
    OP_ADD = 0,
    OP_SUB,
    OP_MUL,
    OP_DIV,
};

struct Node;

struct Call {
    Node*       node;
    List<Node>* first;
    List<Node>* last;
};

struct UnOp {
    Node* node;
    Op    op;
};

struct BinOp {
    Node* nodes[2];
    Op    op;
};

union NodeBody {
    String as_str;
    u32    as_u32;
    Call   as_call;
    UnOp   as_unop;
    BinOp  as_binop;
};

struct Node {
    NodeBody body;
    NodeTag  tag;
};

struct Memory {
    Node       nodes[CAP_NODES];
    u32        len_nodes;
    List<Node> lists[CAP_LISTS];
    u32        len_lists;
    u32        cur_tokens;
};

#define TO_STR(literal)      \
    ((String){               \
        literal,             \
        sizeof(literal) - 1, \
    })

static const Token TOKENS[] = {
    {.tag = TOKEN_SUB},
    {.body = {.as_str = TO_STR("fn_0")}, .tag = TOKEN_IDENT},
    {.tag = TOKEN_LPAREN},
    {.tag = TOKEN_LPAREN},
    {.body = {.as_u32 = 123}, .tag = TOKEN_U32},
    {.tag = TOKEN_DIV},
    {.body = {.as_u32 = 45}, .tag = TOKEN_U32},
    {.tag = TOKEN_MUL},
    {.tag = TOKEN_SUB},
    {.body = {.as_u32 = 6789}, .tag = TOKEN_U32},
    {.tag = TOKEN_ADD},
    {.tag = TOKEN_SUB},
    {.body = {.as_str = TO_STR("fn_1")}, .tag = TOKEN_IDENT},
    {.tag = TOKEN_LPAREN},
    {.body = {.as_str = TO_STR("abc")}, .tag = TOKEN_IDENT},
    {.tag = TOKEN_MUL},
    {.tag = TOKEN_SUB},
    {.body = {.as_str = TO_STR("def")}, .tag = TOKEN_IDENT},
    {.tag = TOKEN_ADD},
    {.body = {.as_str = TO_STR("ghi")}, .tag = TOKEN_IDENT},
    {.tag = TOKEN_COMMA},
    {.body = {.as_str = TO_STR("xyz")}, .tag = TOKEN_IDENT},
    {.tag = TOKEN_COMMA},
    {.body = {.as_str = TO_STR("st")}, .tag = TOKEN_IDENT},
    {.tag = TOKEN_RPAREN},
    {.tag = TOKEN_RPAREN},
    {.tag = TOKEN_RPAREN},
};

#define COUNT_TOKENS ((sizeof(TOKENS) / sizeof(TOKENS[0])))

#define ERROR()                                                      \
    {                                                                \
        fprintf(stderr, "%s:%s:%d\n", __FILE__, __func__, __LINE__); \
        exit(EXIT_FAILURE);                                          \
    }

#define EXIT_IF(condition)           \
    if (condition) {                 \
        fprintf(stderr,              \
                "%s:%s:%d \"%s\"\n", \
                __FILE__,            \
                __func__,            \
                __LINE__,            \
                #condition);         \
        exit(EXIT_FAILURE);          \
    }

static void print(String string) {
    printf("%*s", string.len, string.buffer);
}

static void print(Token token) {
    switch (token.tag) {
    case TOKEN_IDENT: {
        print(token.body.as_str);
        break;
    }
    case TOKEN_STR: {
        printf("\"");
        print(token.body.as_str);
        printf("\"");
        break;
    }
    case TOKEN_U32: {
        printf("%u", token.body.as_u32);
        break;
    }
    case TOKEN_ADD: {
        printf("+");
        break;
    }
    case TOKEN_SUB: {
        printf("-");
        break;
    }
    case TOKEN_MUL: {
        printf("*");
        break;
    }
    case TOKEN_DIV: {
        printf("/");
        break;
    }
    case TOKEN_COMMA: {
        printf(",");
        break;
    }
    case TOKEN_LPAREN: {
        printf("(");
        break;
    }
    case TOKEN_RPAREN: {
        printf(")");
        break;
    }
    default: {
        ERROR();
    }
    }
}

template <typename T, usize N>
static void println(const T array[N]) {
    for (u32 i = 0; i < N; ++i) {
        printf(" ");
        print(array[i]);
    }
    printf("\n");
}

static Node* alloc_node(Memory* memory) {
    EXIT_IF(CAP_NODES <= memory->len_nodes);
    return &memory->nodes[memory->len_nodes++];
}

static List<Node>* alloc_list(Memory* memory) {
    EXIT_IF(CAP_LISTS <= memory->len_lists);
    return &memory->lists[memory->len_lists++];
}

#define TOKENS_EMPTY(memory) (COUNT_TOKENS <= memory->cur_tokens)

static const Token* pop_token(Memory* memory) {
    if (TOKENS_EMPTY(memory)) {
        return null;
    }
    return &TOKENS[memory->cur_tokens++];
}

static const Token* peek_token(Memory* memory) {
    if (TOKENS_EMPTY(memory)) {
        return null;
    }
    return &TOKENS[memory->cur_tokens];
}

#define SET_BINOP(memory, left_node, op_, binding_l, binding_r) \
    {                                                           \
        if (binding_l < prev_binding) {                         \
            return left_node;                                   \
        }                                                       \
        pop_token(memory);                                      \
        Node* binop_node = alloc_node(memory);                  \
        Node* right_node = parse(memory, binding_r);            \
        EXIT_IF(!right_node);                                   \
        binop_node->body.as_binop.nodes[0] = left_node;         \
        binop_node->body.as_binop.nodes[1] = right_node;        \
        binop_node->body.as_binop.op = op_;                     \
        binop_node->tag = NODE_BINOP;                           \
        left_node = binop_node;                                 \
        break;                                                  \
    }

#define BINDING_INFIX_ADD_SUB_L 1
#define BINDING_INFIX_ADD_SUB_R 2
#define BINDING_INFIX_MUL_DIV_L 3
#define BINDING_INFIX_MUL_DIV_R 4
#define BINDING_PREFIX_SUB      5
#define BINDING_CALL_LPAREN     6

static Node* parse(Memory* memory, u8 prev_binding) {
    if (TOKENS_EMPTY(memory)) {
        return null;
    }
    Node*        left_node;
    const Token* token = pop_token(memory);
    EXIT_IF(!token);
    switch (token->tag) {
    case TOKEN_IDENT: {
        left_node = alloc_node(memory);
        left_node->body.as_str = token->body.as_str;
        left_node->tag = NODE_IDENT;
        break;
    }
    case TOKEN_STR: {
        left_node = alloc_node(memory);
        left_node->body.as_str = token->body.as_str;
        left_node->tag = NODE_STR;
        break;
    }
    case TOKEN_U32: {
        left_node = alloc_node(memory);
        left_node->body.as_u32 = token->body.as_u32;
        left_node->tag = NODE_U32;
        break;
    }
    case TOKEN_SUB: {
        left_node = alloc_node(memory);
        Node* inner_node = parse(memory, BINDING_PREFIX_SUB);
        EXIT_IF(!inner_node);
        left_node->body.as_unop.node = inner_node;
        left_node->body.as_unop.op = OP_SUB;
        left_node->tag = NODE_UNOP;
        break;
    }
    case TOKEN_LPAREN: {
        left_node = parse(memory, 0);
        EXIT_IF(!left_node);
        token = pop_token(memory);
        EXIT_IF(token->tag != TOKEN_RPAREN);
        break;
    }
    case TOKEN_ADD:
    case TOKEN_MUL:
    case TOKEN_DIV:
    case TOKEN_COMMA:
    case TOKEN_RPAREN:
    default: {
        ERROR();
    }
    }
    for (;;) {
        token = peek_token(memory);
        if (!token) {
            break;
        }
        switch (token->tag) {
        case TOKEN_ADD: {
            SET_BINOP(memory,
                      left_node,
                      OP_ADD,
                      BINDING_INFIX_ADD_SUB_L,
                      BINDING_INFIX_ADD_SUB_R);
        }
        case TOKEN_SUB: {
            SET_BINOP(memory,
                      left_node,
                      OP_SUB,
                      BINDING_INFIX_ADD_SUB_L,
                      BINDING_INFIX_ADD_SUB_R);
        }
        case TOKEN_MUL: {
            SET_BINOP(memory,
                      left_node,
                      OP_MUL,
                      BINDING_INFIX_MUL_DIV_L,
                      BINDING_INFIX_MUL_DIV_R);
        }
        case TOKEN_DIV: {
            SET_BINOP(memory,
                      left_node,
                      OP_DIV,
                      BINDING_INFIX_MUL_DIV_L,
                      BINDING_INFIX_MUL_DIV_R);
        }
        case TOKEN_LPAREN: {
            if (BINDING_CALL_LPAREN < prev_binding) {
                return left_node;
            }
            pop_token(memory);
            Node* call_node = alloc_node(memory);
            call_node->body.as_call.node = left_node;
            call_node->body.as_call.first = null;
            call_node->body.as_call.last = null;
            call_node->tag = NODE_CALL;
            token = peek_token(memory);
            EXIT_IF(!token);
            if (token->tag == TOKEN_RPAREN) {
                pop_token(memory);
                return call_node;
            }
            for (;;) {
                List<Node>* arg = alloc_list(memory);
                arg->item = parse(memory, 0);
                if (!call_node->body.as_call.first) {
                    call_node->body.as_call.first = arg;
                    call_node->body.as_call.last = arg;
                } else {
                    call_node->body.as_call.last->next = arg;
                    call_node->body.as_call.last = arg;
                }
                token = pop_token(memory);
                EXIT_IF(!token);
                if (token->tag == TOKEN_RPAREN) {
                    return call_node;
                }
                EXIT_IF(token->tag != TOKEN_COMMA);
            }
            // NOTE: This *should* be unreachable!
            ERROR();
        }
        case TOKEN_COMMA:
        case TOKEN_RPAREN: {
            return left_node;
        }
        case TOKEN_IDENT:
        case TOKEN_STR:
        case TOKEN_U32:
        default: {
            ERROR();
        }
        }
    }
    return left_node;
}

static void print(Op op) {
    switch (op) {
    case OP_ADD: {
        printf("+");
        break;
    }
    case OP_SUB: {
        printf("-");
        break;
    }
    case OP_MUL: {
        printf("*");
        break;
    }
    case OP_DIV: {
        printf("/");
        break;
    }
    default: {
        ERROR();
    }
    }
}

#define INDENT(n)                    \
    {                                \
        for (u8 i = 0; i < n; ++i) { \
            printf(" ");             \
        }                            \
    }

static void println(const Node* node, u8 n) {
    u8 m = n + 2;
    switch (node->tag) {
    case NODE_IDENT: {
        print(node->body.as_str);
        break;
    }
    case NODE_U32: {
        printf("%u", node->body.as_u32);
        break;
    }
    case NODE_STR: {
        printf("\"");
        print(node->body.as_str);
        printf("\"");
        break;
    }
    case NODE_CALL: {
        printf("( ");
        println(node->body.as_call.node, m);
        List<Node>* arg = node->body.as_call.first;
        while (arg) {
            printf("\n");
            INDENT(m);
            println(arg->item, m);
            arg = arg->next;
        }
        printf(")");
        break;
    }
    case NODE_UNOP: {
        printf("( ");
        print(node->body.as_unop.op);
        printf("\n");
        INDENT(m);
        println(node->body.as_unop.node, m);
        printf(")");
        break;
    }
    case NODE_BINOP: {
        printf("( ");
        print(node->body.as_binop.op);
        printf("\n");
        INDENT(m);
        println(node->body.as_binop.nodes[0], m);
        printf("\n");
        INDENT(m);
        println(node->body.as_binop.nodes[1], m);
        printf(")");
        break;
    }
    default: {
        ERROR();
    }
    }
    if (n == 0) {
        printf("\n");
    }
}

static void reset(Memory* memory) {
    memory->len_nodes = 0;
    memory->len_lists = 0;
    memory->cur_tokens = 0;
}

i32 main() {
    printf("\n"
           "sizeof(String)     : %zu\n"
           "sizeof(Token)      : %zu\n"
           "sizeof(List<Node>) : %zu\n"
           "sizeof(NodeBody)   : %zu\n"
           "sizeof(Node)       : %zu\n"
           "sizeof(Memory)     : %zu\n"
           "\n",
           sizeof(String),
           sizeof(Token),
           sizeof(List<Node>),
           sizeof(NodeBody),
           sizeof(Node),
           sizeof(Memory));
    println<Token, COUNT_TOKENS>(TOKENS);
    {
        Memory* memory = reinterpret_cast<Memory*>(calloc(1, sizeof(Memory)));
        EXIT_IF(!memory);
        reset(memory);
        println(parse(memory, 0), 0);
        free(memory);
    }
    return EXIT_SUCCESS;
}
