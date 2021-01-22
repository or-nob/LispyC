#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LENGTH 1024
#define TOKEN_SIZE 8
#define T void *

struct lisp_pair {
    T val;
    T type;
};

struct lisp_exp {
    T exp[3];
    T type[3];
};

#define lisp_pair struct lisp_pair
#define lisp_exp struct lisp_exp

int idx = 0, opt_idx = 0;
lisp_pair *operator_list[MAX_LENGTH];

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

    for (int i = 0; i < 3; i++) {
        int *type = walk->type[i];
        if (!type) continue;

        if (!(*type)) {
            printf("{");
            print((lisp_exp *)walk->exp[i]);
            printf("}");
        } else {
            lisp_exp *paired = (lisp_exp *)walk->exp[i];
            print_pair((lisp_pair *)paired->exp);
            if (i == 0) {
                operator_list[opt_idx++] = (lisp_pair *)paired->exp;
            }
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

// void eval(lisp_exp *token_list) {
//     if (token_list == NULL) return;
//
//     lisp_exp *walk = token_list;
//
//     for (int i = 0; i < 3; i++) {
//         int *type = walk->type[i];
//         if (!type) continue;
//
//         if (!(*type)) {
//             printf("{");
//             eval((lisp_exp *)walk->exp[i]);
//             printf("}");
//         } else {
//             lisp_exp *paired = (lisp_exp *)walk->exp[i];
//             print_pair((lisp_pair *)paired->exp);
//             if (i == 0) {
//                 operator_list[opt_idx++] = (lisp_pair *)paired->exp;
//             }
//         }
//     }
// }

int main(void) {
    // char s[MAX_LENGTH] = "(begin (define r 10) (* pi (* r r)))";
    // char s[MAX_LENGTH] = "(5)";
    // char s[MAX_LENGTH] = "(sqrt (* 2 2))";
    char s[MAX_LENGTH] = "(begin (define r 10) (* pi (* r r)))";
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

    printf("{");
    print(val);
    printf("}");

    printf("\n");

    i = 0;

    for (int j = 0; j < opt_idx; j++) {
        print_pair(operator_list[j]);
    }

    printf("\n");
    // while (tokenized_array[i] != NULL) free(tokenized_array[i++]);
    // free(tokenized_array);
}
