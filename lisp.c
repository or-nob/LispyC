#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LENGTH 1024
#define TOKEN_SIZE 8
#define T void *
#define OPERATION(opt, a, b) a opt b
#define IF(arg, opt1, opt2) arg ? opt1 : opt2
// #define FUNCTION(function_name, )

struct lisp_pair {
    T val;
    T type;
};

struct lisp_exp {
    T exp[4];
    T type[4];
};

#define lisp_pair struct lisp_pair
#define lisp_exp struct lisp_exp

int idx = 0, opt_idx = 0, var_cnt = 0;
lisp_pair *operator_list[MAX_LENGTH];
const char *constant_list[MAX_LENGTH] = {"pi"};

char *variable_list[MAX_LENGTH];
int int_list[MAX_LENGTH];

int is_constant(char *val) {
    for (int i = 0; i < 1; i++)
        if (!strcmp(constant_list[i], val)) return 1;

    for (int i = 0; i < var_cnt; i++)
        if (!strcmp(variable_list[i], val)) return 1;

    return 0;
}

int get_var_cnt(char *val) {
    for (int i = 0; i < var_cnt; i++)
        if (!strcmp(variable_list[i], val)) return i;

    return -1;
}

// let's tokenize the string first
char **tokenize(char *s) {
    char **parsed_tokens = (char **)malloc(TOKEN_SIZE * (sizeof(char *)));
    char *token;
    const char splitter[] = " ";
    token = strtok(s, splitter);

    int idx = 0;
    int n = 1;
    while (token != NULL) {
        if (idx >= n * TOKEN_SIZE) {
            parsed_tokens = (char **)realloc(
                parsed_tokens, (n + 1) * TOKEN_SIZE * (sizeof(char *)));
            n++;
        }
        parsed_tokens[idx++] = strdup(token);
        token = strtok(NULL, splitter);
    }

    return parsed_tokens;
}

// atom works fine!!
T atom(char *token) {
    int *token_type = malloc(sizeof(int));
    *token_type = 0;

    float token_to_float = strtof(token, NULL);

    int token_to_int = 0;

    if (strcmp(token, "0.0")) {
        if (token_to_float != 0.0) {
            token_to_int = token_to_float;
            if (token_to_float == token_to_int) {
                *token_type = 1;
            }
        } else {
            *token_type = 2;
        }
    }

    lisp_pair *atom_lisp_pair = (lisp_pair *)malloc(sizeof(lisp_pair));

    if (*token_type == 0) {
        float *float_ptr = (float *)malloc(sizeof(float));
        *float_ptr = token_to_float;
        atom_lisp_pair->val = float_ptr;
    }
    if (*token_type == 1) {
        int *int_ptr = (int *)malloc(sizeof(int));
        *int_ptr = token_to_int;
        atom_lisp_pair->val = int_ptr;
    }
    if (*token_type == 2) {
        atom_lisp_pair->val = token;
    }
    atom_lisp_pair->type = token_type;

    return atom_lisp_pair;
}

T read_from_token(char **tokens, int length, int idx1, lisp_exp *paren_exp) {
    if (length == idx) return NULL;

    int *token_type = malloc(sizeof(int));
    char *token = tokens[idx++];

    if (!strcmp(token, "(")) {
        lisp_exp *L = malloc(sizeof(lisp_exp));
        for (int p = 0; p < 4; p++) {
            L->exp[p] = NULL;
            L->type[p] = NULL;
        }

        int old_idx = idx1;
        *token_type = 0;
        L->type[idx1] = token_type;
        idx1 = 0;
        while (strcmp(tokens[idx], ")")) {
            L->exp[idx1] = read_from_token(tokens, length, idx1, L);
            idx1++;
        }

        idx++;

        if (paren_exp != NULL) {
            paren_exp->exp[old_idx] = L;
            paren_exp->type[old_idx] = token_type;
        }
        return L;
    } else if (!strcmp(token, ")")) {
        return NULL;
    } else {
        *token_type = 1;

        paren_exp->exp[idx1] = atom(token);
        paren_exp->type[idx1] = token_type;

        if (idx1 != 0) { /*variable_list[var_cnt++] =*/
            lisp_pair *tmp_pair = (lisp_pair *)paren_exp->exp[idx1];
            if (*(int *)tmp_pair->type == 2) {
                char *symb = (char *)tmp_pair->val;
                if (!is_constant(symb)) variable_list[var_cnt++] = symb;
            }
        }
        return paren_exp->exp[idx1];
    }
}

void print_pair(lisp_pair *pair) {
    int token_type = *(int *)pair->type;
    if (token_type == 0) {
        printf("%f ", *(float *)pair->val);
    }
    if (token_type == 1) {
        printf("%d ", *(int *)pair->val);
    }
    if (token_type == 2) {
        printf("%s ", (char *)pair->val);
    }
}

void print(lisp_exp *token_list) {
    if (token_list == NULL) return;

    lisp_exp *walk = token_list;

    for (int i = 0; i < 4; i++) {
        if (!walk) continue;
        int *type = walk->type[i];
        if (!type) continue;

        if (!(*type)) {
            printf("{");
            print((lisp_exp *)walk->exp[i]);
            printf("}");
        } else {
            lisp_exp *paired = (lisp_exp *)walk->exp[i];
            print_pair((lisp_pair *)paired->exp);
        }
    }
}

char *get_modified_string(char *s, char *modified_string) {
    int i = 0, idx = 0;
    while (*(s + i)) {
        if (s[i] != '(' && s[i] != ')')
            modified_string[idx++] = s[i++];
        else {
            modified_string[idx++] = ' ';
            modified_string[idx++] = s[i++];
            modified_string[idx++] = ' ';
        }
    }

    return modified_string;
}

int res(char *optr, void *val1, void *val2, void *val3) {
    if (!strcmp(optr, "+"))
        return OPERATION(+, *(int *)val1, *(int *)val2);
    else if (!strcmp(optr, "-"))
        return OPERATION(-, *(int *)val1, *(int *)val2);
    else if (!strcmp(optr, "*"))
        return OPERATION(*, *(int *)val1, *(int *)val2);
    else if (!strcmp(optr, "/"))
        return OPERATION(/, *(int *)val1, *(int *)val2);
    else if (!strcmp(optr, "%"))
        return OPERATION(%, *(int *)val1, *(int *)val2);
    else if (!strcmp(optr, ">"))
        return OPERATION(>, *(int *)val1, *(int *)val2);
    else if (!strcmp(optr, "<"))
        return OPERATION(<, *(int *)val1, *(int *)val2);
    else if (!strcmp(optr, ">="))
        return OPERATION(>=, *(int *)val1, *(int *)val2);
    else if (!strcmp(optr, "<="))
        return OPERATION(<=, *(int *)val1, *(int *)val2);
    else if (!strcmp(optr, "="))
        return OPERATION(=, *(int *)val1, *(int *)val2);
    else if (!strcmp(optr, "if"))
        return IF(*(int *)val1, *(int *)val2, *(int *)val3);
    else if (!strcmp(optr, "define")) {
        char *symb = (char *)val1;
        int_list[get_var_cnt(symb)] = *(int *)val2;
        return int_list[get_var_cnt(symb)];
    } else if (!strcmp(optr, "begin")) {
        return (*(int *)val2);
    }

    return -1;
}

int eval(lisp_exp *token_list) {
    if (token_list == NULL) return -1;

    lisp_exp *walk = token_list;

    T e_val[5] = {NULL, NULL, NULL, NULL, NULL};

    char *optr = NULL;
    for (int i = 0; i < 4; i++) {
        int *type = walk->type[i];
        if (!type) continue;

        T exp = walk->exp[i];

        if (!i) {
            optr = (char *)((lisp_pair *)exp)->val;
            continue;
        }

        if (!*(type)) {
            int val = eval((lisp_exp *)exp);
            e_val[i] = &val;
            continue;
        }

        lisp_pair *pair = (lisp_pair *)exp;

        if (*(int *)pair->type == 1) {
            int val = *(int *)((lisp_pair *)exp)->val;
            e_val[i] = &val;
        } else if (*(int *)pair->type == 2) {
            char *symb = (char *)((lisp_pair *)exp)->val;
            // printf("%s", symb);
            if (strcmp(optr, "define") && strcmp(optr, "begin")) {
                // printf("%s\n", symb);
                // printf("%d\n", int_list[get_var_cnt(symb)]);
                e_val[i] = &int_list[get_var_cnt(symb)];
            } else {
                e_val[i] = symb;
            }
        }
    }

    return res(optr, e_val[1], e_val[2], e_val[3]);
}

int main(void) {
    // char s[MAX_LENGTH] = "(begin (define r 10) (* pi (* r r)))";
    // char s[MAX_LENGTH] = "(5)";
    // char s[MAX_LENGTH] = "(sqrt (* 2 2))";
    // char s[MAX_LENGTH] = "(begin (define r 10) (* pi (* r r)))";

    // char s[MAX_LENGTH] = "(* 3 (+ 1 2))";
    char s[MAX_LENGTH] = "(begin (define r (* 10 10)) (* 3 (* r r)))";

    char modified_string[MAX_LENGTH];

    int i = 0;
    idx = 0;

    char **tokenized_array = tokenize(get_modified_string(s, modified_string));
    i = 0;

    while (tokenized_array[i] != NULL) i++;
    int list_length = i;

    idx = 0;

    lisp_exp *val =
        (lisp_exp *)read_from_token(tokenized_array, list_length, 0, NULL);

    // eval(val);
    printf("%d\n", eval(val));
    // print(val);

    // printf("\n");

    i = 0;

    // for (int j = 0; j < opt_idx; j++) {
    //     print_pair(operator_list[j]);
    // }

    // for (int j = 0; j < var_cnt; j++) {
    //     printf("%s ", variable_list[j]);
    // }

    printf("\n");

    // printf("%d", OPERATION(+, 3, 2));
    // while (tokenized_array[i] != NULL) free(tokenized_array[i++]);
    // free(tokenized_array);
}
