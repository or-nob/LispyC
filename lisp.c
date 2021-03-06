#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LENGTH 1024
#define TOKEN_SIZE 8
#define OPERATION(opt, a, b) a opt b
#define IF(arg, opt1, opt2) arg ? opt1 : opt2

typedef void *T;
typedef struct {
    T val;
    T type;
} lisp_pair;

typedef struct {
    T exp[4];
    T type[4];
} lisp_exp;

char *optr_list[] = {"+", "-", "*", "/", "%", ">", "<", ">=", "<=", "="};

int idx = 0, opt_idx = 0, var_cnt = 0;
lisp_pair *operator_list[MAX_LENGTH];
const char *constant_list[MAX_LENGTH] = {"pi"};

T (*function_table[MAX_LENGTH])(T *args, int argc);
lisp_exp functions[MAX_LENGTH];
int func_cnt = 0;

char *variable_list[MAX_LENGTH];
int int_list[MAX_LENGTH];

T eval(lisp_exp *token_list);

int defined_func(char *func_name, void *val1, void *val2) {
    int casted_val1 = INT32_MAX;
    int casted_val2 = INT32_MAX;
    if (val1) casted_val1 = *(int *)val1;
    if (val2) casted_val2 = *(int *)val2;

    if (!strcmp(func_name, "sqrt"))
        return sqrt(casted_val1);
    else if (!strcmp(func_name, "max"))
        return casted_val1 > casted_val2 ? casted_val1 : casted_val2;
    else if (!strcmp(func_name, "min"))
        return casted_val1 < casted_val2 ? casted_val1 : casted_val2;
    else if (!strcmp(func_name, "eq?"))
        return casted_val1 == casted_val2;

    return -1;
}

int is_optr(char *s) {
    for (int i = 0; i < 10; i++)
        if (!strcmp(s, optr_list[i])) return 1;
    return 0;
}

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

// int get_fun(char *func_name) {
//     for (int i = 0; i < func_cnt; i++)
//         if (!strcmp(func[i], val)) return i;
//
//     return -1;
// }

// let's tokenize the string first
int isLambda = 0;
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
        parsed_tokens[idx] = strdup(token);
        token = strtok(NULL, splitter);
        if (!strcmp(parsed_tokens[idx], "lambda")) isLambda = 1;
        idx++;
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

        if (idx1 != 0) {
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
            // assert((lisp_exp *)walk->exp[i] != NULL);
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

T res(char *optr, void *val1, void *val2, void *val3) {
    if (!(val1 || val2 || val3)) return NULL;
    if (!strcmp(optr, "define")) {
        char *symb = (char *)val1;
        if (!isLambda) {
            int_list[get_var_cnt(symb)] = *(int *)val2;
            return &int_list[get_var_cnt(symb)];
        } else {
            functions[func_cnt++] = *(lisp_exp *)((*(lisp_exp *)val1).exp[2]);
            // print(&functions[0]);
        }
    }
    if (isLambda) return NULL;

    T response = NULL;
    int *val = malloc(sizeof(int));
    if (!strcmp(optr, "+")) {
        *val = OPERATION(+, *(int *)val1, *(int *)val2);
        response = val;
    } else if (!strcmp(optr, "-")) {
        *val = OPERATION(-, *(int *)val1, *(int *)val2);
        response = val;
    } else if (!strcmp(optr, "*")) {
        *val = OPERATION(*, *(int *)val1, *(int *)val2);
        response = val;
    } else if (!strcmp(optr, "/")) {
        *val = OPERATION(/, *(int *)val1, *(int *)val2);
        response = val;
    } else if (!strcmp(optr, "%")) {
        *val = OPERATION(%, *(int *)val1, *(int *)val2);
        response = val;
    } else if (!strcmp(optr, ">")) {
        *val = OPERATION(>, *(int *)val1, *(int *)val2);
        response = val;
    } else if (!strcmp(optr, "<")) {
        *val = OPERATION(<, *(int *)val1, *(int *)val2);
        response = val;
    } else if (!strcmp(optr, ">=")) {
        *val = OPERATION(>=, *(int *)val1, *(int *)val2);
        response = val;
    } else if (!strcmp(optr, "<=")) {
        *val = OPERATION(<=, *(int *)val1, *(int *)val2);
        response = val;
    } else if (!strcmp(optr, "=")) {
        *val = OPERATION(=, *(int *)val1, *(int *)val2);
        response = val;
    } else if (!strcmp(optr, "if")) {
        *val = IF(*(int *)val1, *(int *)val2, *(int *)val3);
        response = val;
    } else if (!strcmp(optr, "begin")) {
        *val = (*(int *)val2);
        response = val;
    } else if (!strcmp(optr, "lamdba")) {
        *val = (*(int *)val2);
        response = val;
    }
    // else {
    //     char *func_name = optr;
    //     // TODO: I was so stupid to not have unlimited parameters.
    //     *val = defined_func(func_name, val1, val2);
    //     if (*val == -1) { /* user defined */
    //         isLambda = 0;
    //         int_list[var_cnt - 1] = *(int *)val1;
    //         // print(&functions[0]);
    //         // int val = *(int *)eval(&functions[0]);
    //         // isLambda = 1;
    //         // return val;
    //     } else /*compiler defined*/
    //         response = val;
    // }

    return response;
}

T eval(lisp_exp *token_list) {
    if (isLambda) {
        res("define", token_list, NULL, NULL);
        return NULL;
    }

    if (token_list == NULL) return NULL;

    lisp_exp *walk = token_list;

    T e_val[5] = {NULL, NULL, NULL, NULL, NULL};

    char *optr = NULL;

    for (int i = 0; i < 4; i++) {
        int *type = walk->type[i];
        if (!type) continue;

        T exp = walk->exp[i];

        if (!exp) continue;

        if (!i) {
            optr = (char *)((lisp_pair *)exp)->val;
            continue;
        }

        if (!*(type)) {
            // print((lisp_exp *)exp);
            // printf("\n");
            // printf("%d", *(int *)eval((lisp_exp *)exp));
            int *heapify = malloc(sizeof(int));
            *heapify = *(int *)eval((lisp_exp *)exp);
            e_val[i] = heapify;
            continue;
        }

        lisp_pair *pair = (lisp_pair *)exp;

        if (*(int *)pair->type == 1) {
            e_val[i] = (int *)((lisp_pair *)exp)->val;
        } else if (*(int *)pair->type == 2) {
            char *symb = (char *)((lisp_pair *)exp)->val;
            if (strcmp(optr, "define") && strcmp(optr, "begin")) {
                e_val[i] = &int_list[get_var_cnt(symb)];
            } else {
                e_val[i] = symb;
            }
        }
    }

    T result = res(optr, e_val[1], e_val[2], e_val[3]);
    if (result) return result;

    // Only lambda functions without local scopes.
    int_list[var_cnt - 1] = *(int *)e_val[1];
    lisp_exp lambda_exp = functions[0];
    // print(lambda_exp.exp[2]);
    // int res = *(int *)eval(lambda_exp.exp[2]);

    // printf("%d", res);

    return eval(lambda_exp.exp[2]);
}

void interpreter(void) {
    while (1) {
        printf("lisp.c> ");
        char s[MAX_LENGTH];

        int i = 0;
        idx = 0;
        fgets(s, MAX_LENGTH, stdin);

        // if (!strcmp(s, "exit")) break;

        char modified_string[MAX_LENGTH];
        char **tokenized_array =
            tokenize(get_modified_string(s, modified_string));

        while (tokenized_array[i] != NULL) i++;

        int list_length = i;

        lisp_exp *val =
            (lisp_exp *)read_from_token(tokenized_array, list_length, 0, NULL);

        // print(val);
        // printf("\n");
        // printf("%d", isLambda);
        // print(val);
        T res = eval(val);
        if (!isLambda) printf("%d\n", *(int *)res);
        isLambda = 0;
    }
}

int main(void) {
    interpreter();

    // printf("%d", OPERATION(+, 3, 2));
    // while (tokenized_array[i] != NULL) free(tokenized_array[i++]);
    // free(tokenized_array);
}

