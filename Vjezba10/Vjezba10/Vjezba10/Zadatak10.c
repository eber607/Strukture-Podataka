#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LINE_BUF 512

/* Simple strdup replacement for portability */
static char *my_strdup(const char *s)
{
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *d = (char*)malloc(n);
    if (d) memcpy(d, s, n);
    return d;
}

/* Trim leading/trailing whitespace in-place */
static char *trim(char *s)
{
    if (!s) return s;
    char *end;
    while (isspace((unsigned char)*s)) s++;
    if (*s == 0) return s;
    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    return s;
}

/* Data structures */
typedef struct CityTreeNode {
    char *name;
    long population;
    struct CityTreeNode *left;
    struct CityTreeNode *right;
} CityTreeNode;

typedef struct CityListNode {
    char *name;
    long population;
    struct CityListNode *next;
} CityListNode;

typedef struct CountryListNode {
    char *name;
    CityTreeNode *cities; /* tree of cities */
    struct CountryListNode *next;
} CountryListNode;

typedef struct CountryTreeNode {
    char *name;
    CityListNode *cities; /* list of cities */
    struct CountryTreeNode *left;
    struct CountryTreeNode *right;
} CountryTreeNode;

/* Compare cities: primary population (ascending), then name (ascending).
   Returns <0 if (p1,name1) < (p2,name2), 0 if equal, >0 otherwise. */
static int compare_city(const char *n1, long p1, const char *n2, long p2)
{
    if (p1 < p2) return -1;
    if (p1 > p2) return 1;
    return strcmp(n1 ? n1 : "", n2 ? n2 : "");
}

/* Insert city into BST (used for country list nodes).
   If duplicate (same name and population) ignore insertion. */
static CityTreeNode *insert_city_tree(CityTreeNode *root, const char *name, long population)
{
    if (!root) {
        CityTreeNode *node = (CityTreeNode*)malloc(sizeof(CityTreeNode));
        node->name = my_strdup(name);
        node->population = population;
        node->left = node->right = NULL;
        return node;
    }
    int cmp = compare_city(name, population, root->name, root->population);
    if (cmp < 0)
        root->left = insert_city_tree(root->left, name, population);
    else if (cmp > 0)
        root->right = insert_city_tree(root->right, name, population);
    else {
        /* consider equal -> do nothing */
    }
    return root;
}

/* Insert city into sorted linked list (used for country tree nodes).
   List is sorted by population ascending, then by name ascending. */
static CityListNode *insert_city_list_sorted(CityListNode *head, const char *name, long population)
{
    CityListNode *newn = (CityListNode*)malloc(sizeof(CityListNode));
    newn->name = my_strdup(name);
    newn->population = population;
    newn->next = NULL;

    if (!head) return newn;

    CityListNode *prev = NULL, *curr = head;
    while (curr) {
        int cmp = compare_city(name, population, curr->name, curr->population);
        if (cmp <= 0) break;
        prev = curr;
        curr = curr->next;
    }
    if (!prev) {
        newn->next = head;
        return newn;
    } else {
        prev->next = newn;
        newn->next = curr;
        return head;
    }
}

/* Free city tree */
static void free_city_tree(CityTreeNode *root)
{
    if (!root) return;
    free_city_tree(root->left);
    free_city_tree(root->right);
    free(root->name);
    free(root);
}

/* Free city list */
static void free_city_list(CityListNode *head)
{
    while (head) {
        CityListNode *t = head;
        head = head->next;
        free(t->name);
        free(t);
    }
}

/* Insert a country into a sorted linked list of countries (by country name ascending).
   The node stores a previously-built city tree. */
static CountryListNode *insert_country_list_sorted(CountryListNode *head, const char *countryName, CityTreeNode *cities)
{
    CountryListNode *newn = (CountryListNode*)malloc(sizeof(CountryListNode));
    newn->name = my_strdup(countryName);
    newn->cities = cities;
    newn->next = NULL;

    if (!head) return newn;

    CountryListNode *prev = NULL, *curr = head;
    while (curr && strcmp(curr->name, countryName) < 0) {
        prev = curr;
        curr = curr->next;
    }
    if (!prev) {
        newn->next = head;
        return newn;
    } else {
        prev->next = newn;
        newn->next = curr;
        return head;
    }
}

/* Insert a country into a BST of countries (by country name ascending).
   The node stores a previously-built city linked list. */
static CountryTreeNode *insert_country_tree(CountryTreeNode *root, const char *countryName, CityListNode *cities)
{
    if (!root) {
        CountryTreeNode *node = (CountryTreeNode*)malloc(sizeof(CountryTreeNode));
        node->name = my_strdup(countryName);
        node->cities = cities;
        node->left = node->right = NULL;
        return node;
    }
    int cmp = strcmp(countryName, root->name);
    if (cmp < 0)
        root->left = insert_country_tree(root->left, countryName, cities);
    else if (cmp > 0)
        root->right = insert_country_tree(root->right, countryName, cities);
    else {
        /* country already exists -- free incoming cities and ignore insertion */
        free_city_list(cities);
    }
    return root;
}

/* Read a city file and build a city tree (used for linked-list country nodes).
   Expected line format: city_name,number
*/
static CityTreeNode *build_city_tree_from_file(const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Warning: Cannot open city file '%s'\n", filename);
        return NULL;
    }
    char buf[LINE_BUF];
    CityTreeNode *root = NULL;
    while (fgets(buf, sizeof(buf), f)) {
        char *line = trim(buf);
        if (*line == '\0') continue;
        char *comma = strchr(line, ',');
        if (!comma) continue;
        *comma = '\0';
        char *name = trim(line);
        char *popstr = trim(comma + 1);
        long pop = strtol(popstr, NULL, 10);
        if (name && *name)
            root = insert_city_tree(root, name, pop);
    }
    fclose(f);
    return root;
}

/* Read a city file and build a city linked list (used for country tree nodes). */
static CityListNode *build_city_list_from_file(const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Warning: Cannot open city file '%s'\n", filename);
        return NULL;
    }
    char buf[LINE_BUF];
    CityListNode *head = NULL;
    while (fgets(buf, sizeof(buf), f)) {
        char *line = trim(buf);
        if (*line == '\0') continue;
        char *comma = strchr(line, ',');
        if (!comma) continue;
        *comma = '\0';
        char *name = trim(line);
        char *popstr = trim(comma + 1);
        long pop = strtol(popstr, NULL, 10);
        if (name && *name)
            head = insert_city_list_sorted(head, name, pop);
    }
    fclose(f);
    return head;
}

/* Print city tree in-order (ascending by population, then name). */
static void print_city_tree_inorder(CityTreeNode *root)
{
    if (!root) return;
    print_city_tree_inorder(root->left);
    printf("    %s (%ld)\n", root->name, root->population);
    print_city_tree_inorder(root->right);
}

/* Print city linked list */
static void print_city_list(CityListNode *head)
{
    for (CityListNode *p = head; p; p = p->next)
        printf("    %s (%ld)\n", p->name, p->population);
}

/* Print countries and their city trees from the country linked list */
static void print_country_list(CountryListNode *head)
{
    puts("Countries (linked list) with city BSTs:");
    for (CountryListNode *p = head; p; p = p->next) {
        printf("Country: %s\n", p->name);
        if (p->cities)
            print_city_tree_inorder(p->cities);
        else
            printf("    (no cities)\n");
    }
    puts("");
}

/* Print countries and their city lists from the country BST (in-order) */
static void print_country_tree_inorder(CountryTreeNode *root)
{
    if (!root) return;
    print_country_tree_inorder(root->left);
    printf("Country: %s\n", root->name);
    if (root->cities)
        print_city_list(root->cities);
    else
        printf("    (no cities)\n");
    print_country_tree_inorder(root->right);
}

/* Find country in country BST by name */
static CountryTreeNode *find_country_tree(CountryTreeNode *root, const char *name)
{
    if (!root) return NULL;
    int cmp = strcmp(name, root->name);
    if (cmp == 0) return root;
    if (cmp < 0) return find_country_tree(root->left, name);
    return find_country_tree(root->right, name);
}

/* Find country in linked list by name */
static CountryListNode *find_country_list(CountryListNode *head, const char *name)
{
    for (CountryListNode *p = head; p; p = p->next)
        if (strcmp(p->name, name) == 0)
            return p;
    return NULL;
}

/* Free country list and its city trees */
static void free_country_list(CountryListNode *head)
{
    while (head) {
        CountryListNode *t = head;
        head = head->next;
        free(t->name);
        free_city_tree(t->cities);
        free(t);
    }
}

/* Free country tree and its city lists (post-order) */
static void free_country_tree(CountryTreeNode *root)
{
    if (!root) return;
    free_country_tree(root->left);
    free_country_tree(root->right);
    free(root->name);
    free_city_list(root->cities);
    free(root);
}

/* Print cities in tree with population > threshold */
static void print_city_tree_threshold(CityTreeNode *node, long thr, int *fnd) {
    if (!node) return;
    print_city_tree_threshold(node->left, thr, fnd);
    if (node->population > thr) {
        printf("  %s (%ld)\n", node->name, node->population);
        *fnd = 1;
    }
    print_city_tree_threshold(node->right, thr, fnd);
}

/* MAIN: read "drzave.txt", build data structures, print them and allow search */
int main(void)
{
    const char *countries_file = "drzave.txt";
    FILE *f = fopen(countries_file, "r");
    if (!f) {
        fprintf(stderr, "Error: Cannot open '%s'\n", countries_file);
        return 1;
    }

    CountryListNode *countryList = NULL;
    CountryTreeNode *countryTree = NULL;
    char line[LINE_BUF];

    while (fgets(line, sizeof(line), f)) {
        char *ln = trim(line);
        if (*ln == '\0') continue; /* skip empty */
        /* Expect: CountryName CityFileName */
        char *token = strtok(ln, " \t\r\n");
        if (!token) continue;
        char *countryName = token;
        token = strtok(NULL, " \t\r\n");
        if (!token) {
            fprintf(stderr, "Warning: missing city filename for country '%s'\n", countryName);
            continue;
        }
        char *cityFilename = token;

        /* Build city structures separately for each country node type */
        CityTreeNode *cityTree = build_city_tree_from_file(cityFilename);
        CityListNode *cityList = build_city_list_from_file(cityFilename);

        countryList = insert_country_list_sorted(countryList, countryName, cityTree);
        countryTree = insert_country_tree(countryTree, countryName, cityList);
    }
    fclose(f);

    /* Print both structures */
    print_country_list(countryList);
    puts("Countries (BST) with city lists:");
    print_country_tree_inorder(countryTree);
    puts("");

    /* Allow user to search for cities of a particular country with population > threshold */
    char input[LINE_BUF];
    printf("Enter country name to search: ");
    if (!fgets(input, sizeof(input), stdin)) {
        /* clean up */
        free_country_list(countryList);
        free_country_tree(countryTree);
        return 0;
    }
    trim(input);
    char *searchCountry = input;
    printf("Enter population threshold (print cities with population > threshold): ");
    if (!fgets(input, sizeof(input), stdin)) {
        free_country_list(countryList);
        free_country_tree(countryTree);
        return 0;
    }
    long threshold = strtol(trim(input), NULL, 10);

    /* Search in country tree first */
    CountryTreeNode *ct = find_country_tree(countryTree, searchCountry);
    if (ct) {
        printf("Results (from country BST -> city list) for '%s' with population > %ld:\n", searchCountry, threshold);
        int found = 0;
        for (CityListNode *c = ct->cities; c; c = c->next) {
            if (c->population > threshold) {
                printf("  %s (%ld)\n", c->name, c->population);
                found = 1;
            }
        }
        if (!found) printf("  (no cities meet the criteria)\n");
    } else {
        /* fallback: try linked list */
        CountryListNode *cl = find_country_list(countryList, searchCountry);
        if (cl) {
            printf("Results (from country LIST -> city BST) for '%s' with population > %ld:\n", searchCountry, threshold);
            int found = 0;
            /* traverse city BST and print those > threshold (inorder) */
            /* helper stackless approach: use recursion with printing condition */
            /* define nested function? not in C, so create small recursive printer here */
            /* We print nodes with population > threshold */
            /* Use a simple recursion that checks each node. */
            int foundFlag = 0;
            print_city_tree_threshold(cl->cities, threshold, &foundFlag);
            if (!foundFlag) printf("  (no cities meet the criteria)\n");
        } else {
            printf("Country '%s' not found in either structure.\n", searchCountry);
        }
    }

    /* Cleanup */
    free_country_list(countryList);
    free_country_tree(countryTree);

    return 0;
}