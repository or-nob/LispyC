#include <stdio.h>
#include <stdlib.h>

int f(void** param_list, int param_no) {
    int sum = 0;
    for (int i = 0; i < param_no; i++) {
        sum += *(int*)param_list[i];
    }
    return sum;
}

int f1(void** param_list, int param_no) {
    int mul = 1;
    for (int i = 0; i < param_no; i++) {
        mul *= *(int*)param_list[i];
    }
    return mul;
}

// function pointer array
int (*fptr_arr[5])(void** arg, int param_no);

int main(void) {
    void** a = malloc(5 * sizeof(void*));
    int* tracker[5];

    for (int i = 0; i < 5; i++) {
        int* val = malloc(sizeof(int));
        tracker[i] = val;
        *val = i + 1;
        a[i] = val;
        fptr_arr[i] = !(i % 2) ? f : f1;
    }

    for (int i = 0; i < 5; i++) {
        printf("%d\n", fptr_arr[i](a, i + 1));
    }

    for (int i = 0; i < 5; i++) free(tracker[i]);

    free(a);
}
