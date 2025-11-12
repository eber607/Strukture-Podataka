#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

/*
  Zadatak4.c
  Converted to C from a C++14 implementation.
  Reads two polynomials from a file, sorts terms on input,
  computes their sum and product, and prints results.

  File format:
    - Two polynomials separated by an empty line.   
    - Each polynomial: lines with "<coefficient> <exponent>"
    - Lines starting with '#' are comments and ignored.

  Usage:
    Zadatak4.exe input.txt
    If no filename argument is given the program will prompt for one.

  Output:
    Numeric results are displayed as integers (no decimal places).
    Coefficients are rounded to nearest integer for display. Example:
      1.6 -> 2, 1.4 -> 1, -1.6 -> -2, -1.4 -> -1
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#ifndef LINE_MAX_SIZE
#define LINE_MAX_SIZE 1024
#endif

/* Singly linked list node representing a polynomial term. */
typedef struct Term {
    double coef;
    int exp;
    struct Term* next;
} Term;

/*
  create_term
  Allocate and initialize a Term node with given coefficient and exponent.
*/
static Term* create_term(double coef, int exp) {
    Term* t = (Term*)malloc(sizeof(Term));
    if (!t) return NULL;
    t->coef = coef;
    t->exp = exp;
    t->next = NULL;
    return t;
}

/*
  is_blank_or_whitespace
  Returns non-zero if the given C-string is empty or contains only whitespace.
*/
static int is_blank_or_whitespace(const char* s) {
    while (*s) {
        if (!isspace((unsigned char)*s)) return 0;
        ++s;
    }
    return 1;
}

/*
  first_nonspace_char
  Returns pointer to first non-whitespace character in the string,
  or to the terminating '\0' if none exists.
*/
static const char* first_nonspace_char(const char* s) {
    while (*s && isspace((unsigned char)*s)) ++s;
    return s;
}

/*
  round_to_int
  Round input value to nearest integer (ties away from zero per C99 round()).
  Examples:
    1.6 -> 2
    1.4 -> 1
   -1.6 -> -2
   -1.4 -> -1
*/
static long round_to_int(double x) {
    return (long)round(x);
}

/*
  insert_term_sorted
  Insert a term (coef, exp) into the polynomial pointed by *head.
  The list is maintained in descending order by exponent.
  If a term with the same exponent exists, coefficients are combined.
  Terms with resulting coefficient == 0 are removed.
*/
void insert_term_sorted(Term** head, double coef, int exp) {
    if (coef == 0.0) return;

    Term* prev = NULL;
    Term* cur = *head;

    /* find insertion point (descending exponents) */
    while (cur && cur->exp > exp) {
        prev = cur;
        cur = cur->next;
    }

    if (cur && cur->exp == exp) {
        /* combine like terms */
        cur->coef += coef;
        if (fabs(cur->coef) < 1e-12) {
            /* remove zero-coef term */
            if (prev) prev->next = cur->next;
            else *head = cur->next;
            free(cur);
        }
    } else {
        /* insert new node between prev and cur */
        Term* node = create_term(coef, exp);
        if (!node) {
            fprintf(stderr, "Memory allocation failed\n");
            return;
        }
        node->next = cur;
        if (prev) prev->next = node;
        else *head = node;
    }
}

/*
  read_polynomial_from_file
  Reads one polynomial from FILE* f. Reads lines until an empty line
  or EOF is encountered. Lines starting with '#' are treated as comments.
  Each non-comment line should contain two values: coefficient and exponent.
  Terms are inserted sorted via insert_term_sorted.
  Returns 1 if at least one term was read, otherwise 0.
*/
int read_polynomial_from_file(FILE* f, Term** poly) {
    char line[LINE_MAX_SIZE];
    *poly = NULL;
    int any = 0;

    while (fgets(line, sizeof(line), f)) {
        const char* p = first_nonspace_char(line);
        if (*p == '\0' || *p == '\n' || is_blank_or_whitespace(p)) {
            /* empty line: separator between polynomials */
            break;
        }
        if (*p == '#') continue; /* comment */

        double coef;
        int exp;
        /* parse coefficient and exponent; ignore invalid lines */
        if (sscanf(p, "%lf %d", &coef, &exp) != 2) continue;

        insert_term_sorted(poly, coef, exp);
        any = 1;
    }

    return any;
}

/*
  add_polynomials
  Returns a newly allocated polynomial containing the sum of a and b.
  a and b are not modified. Caller must free the returned list.
*/
Term* add_polynomials(const Term* a, const Term* b) {
    Term* result = NULL;
    const Term* p;
    for (p = a; p; p = p->next) insert_term_sorted(&result, p->coef, p->exp);
    for (p = b; p; p = p->next) insert_term_sorted(&result, p->coef, p->exp);
    return result;
}

/*
  multiply_polynomials
  Returns a newly allocated polynomial containing the product of a and b.
  a and b are not modified. Caller must free the returned list.
*/
Term* multiply_polynomials(const Term* a, const Term* b) {
    Term* result = NULL;
    const Term* p;
    const Term* q;
    for (p = a; p; p = p->next) {
        for (q = b; q; q = q->next) {
            double nc = p->coef * q->coef;
            int ne = p->exp + q->exp;
            insert_term_sorted(&result, nc, ne);
        }
    }
    return result;
}

/*
  print_polynomial
  Prints polynomial to stdout in human-readable algebraic form.
  Coefficients are rounded to nearest integer for display (no decimals).
  If, after rounding, a term's coefficient becomes 0 it is skipped in output.
  Example: 3x^4 - 2x^2 + 5
*/
void print_polynomial(const Term* poly) {
    if (!poly) {
        printf("0\n");
        return;
    }
    int printed = 0;
    for (const Term* p = poly; p; p = p->next) {
        double c = p->coef;
        int e = p->exp;
        long r = round_to_int(c); /* rounded integer value */

        /* skip terms that become zero after rounding */
        if (r == 0L) continue;

        if (printed) {
            /* use sign of rounded value */
            printf("%s", (r >= 0L) ? " + " : " - ");
        } else {
            if (r < 0L) putchar('-');
        }

        unsigned long ac = (unsigned long)llabs(r);
        if (e == 0) {
            printf("%lu", ac);
        } else {
            /* suppress coefficient 1 after rounding (i.e. print "x" not "1x") */
            if (ac != 1UL) printf("%lu", ac);
            printf("x");
            if (e != 1) printf("^%d", e);
        }
        printed = 1;
    }
    if (!printed) {
        printf("0");
    }
    printf("\n");
}

/*
  free_polynomial
  Frees memory used by polynomial linked list.
*/
void free_polynomial(Term* poly) {
    while (poly) {
        Term* t = poly;
        poly = poly->next;
        free(t);
    }
}

/* Prompt user for filename if not provided on command line. */
static int prompt_filename_if_needed(int argc, char* argv[], char* out, size_t outsz) {
    if (argc >= 2) {
        strncpy(out, argv[1], outsz - 1);
        out[outsz - 1] = '\0';
        return 1;
    } else {
        printf("Enter input filename: ");
        if (!fgets(out, (int)outsz, stdin)) return 0;
        /* strip newline */
        size_t len = strlen(out);
        if (len && out[len - 1] == '\n') out[len - 1] = '\0';
        return out[0] != '\0';
    }
}

int main(int argc, char* argv[]) {
    char filename[512];
    if (!prompt_filename_if_needed(argc, argv, filename, sizeof(filename))) {
        fprintf(stderr, "No filename provided.\n");
        return 1;
    }

    FILE* in = fopen(filename, "r");
    if (!in) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return 1;
    }

    Term* poly1 = NULL;
    Term* poly2 = NULL;

    if (!read_polynomial_from_file(in, &poly1)) {
        fprintf(stderr, "No first polynomial found in file.\n");
        free_polynomial(poly1);
        fclose(in);
        return 1;
    }

    if (!read_polynomial_from_file(in, &poly2)) {
        fprintf(stderr, "No second polynomial found in file.\n");
        free_polynomial(poly1);
        free_polynomial(poly2);
        fclose(in);
        return 1;
    }

    fclose(in);

    printf("Polynomial A: ");
    print_polynomial(poly1);
    printf("Polynomial B: ");
    print_polynomial(poly2);

    Term* sum = add_polynomials(poly1, poly2);
    Term* prod = multiply_polynomials(poly1, poly2);

    printf("A + B: ");
    print_polynomial(sum);
    printf("A * B: ");
    print_polynomial(prod);

    free_polynomial(poly1);
    free_polynomial(poly2);
    free_polynomial(sum);
    free_polynomial(prod);

    return 0;
}