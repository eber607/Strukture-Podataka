#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/* Function declarations */

/* Date helpers */
typedef struct Date {
    int y, m, d;
} Date;
bool parse_date(const char *s, Date *out);
int compare_date(const Date *a, const Date *b); /* -1 if a<b, 0 if equal, 1 if a>b */

/* String helpers */
char *str_dup(const char *s);
void trim_inplace(char *s);
int ci_strcmp(const char *a, const char *b); /* case-insensitive compare */

/* Item list */
typedef struct Item {
    char *name;
    long long qty;
    double total_cost; /* sum of qty * unit_price for occurrences */
    struct Item *next;
} Item;
Item *create_item(const char *name, long long qty, double unit_price);
Item *insert_item_sorted_merge(Item *head, Item *node); /* keeps list sorted by name, merges same names */
void free_items(Item *head);

/* Invoice list */
typedef struct Invoice {
    Date date;
    Item *items;
    struct Invoice *next;
} Invoice;
Invoice *create_invoice(const Date *date);
Invoice *insert_invoice_sorted(Invoice *head, Invoice *node); /* sorted by date ascending */
void free_invoices(Invoice *head);

/* File reading */
int read_index_and_build(const char *index_filename, Invoice **out_head);
int read_single_invoice_file(const char *filename, Invoice **out_head);

/* Query */
void query_range(const Invoice *head, const char *item_name, const Date *from, const Date *to, long long *out_qty, double *out_spent);

/* Utility / printing */
void print_date(const Date *d);
void print_invoice_list(const Invoice *head);

/* Main */
int main(void)
{
    Invoice *head = NULL;
    const char *index_filename = "racuni.txt";

    printf("Reading index file '%s'...\n", index_filename);
    if (read_index_and_build(index_filename, &head) != 0) {
        fprintf(stderr, "Failed to read index file or build invoice list.\n");
        free_invoices(head);
        return 1;
    }

    /* Optionally print the built lists for verification */
    /* print_invoice_list(head); */

    char item_query[256];
    char date_from_s[32];
    char date_to_s[32];

    printf("Enter item name to query: ");
    if (!fgets(item_query, sizeof(item_query), stdin)) {
        fprintf(stderr, "No input\n");
        free_invoices(head);
        return 1;
    }
    trim_inplace(item_query);
    if (item_query[0] == '\0') {
        fprintf(stderr, "Empty item name\n");
        free_invoices(head);
        return 1;
    }

    printf("Enter start date (YYYY-MM-DD): ");
    if (!fgets(date_from_s, sizeof(date_from_s), stdin)) {
        fprintf(stderr, "No input\n");
        free_invoices(head);
        return 1;
    }
    trim_inplace(date_from_s);

    printf("Enter end date (YYYY-MM-DD): ");
    if (!fgets(date_to_s, sizeof(date_to_s), stdin)) {
        fprintf(stderr, "No input\n");
        free_invoices(head);
        return 1;
    }
    trim_inplace(date_to_s);

    Date from, to;
    if (!parse_date(date_from_s, &from) || !parse_date(date_to_s, &to)) {
        fprintf(stderr, "Invalid date format. Use YYYY-MM-DD.\n");
        free_invoices(head);
        return 1;
    }

    /* Ensure from <= to */
    if (compare_date(&from, &to) > 0) {
        fprintf(stderr, "Start date is after end date. Swapping.\n");
        Date tmp = from; from = to; to = tmp;
    }

    long long total_qty = 0;
    double total_spent = 0.0;
    query_range(head, item_query, &from, &to, &total_qty, &total_spent);

    printf("Result for item \"%s\" between ", item_query);
    print_date(&from);
    printf(" and ");
    print_date(&to);
    printf(":\n");
    printf("  Total quantity: %lld\n", total_qty);
    printf("  Total spent: %.2f\n", total_spent);

    free_invoices(head);
    return 0;
}

/* ---------------- Function definitions ---------------- */

/* Parse date string "YYYY-MM-DD" into Date; returns true on success */
bool parse_date(const char *s, Date *out)
{
    if (!s || !out) return false;
    int y, m, d;
    if (sscanf(s, "%d-%d-%d", &y, &m, &d) != 3) return false;
    if (m < 1 || m > 12 || d < 1 || d > 31) return false;
    out->y = y; out->m = m; out->d = d;
    return true;
}

/* Compare two dates: -1 if a<b, 0 if equal, 1 if a>b */
int compare_date(const Date *a, const Date *b)
{
    if (a->y != b->y) return (a->y < b->y) ? -1 : 1;
    if (a->m != b->m) return (a->m < b->m) ? -1 : 1;
    if (a->d != b->d) return (a->d < b->d) ? -1 : 1;
    return 0;
}

/* Duplicate string using malloc */
char *str_dup(const char *s)
{
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

/* Trim leading and trailing whitespace in-place */
void trim_inplace(char *s)
{
    if (!s) return;
    char *start = s;
    while (isspace((unsigned char)*start)) start++;
    if (start != s) memmove(s, start, strlen(start) + 1);
    char *end = s + strlen(s) - 1;
    while (end >= s && isspace((unsigned char)*end)) *end-- = '\0';
}

/* Case-insensitive compare, returns same semantics as strcmp */
int ci_strcmp(const char *a, const char *b)
{
    unsigned char ca, cb;
    while (*a && *b) {
        ca = (unsigned char) tolower((unsigned char)*a);
        cb = (unsigned char) tolower((unsigned char)*b);
        if (ca != cb) return (ca < cb) ? -1 : 1;
        a++; b++;
    }
    if (*a) return 1;
    if (*b) return -1;
    return 0;
}

/* Create an item node (total_cost = qty * unit_price) */
Item *create_item(const char *name, long long qty, double unit_price)
{
    Item *it = malloc(sizeof(Item));
    if (!it) return NULL;
    it->name = str_dup(name ? name : "");
    it->qty = qty;
    it->total_cost = (double)qty * unit_price;
    it->next = NULL;
    return it;
}

/* Insert item into sorted list by name. If name exists (case-insensitive), merge quantities and costs. */
Item *insert_item_sorted_merge(Item *head, Item *node)
{
    if (!node) return head;
    if (!head) return node;

    Item *prev = NULL;
    Item *cur = head;
    while (cur) {
        int cmp = ci_strcmp(node->name, cur->name);
        if (cmp == 0) {
            /* merge */
            cur->qty += node->qty;
            cur->total_cost += node->total_cost;
            free(node->name);
            free(node);
            return head;
        } else if (cmp < 0) {
            /* insert before cur */
            if (prev) {
                prev->next = node;
            } else {
                head = node;
            }
            node->next = cur;
            return head;
        }
        prev = cur;
        cur = cur->next;
    }
    /* insert at end */
    prev->next = node;
    node->next = NULL;
    return head;
}

/* Free item list */
void free_items(Item *head)
{
    while (head) {
        Item *n = head->next;
        free(head->name);
        free(head);
        head = n;
    }
}

/* Create invoice node */
Invoice *create_invoice(const Date *date)
{
    Invoice *inv = malloc(sizeof(Invoice));
    if (!inv) return NULL;
    inv->date = *date;
    inv->items = NULL;
    inv->next = NULL;
    return inv;
}

/* Insert invoice into list sorted by date ascending */
Invoice *insert_invoice_sorted(Invoice *head, Invoice *node)
{
    if (!node) return head;
    if (!head) return node;
    Invoice *prev = NULL;
    Invoice *cur = head;
    while (cur && compare_date(&cur->date, &node->date) < 0) {
        prev = cur;
        cur = cur->next;
    }
    if (!prev) {
        node->next = head;
        return node;
    } else {
        prev->next = node;
        node->next = cur;
        return head;
    }
}

/* Free invoice list and contained items */
void free_invoices(Invoice *head)
{
    while (head) {
        Invoice *n = head->next;
        free_items(head->items);
        free(head);
        head = n;
    }
}

/* Read invoices from an already-open FILE stream. The stream may contain
   multiple invoices one after another: each invoice starts with a date line
   "YYYY-MM-DD" followed by item lines "name, quantity, price" until a blank
   line or the next date. Returns 0 on success. */
static int read_invoices_from_stream(FILE *f, Invoice **out_head)
{
    if (!f || !out_head) return -1;
    char line[1024];

    while (1) {
        /* find next date line */
        Date date;
        bool got_date = false;
        while (fgets(line, sizeof(line), f)) {
            trim_inplace(line);
            if (line[0] == '\0') continue;
            if (parse_date(line, &date)) {
                got_date = true;
                break;
            }
            /* ignore any lines until a date is found */
        }
        if (!got_date) break;

        Invoice *inv = create_invoice(&date);
        if (!inv) return -1;

        /* read item lines until blank line or next date */
        while (1) {
            long pos_before = ftell(f);
            if (!fgets(line, sizeof(line), f)) break;
            trim_inplace(line);
            if (line[0] == '\0') break;

            Date nextd;
            if (parse_date(line, &nextd)) {
                /* this line is the next invoice's date: roll back so outer loop will see it */
                fseek(f, pos_before, SEEK_SET);
                break;
            }

            /* parse item: name, qty, price */
            char *copy = str_dup(line);
            if (!copy) continue;
            char *p = copy;
            char *token;

            token = strtok(p, ",");
            if (!token) { free(copy); continue; }
            char name[512];
            strncpy(name, token, sizeof(name)-1); name[sizeof(name)-1] = '\0';
            trim_inplace(name);

            token = strtok(NULL, ",");
            if (!token) { free(copy); continue; }
            char qtys[64];
            strncpy(qtys, token, sizeof(qtys)-1); qtys[sizeof(qtys)-1] = '\0';
            trim_inplace(qtys);

            token = strtok(NULL, ",");
            if (!token) { free(copy); continue; }
            char prices[64];
            strncpy(prices, token, sizeof(prices)-1); prices[sizeof(prices)-1] = '\0';
            trim_inplace(prices);

            long long qty = atoll(qtys);
            double price = atof(prices);

            Item *it = create_item(name, qty, price);
            if (it) inv->items = insert_item_sorted_merge(inv->items, it);
            free(copy);
        }

        *out_head = insert_invoice_sorted(*out_head, inv);
    }

    return 0;
}

/* Read index file (each line = invoice filename) and build invoice list.
   If the index file actually contains invoice data (date / items) instead of
   filenames, detect that and parse invoices inline. */
int read_index_and_build(const char *index_filename, Invoice **out_head)
{
    if (!index_filename || !out_head) return -1;
    FILE *f = fopen(index_filename, "r");
    if (!f) {
        perror(index_filename);
        return -1;
    }

    char line[1024];
    bool found = false;
    /* locate first non-empty line to guess format */
    while (fgets(line, sizeof(line), f)) {
        trim_inplace(line);
        if (line[0] == '\0') continue;
        found = true;
        break;
    }
    if (!found) {
        fclose(f);
        return -1;
    }

    /* If the first non-empty line contains a comma (item line) or is a date,
       treat the whole file as one that contains invoices inline. */
    Date tmp;
    if (strchr(line, ',') != NULL || parse_date(line, &tmp)) {
        /* rewind and parse invoices from this file */
        fseek(f, 0, SEEK_SET);
        int r = read_invoices_from_stream(f, out_head);
        fclose(f);
        return r;
    }

    /* Otherwise, treat file as an index of filenames (one filename per non-empty line) */
    fseek(f, 0, SEEK_SET);
    while (fgets(line, sizeof(line), f)) {
        trim_inplace(line);
        if (line[0] == '\0') continue;
        if (read_single_invoice_file(line, out_head) != 0) {
            fprintf(stderr, "Warning: failed to read invoice file '%s'\n", line);
            /* continue reading others */
        }
    }

    fclose(f);
    return 0;
}

/* Read a single invoice file: first line date, subsequent lines "name, quantity, price" */
int read_single_invoice_file(const char *filename, Invoice **out_head)
{
    if (!filename || !out_head) return -1;
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror(filename);
        return -1;
    }
    char line[1024];
    /* read date line */
    bool got_date = false;
    Date date;
    while (fgets(line, sizeof(line), f)) {
        trim_inplace(line);
        if (line[0] == '\0') continue;
        if (parse_date(line, &date)) {
            got_date = true;
            break;
        } else {
            /* If first non-empty line is not a date, fail */
            fprintf(stderr, "Invalid date line in '%s': %s\n", filename, line);
            fclose(f);
            return -1;
        }
    }
    if (!got_date) {
        fprintf(stderr, "No date found in '%s'\n", filename);
        fclose(f);
        return -1;
    }

    Invoice *inv = create_invoice(&date);
    if (!inv) {
        fclose(f);
        return -1;
    }

    /* parse item lines */
    while (fgets(line, sizeof(line), f)) {
        trim_inplace(line);
        if (line[0] == '\0') continue;
        /* Expect: name, quantity, price */
        char *copy = str_dup(line);
        char *p = copy;
        char *token;

        token = strtok(p, ",");
        if (!token) { free(copy); continue; }
        char name[512];
        strncpy(name, token, sizeof(name)-1); name[sizeof(name)-1] = '\0';
        trim_inplace(name);

        token = strtok(NULL, ",");
        if (!token) { free(copy); continue; }
        char qtys[64];
        strncpy(qtys, token, sizeof(qtys)-1); qtys[sizeof(qtys)-1] = '\0';
        trim_inplace(qtys);

        token = strtok(NULL, ",");
        if (!token) { free(copy); continue; }
        char prices[64];
        strncpy(prices, token, sizeof(prices)-1); prices[sizeof(prices)-1] = '\0';
        trim_inplace(prices);

        long long qty = atoll(qtys);
        double price = atof(prices);

        Item *it = create_item(name, qty, price);
        if (!it) {
            free(copy);
            continue;
        }
        inv->items = insert_item_sorted_merge(inv->items, it);
        free(copy);
    }

    fclose(f);

    /* insert invoice into list */
    *out_head = insert_invoice_sorted(*out_head, inv);
    return 0;
}

/* Query total quantity and spent for an item name (case-insensitive) between from and to inclusive */
void query_range(const Invoice *head, const char *item_name, const Date *from, const Date *to, long long *out_qty, double *out_spent)
{
    if (!out_qty || !out_spent) return;
    *out_qty = 0;
    *out_spent = 0.0;
    for (const Invoice *cur = head; cur != NULL; cur = cur->next) {
        if (compare_date(&cur->date, from) < 0) continue;
        if (compare_date(&cur->date, to) > 0) break; /* list sorted by date ascending */
        for (Item *it = cur->items; it != NULL; it = it->next) {
            if (ci_strcmp(it->name, item_name) == 0) {
                *out_qty += it->qty;
                *out_spent += it->total_cost;
                break; /* each invoice has unique item names after merge */
            }
        }
    }
}

/* Print date in YYYY-MM-DD */
void print_date(const Date *d)
{
    printf("%04d-%02d-%02d", d->y, d->m, d->d);
}

/* Debug: print invoices and items */
void print_invoice_list(const Invoice *head)
{
    for (const Invoice *inv = head; inv != NULL; inv = inv->next) {
        print_date(&inv->date);
        printf(":\n");
        for (Item *it = inv->items; it != NULL; it = it->next) {
            printf("  %s, qty=%lld, total=%.2f\n", it->name, it->qty, it->total_cost);
        }
    }
}