#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncursesw/curses.h>
#include <locale.h>

#define MEMBER_NO 1
#define MEMBER_NAME 2
#define NODE_WIDTH 7
#define NODE_HEIGHT 3

typedef struct {
    int no;
    char name[20];
}member_t;

typedef struct binaryNode {
    member_t data;
    struct binaryNode* left;
    struct binaryNode* right;
}bnode_t;

typedef enum {
    TERMINATE = 0, 
    ADD, 
    REMOVE, 
    SEARCH, 
    PRINT
} menu_t;

int row, col;
int max_level = 0;

int member_cmp_no(const member_t* x, const member_t* y);
int member_cmp_name(const member_t* x, const member_t* y);
void member_print(const member_t* x);
void member_print_ln(const member_t* x);
member_t member_scan(const char* message, int sw);
static bnode_t* alloc_bnode_t(void);
static void set_bnode_t(bnode_t* n, const member_t* x, const bnode_t* left, const bnode_t* right);
bnode_t* tree_search(bnode_t* p, const member_t* x);
bnode_t* tree_add(bnode_t* p, const member_t* x, int level_cnt);
int tree_remove(bnode_t** root, const member_t* x);
void tree_print(const bnode_t* p);
void tree_free(bnode_t* p);
menu_t menu_select(void);
void tree_visual_print(const bnode_t* p, int level, int center);
int num_form(int a);

int main(void) {
    setlocale(LC_ALL, "");
    menu_t menu;
    bnode_t* root = NULL;

    initscr();
    getmaxyx(stdscr, row, col);
    null_visual_print();
    refresh();
    do {
        member_t x;
        bnode_t* temp;
        
        switch (menu = menu_select()) {
        case ADD :
            x = member_scan("add", MEMBER_NO | MEMBER_NAME);
            root = tree_add(root, &x, 0);
            tree_visual_print(root, 0, col/2);
            break;

        case REMOVE :
            x = member_scan("remove", MEMBER_NO);
            if (tree_remove(&root, &x) != -1) {
                tree_visual_print(root, 0, col/2);
            }
            break;

        case SEARCH :
            x = member_scan("search", MEMBER_NO);
            if ((temp = tree_search(root, &x)) != NULL) {
                member_print_ln(&temp->data);
            }
            else {
                clrtobot();
                printw("[error] %d is not allocated\n", x.no);
            }
            break;

        case PRINT :
            printw("[print all node]\n");
            tree_print(root);
            clrtobot();
            break;
        }
        refresh();
    } while (menu != TERMINATE);
    tree_free(root);
    endwin();

    return 0;
}
int member_cmp_no(const member_t* x, const member_t* y) {
    return x->no < y->no ? -1 : x->no > y->no ? 1 : 0;
}
int member_cmp_name(const member_t* x, const member_t* y) {
    return strcmp(x->name, y->name);
}
void member_print(const member_t* x){
    printw("{%d : %s}", x->no, x->name);
}
void member_print_ln(const member_t* x) {
    printw("{%d : %s}\t", x->no, x->name);
}
member_t member_scan(const char* message, int sw) {
    int cnt = 0;
    member_t temp;
    move(row-4,0);
    clrtobot();
    printw("Write your data to %s\n", message);
    if (sw & MEMBER_NO) {
        mvprintw(row-3, 0, "no : ");
        scanw("%d", &temp.no);
        cnt++;
    }
    if (sw & MEMBER_NAME) {
        mvprintw(row-3+cnt, 0, "name : ");
        scanw("%s", temp.name);
    }
    return temp;
}

static bnode_t* alloc_bnode_t(void) {
    return calloc(1, sizeof(bnode_t));
}

static void set_bnode_t(bnode_t* n, const member_t* x, const bnode_t* left, const bnode_t* right) {
   n->data = *x;
   n->left = left;
   n->right = right;
}

bnode_t* tree_search(bnode_t* p, const member_t* x) {
    int cond;
    if (p == NULL) {
        return NULL;
    } else if ((cond = member_cmp_no(x, &p->data)) == 0) {
        return p;
    } else if (cond < 0) {
        tree_search(p->left, x);
    } else {
        tree_search(p->right, x);
    }
}

bnode_t* tree_add(bnode_t* p, const member_t* x, int level_cnt) {
    int cond;
    if (p == NULL) {
        p = alloc_bnode_t();
        set_bnode_t(p, x, NULL, NULL);
        if (level_cnt > max_level) {
            max_level = level_cnt;
        }
    } else if ((cond = member_cmp_no(x, &p->data)) == 0) {
        // move(row-4,0);
        clrtobot();
        printw("[error] %d is already allocated\n", x->no);
        refresh();
        return p;
    } else if (cond < 0) {
        p->left = tree_add(p->left, x, level_cnt+1);
        return p;
    } else {
        p->right = tree_add(p->right, x, level_cnt+1);
        return p;
    }
    erase();
    return p;
}

int tree_remove(bnode_t** root, const member_t* x) {
    bnode_t* next, *temp;
    bnode_t** left;
    bnode_t** p = root;

    while(1) {
        int cond;
        if (*p == NULL) {
            clrtobot();
            printw("[error] %d is not allocated\n", x->no);
            refresh();
            return -1;
        } else if ((cond = member_cmp_no(x, &(*p)->data)) == 0) {
            break;
        } else if (cond < 0) {
            p = &((*p)->left);
        } else {
            p = &((*p)->right);
        }
    }

    if ((*p)->left == NULL) {
        if ((*p)->right == NULL) {
            *p = NULL;
            erase();
            null_visual_print();
            return -1;
        } else {
            next = (*p)->right;
        }
    } else {
        left = &((*p)->left);
        while ((*left)->right != NULL) {
            left = &(*left)->right;
        }
        next = *left;
        *left = (*left)->left;
        next->left = (*p)->left;
        next->right = (*p)->right;
    }
    temp = *p;
    *p = next;
    free(temp);
    erase();
    return 0;
}

void tree_print(const bnode_t* p) {
    if (p != NULL) {
        tree_print(p->left);
        member_print_ln(&p->data);
        tree_print(p->right);
    }
}

void tree_free(bnode_t* p) {
    if (p != NULL) {
        tree_free(p->left);
        tree_free(p->right);
        free(p);
    }
}

menu_t menu_select(void) {
    int ch;
    do {
        move(row-5, 0);
        clrtoeol();
        printw("(1)Add (2)Remove (3)Search (4)Print (0)Exit : ");
        refresh();
        scanw("%d", &ch);
    } while (ch < TERMINATE || ch > PRINT);
    return (menu_t)ch;
}

int num_form(int a){
    if (a > 99999) {
        while (a > 999){
            a /= 10;
        }
    }
    else if (a < -9999) {
        while (a < -99) {
            a /= 10;
        }
    }
    return a;
}

void tree_visual_print(const bnode_t* p, int level, int center) {
    member_t* x = &p->data;
    mvprintw(level * 4,     (center - NODE_WIDTH/2), level == 0 ? "┌─────┐" : "┌──┴──┐");
    mvprintw(level * 4 + 1, (center - NODE_WIDTH/2), "│     │");
    if (p->left == NULL && p-> right == NULL) {
        mvprintw(level * 4 + 2, (center - NODE_WIDTH/2), "└─────┘");
        mvprintw(level * 4 + 1, (center - NODE_WIDTH/2)+1, "%d%s", num_form(x->no), x->no > 99999 || x->no < -9999 ? ".." : "");
    }
    else if (p->left != NULL && p->right == NULL) {
        // mvprintw(level * 4,     (col/2 - NODE_WIDTH/2),     "┌─────┐");
        // mvprintw(level * 4 + 1, (col/2 - NODE_WIDTH/2),     "│     │");
        mvprintw(level * 4 + 2, (center - NODE_WIDTH/2),     "└──┬──┘");
        mvprintw(level * 4 + 3, (center - (1 << (max_level - level + 1))), "┌");
        for (int i=0; i<(1 << (max_level - level + 1)) - 1; i++) { printw("─"); }
        printw("┘");
        mvprintw(level * 4 + 1, (center - NODE_WIDTH/2)+1, "%d%s", num_form(x->no), x->no > 99999 || x->no < -9999 ? ".." : "");
        tree_visual_print(p->left, level+1, center - (1 << (max_level - level + 1)));
    }
    else if (p->left == NULL && p->right != NULL) {
        // mvprintw(level * 4,     (col/2 - NODE_WIDTH/2),     "┌─────┐");
        // mvprintw(level * 4 + 1, (col/2 - NODE_WIDTH/2),     "│     │");
        mvprintw(level * 4 + 2, (center - NODE_WIDTH/2),     "└──┬──┘");
        mvprintw(level * 4 + 3, center, "└");
        for (int i=0; i<(1 << (max_level - level + 1)) - 1; i++) { printw("─"); }
        printw("┐");
        mvprintw(level * 4 + 1, (center - NODE_WIDTH/2)+1, "%d%s", num_form(x->no), x->no > 99999 || x->no < -9999 ? ".." : "");
        tree_visual_print(p->right, level+1, center + (1 << (max_level - level + 1)));
    }
    else {
        // mvprintw(level * 4,     (col/2 - NODE_WIDTH/2),     "┌─────┐");
        // mvprintw(level * 4 + 1, (col/2 - NODE_WIDTH/2),     "│     │");
        mvprintw(level * 4 + 2, (center - NODE_WIDTH/2),     "└──┬──┘");
        mvprintw(level * 4 + 3, (center - (1 << (max_level - level + 1))), "┌");
        for (int i=0; i<(1 << (max_level - level + 2)) - 1; i++) { printw("─"); }
        printw("┐");
        mvprintw(level * 4 + 3, (center), "┴");
        mvprintw(level * 4 + 1, (center - NODE_WIDTH/2)+1, "%d%s", num_form(x->no), x->no > 99999 || x->no < -9999 ? ".." : "");
        tree_visual_print(p->left, level+1, center - (1 << (max_level - level + 1)));
        tree_visual_print(p->right, level+1, center + (1 << (max_level - level + 1)));
    }
    refresh();
}

void null_visual_print(void) {
    mvprintw(0, (col/2 - NODE_WIDTH/2), "┌─────┐");
    mvprintw(1, (col/2 - NODE_WIDTH/2), "│     │");
    mvprintw(2, (col/2 - NODE_WIDTH/2), "└─────┘");
    refresh();
}