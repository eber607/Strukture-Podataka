#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>

#if defined(_MSC_VER)
#define strtok_r(str, delim, saveptr) strtok_s((str), (delim), (saveptr))
#endif

/* Node of the linked-list stack storing integer values. */
typedef struct Node {
    long long value;
    struct Node* next;
} Node;

/* Create a new node with the given value. Returns pointer or NULL on allocation failure. */
Node* create_node(long long value) {
    Node* n = (Node*)malloc(sizeof(Node));
    if (!n) return NULL;
    n->value = value;
    n->next = NULL;
    return n;
}

/* Push a value onto the stack whose top is pointed to by 'top'. */
int push(Node** top, long long value) {
    Node* n = create_node(value);
    if (!n) return 0; /* failure */
    n->next = *top;
    *top = n;
    return 1; /* success */
}

/* Pop a value from the stack. If stack is empty, set *err = 1 and return 0. */
long long pop(Node** top, int* err) {
    if (!top || !*top) {
        if (err) *err = 1;
        return 0;
    }
    Node* n = *top;
    long long val = n->value;
    *top = n->next;
    free(n);
    if (err) *err = 0;
    return val;
}

/* Return non-zero if the stack is empty. */
int is_empty(Node* top) {
    return top == NULL;
}

/* Free all nodes in the stack. */
void free_stack(Node** top) {
    if (!top) return;
    Node* cur = *top;
    while (cur) {
        Node* nxt = cur->next;
        free(cur);
        cur = nxt;
    }
    *top = NULL;
}

/* Compute integer power base^exp for non-negative exp. */
long long int_pow(long long base, long long exp) {
    long long result = 1;
    while (exp > 0) {
        if (exp & 1) result *= base;
        base *= base;
        exp >>= 1;
    }
    return result;
}

/* Read entire file into a newly allocated buffer. Caller must free the buffer.
   On success returns pointer to buffer and sets *size to number of bytes (excluding NUL).
   On failure returns NULL. */
char* read_file_to_buffer(const char* filename, long* size) {
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long len = ftell(f);
    if (len < 0) { fclose(f); return NULL; }
    rewind(f);
    char* buf = (char*)malloc((size_t)len + 1);
    if (!buf) { fclose(f); return NULL; }
    if (fread(buf, 1, (size_t)len, f) != (size_t)len) {
        free(buf);
        fclose(f);
        return NULL;
    }
    buf[len] = '\0';
    fclose(f);
    if (size) *size = len;
    return buf;
}

/* Evaluate a postfix expression read from file 'input'.
   The expression should contain tokens separated by whitespace; tokens are integers or operators: + - * / % ^.
   On success returns 0 and stores result into *out_result.
   On error returns non-zero error code:
     1 - file read error
     2 - invalid token
     3 - stack underflow / malformed expression
     4 - division by zero
     5 - negative exponent (for ^)
*/
int evaluate_postfix(const char* filename, long long* out_result) {
    long size = 0;
    char* content = read_file_to_buffer(filename, &size);
    if (!content) return 1;

    Node* stack = NULL;
    const char* delims = " \t\r\n";
    char* saveptr = NULL;
    char* token = strtok_r(content, delims, &saveptr);

    while (token) {
        /* Try to parse token as integer (allowing optional leading + or -). */
        char* endptr = NULL;
        errno = 0;
        long long val = strtoll(token, &endptr, 10);
        if (endptr && *endptr == '\0' && errno == 0) {
            if (!push(&stack, val)) {
                free(content);
                free_stack(&stack);
                return 1; /* memory failure treated as file/read error code */
            }
        } else if (strlen(token) == 1) {
            /* Single-character operator */
            char op = token[0];
            int err = 0;
            long long right = pop(&stack, &err);
            if (err) { free(content); free_stack(&stack); return 3; }
            long long left = pop(&stack, &err);
            if (err) { free(content); free_stack(&stack); return 3; }

            long long res = 0;
            switch (op) {
                case '+': res = left + right; break;
                case '-': res = left - right; break;
                case '*': res = left * right; break;
                case '/':
                    if (right == 0) { free(content); free_stack(&stack); return 4; }
                    res = left / right;
                    break;
                case '%':
                    if (right == 0) { free(content); free_stack(&stack); return 4; }
                    res = left % right;
                    break;
                case '^':
                    if (right < 0) { free(content); free_stack(&stack); return 5; }
                    res = int_pow(left, right);
                    break;
                default:
                    free(content);
                    free_stack(&stack);
                    return 2; /* invalid token/operator */
            }
            if (!push(&stack, res)) {
                free(content);
                free_stack(&stack);
                return 1;
            }
        } else {
            /* Token is neither a valid integer nor single-char operator */
            free(content);
            free_stack(&stack);
            return 2;
        }

        token = strtok_r(NULL, delims, &saveptr);
    }

    /* After processing tokens, there must be exactly one value on the stack */
    int err = 0;
    long long final = pop(&stack, &err);
    if (err || !is_empty(stack)) {
        free(content);
        free_stack(&stack);
        return 3;
    }

    free(content);
    *out_result = final;
    return 0;
}


int main(int argc, char* argv[]) {
    const char* filename = (argc >= 2) ? argv[1] : "input.txt";
    long long result = 0;
    int rc = evaluate_postfix(filename, &result);
    if (rc == 0) {
        printf("%lld\n", result);
        return 0;
    } else {
        switch (rc) {
            case 1: fprintf(stderr, "Error: cannot read file '%s'\n", filename); break;
            case 2: fprintf(stderr, "Error: invalid token in postfix expression\n"); break;
            case 3: fprintf(stderr, "Error: malformed expression (stack underflow or extra operands)\n"); break;
            case 4: fprintf(stderr, "Error: division by zero\n"); break;
            case 5: fprintf(stderr, "Error: negative exponent not supported\n"); break;
            default: fprintf(stderr, "Error: unknown\n"); break;
        }
        return rc;
    }
}

