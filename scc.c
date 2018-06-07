/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Defines */
typedef int key_t;
typedef key_t value_t;

#define MAX_MEMBER      5000

#define EXIT_STATUS     0
#define CONTINUE        1


/* Structures */

// Singly linked node structure
struct node_s {
    value_t value;
    struct node_s *next;
};

// Singly linked list structure
struct list_s {
    struct node_s *head;
    struct node_s *tail;
};

// Stack structure
struct stack_s {
    value_t *arr;
    int top;
};

// Information of each member
struct member_s {
    char name[21];
    char phone[12];
    
    key_t leader;

    // Adjacency list
    struct list_s *following;
    struct list_s *followed;
};

typedef struct node_s   node_t;
typedef struct list_s   list_t;
typedef struct stack_s  stack_t;
typedef struct member_s member_t;

/* Declare function prototype */

// Node
node_t *node_create();
void    node_delete(node_t *node);

// List
list_t *list_create();
int     list_insert(list_t *list, value_t v);
int     list_remove(list_t *list, value_t v);

// Stack
stack_t *stack_create(int size);
int      stack_push(stack_t *stack, value_t v);
int      stack_pop(stack_t *stack);
value_t  stack_top(stack_t *stack);
int      stack_empty(stack_t *stack);

// Member
member_t *member_create();

// Main wrapper functions
void    Init();
void    Setup();
void    Execute();

// Execute wrapper functions
int     execute_operation(char op);
void    op_leader();
void    op_follow();

// SCC implementation
void    scc();
void    dfs_phase1(key_t id);
void    dfs_phase2(key_t id, key_t lead);


/* Global variables */
member_t member[MAX_MEMBER];
int visit[MAX_MEMBER];
stack_t *stk;

// The number of *
int members;
int follows;
int groups;


/* Main function */
int main() {
    Init();
    Setup();
    Execute();
    return 0;
}


/* Function implementation */

/* Initiate global variables */
void Init() {
    int i;
    
    for (i = 0; i < MAX_MEMBER; i++) {
        member[i].leader = -1;
        member[i].following = list_create();
        member[i].followed  = list_create();
    }

    stk = stack_create(MAX_MEMBER);

    // visit[]  <- 0
    // members  <- 0
    // follows  <- 0
    // groups   <- 0
}

/* Setup existing information(member, follow) from file */
void Setup() {
    char filename[1024];
    FILE *input;
    
    int i;

    fputs("input filename: ", stdout);
    fgets(filename, 1024, stdin);
    filename[strlen(filename)-1] = 0;
    
    if ((input = fopen(filename, "r")) == NULL) {
        fputs("fopen() error\n", stderr);
        exit(1);
    }

    // Input
    fscanf(input, "%d %d", &members, &follows);
    for (i = 0; i < members; i++) {
        key_t id;
        fscanf(input, "%d", &id);
        //member[id] = member_create();
        fscanf(input, "%s %s", member[id].name, member[id].phone);
    }
    for (i = 0; i < follows; i++) {
        key_t u, v;
        fscanf(input, "%d %d", &u, &v);
        
        // u -> v (u follows v)
        list_insert(member[u].following, v);
        list_insert(member[v].followed, u);    // transpose graph
    }

    // Setup strongly connected components 
    scc();
}

/* Make strongly connected components */
void scc() {
    int i;

    groups = 0;

    // Phase 1:
    // While performing a standard depth-first search on graph,
    // put the vertices in a stack their finishing times
    for (i = 0; i < members; i++) {
        if (!visit[i]) {
            dfs_phase1(i);
        }
    }
    memset(visit, 0, members * sizeof(int));

    // Phase 2:
    // A depth-first search is performed on transpose graph
    // To start search, vertices are popped from the stack
    while (!stack_empty(stk)) {
        key_t id = stack_top(stk);
        stack_pop(stk);
        
        if (!visit[id]) {
            dfs_phase2(id, id);
            ++groups;
        }
    }
    memset(visit, 0, members * sizeof(int));
}

/* DFS on phase 1 */
void dfs_phase1(key_t id) {
    node_t *iter = member[id].following->head;

    visit[id] = 1;
    
    for (; iter != NULL; iter = iter->next) {
        if (!visit[iter->value]) {
            dfs_phase1(iter->value);
        }
    }

    stack_push(stk, id);
}

/* DFS on phase 2 */
void dfs_phase2(key_t id, key_t lead) {
    node_t *iter = member[id].followed->head;

    visit[id] = 1;
    member[id].leader = lead;

    for (; iter != NULL; iter = iter->next) {
        if (!visit[iter->value]) {
            dfs_phase2(iter->value, lead);
        }
    }
}

/* Execute queries */
void Execute() {
    char op;
    while (1) {
        scanf("%c", &op);
        if (execute_operation(op) == EXIT_STATUS) {
            break;
        }
    }
}

/* Execute an operation (by translating query) */
int execute_operation(char op) {
    switch (op) {
    
    case 'T': // print leader info
        op_leader();
        break;
    case 'A': // follow or unfollow
        op_follow();
        break;
    case '0': // exit with group count
        printf("%d\n", groups);
        return EXIT_STATUS;
    default:
        printf("Invalid operation %c\n", op);
    }

    getc(stdin); // to flush '\n' character

    return CONTINUE;
}

/* Print information of leader */
void op_leader() {
    key_t id, leader;

    scanf("%d", &id);

    leader = member[id].leader;

    printf("%d %s %s\n", leader, member[leader].name, member[leader].phone);
}

/* Renew the follow information */
void op_follow() {
    key_t u, v;
    
    scanf("%d %d", &u, &v);
    
    if (list_remove(member[u].following, v) == -1) {
        // Case of follow
        list_insert(member[u].following, v);
        list_insert(member[v].followed, u);
    } else {
        // Case of unfollow
        list_remove(member[v].followed, u);
    }

    // Renew SCC
    scc();

    printf("%d\n", member[u].leader);
}

/* Create node */
node_t *node_create() {
    node_t *node = NULL;

    if ((node = malloc(sizeof(node_t))) == NULL) {
        return NULL;
    }

    node->value = 0;
    node->next  = NULL;

    return node;
}

/* Delete node */
void node_delete(node_t *node) {
    free(node);
}

/* Create list */
list_t *list_create() {
    list_t *list = NULL;

    if ((list = malloc(sizeof(list_t))) == NULL) {
        return NULL;
    }

    list->head = NULL;
    
    return list;
}

/* Insert an element to list */
int list_insert(list_t *list, value_t v) {
    node_t *node = node_create();

    if (node == NULL) {
        return -1;
    }

    node->value = v;
    if (list->head == NULL) {
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }

    return 0;
}

/* Remove an element from list */
int list_remove(list_t *list, value_t v) {
    node_t *iter, *prev;

    iter = list->head;
    prev = NULL;

    while (iter != NULL) {
        if (iter->value == v) {
            if (iter == list->head) {
                list->head = iter->next;
            } else {
                prev->next = iter->next;
                if (iter == list->tail) {
                    list->tail = prev;
                }
            }
            node_delete(iter);
            
            return 0;
        }

        prev = iter;
        iter = iter->next;
    }

    return -1;
}

/* Create stack */
stack_t *stack_create(int size) {
    stack_t *stack = NULL;

    if ((stack = malloc(sizeof(stack_t))) == NULL) {
        return NULL;
    }

    if ((stack->arr = malloc(sizeof(value_t) * size)) == NULL) {
        free(stack);
        return NULL;
    }

    stack->top = -1;

    return stack;
}

/* Push an element to stack */
int stack_push(stack_t *stack, value_t v) {
    (stack->arr)[++(stack->top)] = v;
}

/* Pop an element from stack */
int stack_pop(stack_t *stack) {
    --(stack->top);
}

/* Return top element from stack */
value_t stack_top(stack_t *stack) {
    return (stack->arr)[stack->top];
}

/* Check the stack is empty */
int stack_empty(stack_t *stack) {
    return (stack->top == -1);
}
