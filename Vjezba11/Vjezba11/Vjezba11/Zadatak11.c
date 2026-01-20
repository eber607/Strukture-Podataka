#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TABLE_SIZE 11
#define LINE_BUF 1024


/* Data structures:
 * Country: node in a sorted linked list stored in a hash table bucket.
 * City: node in a binary search tree sorted by population (desc), then name (asc).
 */

typedef struct City {
    char *name;
    int population;
    struct City *left;
    struct City *right;
} City;

typedef struct Country {
    char *name;
    City *cities;            // root of BST of cities
    struct Country *next;    // next country in bucket's linked list (sorted by country name asc)
} Country;

/* Hash table: array of pointers to Country (heads of sorted linked lists) */
Country *hash_table[TABLE_SIZE] = {0};

/* Helper: allocate and duplicate a string (portable replacement for strdup) */
static char *strdup_s(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *d = (char *)malloc(n);
    if (d) memcpy(d, s, n);
    return d;
}

/* Helper: trim leading and trailing whitespace in place */
static void trim(char *s) {
    if (!s) return;
    // trim leading
    char *p = s;
    while (isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    // trim trailing
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) s[--len] = '\0';
}

/* Compute key: sum ASCII of first five characters of country name, mod TABLE_SIZE */
static int compute_key(const char *country_name) {
    int sum = 0;
    for (int i = 0; i < 5 && country_name[i] != '\0'; ++i) {
        sum += (unsigned char)country_name[i];
    }
    return sum % TABLE_SIZE;
}

/* Create new city node */
static City *create_city(const char *name, int population) {
    City *c = (City *)malloc(sizeof(City));
    if (!c) return NULL;
    c->name = strdup_s(name);
    c->population = population;
    c->left = c->right = NULL;
    return c;
}

/* Insert city into BST sorted by population (desc), then name (asc).
 * If a city with identical name and population exists, do not insert duplicate.
 */
static City *insert_city(City *root, const char *name, int population) {
    if (!root) return create_city(name, population);
    if (population > root->population) {
        root->left = insert_city(root->left, name, population);
    } else if (population < root->population) {
        root->right = insert_city(root->right, name, population);
    } else {
        int cmp = strcmp(name, root->name);
        if (cmp < 0) {
            root->left = insert_city(root->left, name, population);
        } else if (cmp > 0) {
            root->right = insert_city(root->right, name, population);
        } else {
            // identical city (same name and pop): skip
        }
    }
    return root;
}

/* Create and insert country into bucket's linked list, keeping list sorted by country name (asc).
 * If country already exists, return pointer to existing node.
 */
static Country *insert_country_sorted(Country **bucket_head, const char *country_name) {
    Country *prev = NULL;
    Country *cur = *bucket_head;
    while (cur && strcmp(cur->name, country_name) < 0) {
        prev = cur;
        cur = cur->next;
    }
    if (cur && strcmp(cur->name, country_name) == 0) {
        // already exists
        return cur;
    }
    Country *node = (Country *)malloc(sizeof(Country));
    if (!node) {
        fprintf(stderr, "Error: Memory allocation failed for country '%s'\n", country_name);
        return NULL;
    }
    node->name = strdup_s(country_name);
    node->cities = NULL;
    // insert between prev and cur
    node->next = cur;
    if (prev) prev->next = node;
    else *bucket_head = node;
    return node;
}

/* Read cities from a file with lines in format: city_name,number
 * and insert them into the country's city BST.
 */
static void load_cities_for_country(Country *country, const char *city_filename) {
    FILE *f = fopen(city_filename, "r");
    if (!f) {
        fprintf(stderr, "Warning: cannot open city file '%s' for country '%s'\n", city_filename, country->name);
        return;
    }
    char line[LINE_BUF];
    while (fgets(line, sizeof(line), f)) {
        trim(line);
        if (line[0] == '\0') continue;
        // find last comma (in case city names contain commas, but spec suggests single comma)
        char *comma = strrchr(line, ',');
        if (!comma) continue;
        *comma = '\0';
        char *city_name = line;
        char *pop_str = comma + 1;
        trim(city_name);
        trim(pop_str);
        int pop = atoi(pop_str);
        if (city_name[0] == '\0') continue;
        country->cities = insert_city(country->cities, city_name, pop);
    }
    fclose(f);
}

/* Insert a country (and load its cities) by reading a line describing it:
 * each line in drzave.txt is assumed to have the country name followed by the city filename.
 * To support country names with spaces, the last token is interpreted as the filename, the rest as the country name.
 */
static void process_country_line(char *line) {
    trim(line);
    if (line[0] == '\0') return;
    // find last whitespace separating country name and filename
    int len = (int)strlen(line);
    int pos = len - 1;
    while (pos >= 0 && !isspace((unsigned char)line[pos])) pos--;
    if (pos < 0) {
        fprintf(stderr, "Skipping malformed line (no filename): '%s'\n", line);
        return;
    }
    // split
    char filename[LINE_BUF];
    strcpy(filename, line + pos + 1);
    line[pos] = '\0';
    trim(line);
    trim(filename);
    if (line[0] == '\0' || filename[0] == '\0') {
        fprintf(stderr, "Skipping malformed line: '%s' '%s'\n", line, filename);
        return;
    }
    int key = compute_key(line);
    Country *country = insert_country_sorted(&hash_table[key], line);
    // load cities into country's BST
    load_cities_for_country(country, filename);
}

/* Print cities in-order to produce list sorted by population desc then name asc */
static void print_cities_inorder(const City *root, int indent) {
    if (!root) return;
    // left -> node -> right yields population descending because left contains larger pops
    print_cities_inorder(root->left, indent + 2);
    for (int i = 0; i < indent; ++i) putchar(' ');
    printf("%s, %d\n", root->name, root->population);
    print_cities_inorder(root->right, indent + 2);
}

/* Print full hash table contents: for each bucket, print countries and their cities */
static void print_table() {
    printf("Hash table contents (TABLE_SIZE = %d):\n", TABLE_SIZE);
    for (int i = 0; i < TABLE_SIZE; ++i) {
        printf("Bucket %d:\n", i);
        Country *c = hash_table[i];
        if (!c) {
            printf("  (empty)\n");
            continue;
        }
        while (c) {
            printf("  Country: %s\n", c->name);
            print_cities_inorder(c->cities, 4);
            c = c->next;
        }
    }
}

/* Print cities of a country with population > threshold.
 * Uses tree structure to prune traversal where possible.
 */
static void print_cities_above_threshold(const City *root, int threshold) {
    if (!root) return;
    // left subtree has >= populations, so traverse left first
    if (root->left) print_cities_above_threshold(root->left, threshold);
    if (root->population > threshold) {
        printf("%s, %d\n", root->name, root->population);
        if (root->right) print_cities_above_threshold(root->right, threshold);
    } else {
        // root->population <= threshold, nodes in right are <= root->population so they are all <= threshold; skip right
    }
}

/* Free memory used by cities tree */
static void free_cities(City *root) {
    if (!root) return;
    free_cities(root->left);
    free_cities(root->right);
    free(root->name);
    free(root);
}

/* Free countries and their city trees in the entire table */
static void free_table() {
    for (int i = 0; i < TABLE_SIZE; ++i) {
        Country *c = hash_table[i];
        while (c) {
            Country *next = c->next;
            free_cities(c->cities);
            free(c->name);
            free(c);
            c = next;
        }
        hash_table[i] = NULL;
    }
}

/* Main: load 'drzave.txt', build hash table, print contents, and allow user search */
int main(void) {
    const char *master_filename = "drzave.txt";
    FILE *master = fopen(master_filename, "r");
    if (!master) {
        fprintf(stderr, "Error: cannot open '%s'\n", master_filename);
        return 1;
    }
    char line[LINE_BUF];
    while (fgets(line, sizeof(line), master)) {
        process_country_line(line);
    }
    fclose(master);

    print_table();

    // Interactive search loop: allow the user to lookup a country and enter threshold.
    printf("\nSearch cities of a country with population greater than a threshold.\n");
    printf("Enter country name (or empty line to exit):\n");
    while (1) {
        printf("> ");
        if (!fgets(line, sizeof(line), stdin)) break;
        trim(line);
        if (line[0] == '\0') break;
        char country_name[LINE_BUF];
        strcpy(country_name, line);
        int key = compute_key(country_name);
        Country *c = hash_table[key];
        while (c && strcmp(c->name, country_name) != 0) c = c->next;
        if (!c) {
            printf("Country '%s' not found in table.\n", country_name);
            continue;
        }
        printf("Enter population threshold (integer): ");
        if (!fgets(line, sizeof(line), stdin)) break;
        trim(line);
        int threshold = atoi(line);
        printf("Cities in '%s' with population > %d:\n", c->name, threshold);
        print_cities_above_threshold(c->cities, threshold);
    }

    free_table();
    printf("Exiting.\n");
    return 0;
}