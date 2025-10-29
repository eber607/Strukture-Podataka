#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME 50
#define MAX_SURNAME 50

/* Provide compatibility for strtok_r on MSVC */
#if defined(_MSC_VER)
static char* strtok_r_compat(char* str, const char* delim, char** saveptr) {
    /* strtok_s has the same third-parameter semantics on MSVC */
    return strtok_s(str, delim, saveptr);
}
#else
#define strtok_r_compat(str, delim, saveptr) strtok_r(str, delim, saveptr)
#endif

/* Structure that holds person's data */
typedef struct {
    char name[MAX_NAME];
    char surname[MAX_SURNAME];
    int year;
} Person;

/* Node of singly linked list that stores a Person */
typedef struct Node {
    Person data;
    struct Node* next;
} Node;

/* Create a new node on the heap copying the given person and return its pointer. */
Node* create_node(const Person* p) {
    Node* n = (Node*)malloc(sizeof(Node));
    if (!n) return NULL;
    n->data = *p;
    n->next = NULL;
    return n;
}

/* Add a new element to the beginning of the list. */
int push_front(Node** head, const Person* p) {
    Node* n = create_node(p);
    if (!n) return 0;
    n->next = *head;
    *head = n;
    return 1;
}

/* Add a new element to the end of the list. */
int push_back(Node** head, const Person* p) {
    Node* n = create_node(p);
    if (!n) return 0;
    if (*head == NULL) {
        *head = n;
        return 1;
    }
    Node* cur = *head;
    while (cur->next) cur = cur->next;
    cur->next = n;
    return 1;
}

/* Print the whole list to stdout. */
void print_list(const Node* head) {
    const Node* cur = head;
    if (!cur) {
        printf("List is empty.\n");
        return;
    }
    while (cur) {
        printf("Name: %s, Surname: %s, Year: %d\n", cur->data.name, cur->data.surname, cur->data.year);
        cur = cur->next;
    }
}

/* Find the first node with matching surname and return its pointer (NULL if not found). */
Node* find_by_surname(Node* head, const char* surname) {
    Node* cur = head;
    while (cur) {
        if (strcmp(cur->data.surname, surname) == 0) return cur;
        cur = cur->next;
    }
    return NULL;
}

/* Delete the first element with the given surname from the list and free its memory. */
int delete_by_surname(Node** head, const char* surname) {
    Node* cur = *head;
    Node* prev = NULL;
    while (cur) {
        if (strcmp(cur->data.surname, surname) == 0) {
            if (prev) prev->next = cur->next;
            else *head = cur->next;
            free(cur);
            return 1;
        }
        prev = cur;
        cur = cur->next;
    }
    return 0;
}

/* Free all nodes in the list and set head to NULL. */
void free_list(Node** head) {
    Node* cur = *head;
    while (cur) {
        Node* tmp = cur;
        cur = cur->next;
        free(tmp);
    }
    *head = NULL;
}

/* Read a line from stdin into buffer. */
void read_line(char* buf, size_t size) {
    if (fgets(buf, (int)size, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';
}

/* Read a Person from stdin by prompting user for name, surname and year. */
void input_person(Person* p) {
    printf("Enter name: ");
    read_line(p->name, MAX_NAME);
    printf("Enter surname: ");
    read_line(p->surname, MAX_SURNAME);
    char yearbuf[20];
    printf("Enter year of birth: ");
    read_line(yearbuf, sizeof(yearbuf));
    p->year = atoi(yearbuf);
}

/* 
 * Insert a new element dynamically after the first node with the given surname.
 * Returns 1 on success, 0 on failure (allocation error or not found).
 */
int insert_after(Node* head, const char* surname, const Person* p) {
    Node* cur = head;
    while (cur) {
        if (strcmp(cur->data.surname, surname) == 0) {
            Node* n = create_node(p);
            if (!n) return 0;
            n->next = cur->next;
            cur->next = n;
            return 1;
        }
        cur = cur->next;
    }
    return 0; /* not found */
}

/* 
 * Insert a new element dynamically before the first node with the given surname.
 * If the matching node is the head, the head pointer is updated.
 * Returns 1 on success, 0 on failure (allocation error or not found).
 */
int insert_before(Node** head, const char* surname, const Person* p) {
    if (head == NULL) return 0;
    Node* cur = *head;
    Node* prev = NULL;
    while (cur) {
        if (strcmp(cur->data.surname, surname) == 0) {
            /* insert before cur */
            Node* n = create_node(p);
            if (!n) return 0;
            n->next = cur;
            if (prev) prev->next = n;
            else *head = n;
            return 1;
        }
        prev = cur;
        cur = cur->next;
    }
    return 0; /* not found */
}

/* Helper: compare two persons by surname, then by name, then by year. */
/* Returns negative if a < b, 0 if equal, positive if a > b */
static int person_compare(const Person* a, const Person* b) {
    int c = strcmp(a->surname, b->surname);
    if (c != 0) return c;
    c = strcmp(a->name, b->name);
    if (c != 0) return c;
    return a->year - b->year;
}

/* Helper: merge two sorted lists and return head of merged list. */
static Node* merge_sorted(Node* a, Node* b) {
    Node dummy;
    Node* tail = &dummy;
    dummy.next = NULL;
    while (a && b) {
        if (person_compare(&a->data, &b->data) <= 0) {
            tail->next = a;
            a = a->next;
        } else {
            tail->next = b;
            b = b->next;
        }
        tail = tail->next;
    }
    tail->next = (a ? a : b);
    return dummy.next;
}

/* Helper: split list into two halves; returns head of second half. */
static Node* split_list(Node* head) {
    if (!head || !head->next) return NULL;
    Node* slow = head;
    Node* fast = head->next;
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    Node* second = slow->next;
    slow->next = NULL;
    return second;
}

/* 
 * Sort the linked list by persons' surnames (ascending).
 * Uses merge sort for O(n log n) time. The head pointer is updated.
 */
void sort_by_surname(Node** head) {
    if (!head || !*head || !(*head)->next) return;
    Node* first = *head;
    Node* second = split_list(first);
    sort_by_surname(&first);
    sort_by_surname(&second);
    *head = merge_sorted(first, second);
}

/* 
 * Write the list to a text file.
 * Each person is written on a separate line in the format:
 * name;surname;year\n
 * Returns 1 on success, 0 on failure (e.g., could not open file).
 */
int write_list_to_file(const Node* head, const char* filename) {
    if (!filename) return 0;
    FILE* f = fopen(filename, "w");
    if (!f) return 0;
    const Node* cur = head;
    while (cur) {
        /* Use semicolon as delimiter to allow spaces in names */
        fprintf(f, "%s;%s;%d\n", cur->data.name, cur->data.surname, cur->data.year);
        cur = cur->next;
    }
    fclose(f);
    return 1;
}

/* 
 * Read a list from a text file previously written with write_list_to_file.
 * Existing list (if any) pointed to by head is freed first.
 * File format expected: name;surname;year per line.
 * Returns number of persons read on success, -1 on failure (e.g., cannot open file).
 */
int read_list_from_file(Node** head, const char* filename) {
    if (!head || !filename) return -1;
    FILE* f = fopen(filename, "r");
    if (!f) return -1;
    /* Clear existing list */
    free_list(head);
    char line[256];
    int count = 0;
    while (fgets(line, sizeof(line), f)) {
        /* remove trailing newline */
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';
        char* token;
        char* saveptr = NULL;
        token = strtok_r_compat(line, ";", &saveptr);
        if (!token) continue;
        Person p;
        /* name */
        strncpy(p.name, token, MAX_NAME - 1);
        p.name[MAX_NAME - 1] = '\0';
        /* surname */
            token = strtok_r_compat(NULL, ";", &saveptr);
        if (!token) continue;
        strncpy(p.surname, token, MAX_SURNAME - 1);
        p.surname[MAX_SURNAME - 1] = '\0';
        /* year */
        token = strtok_r_compat(NULL, ";", &saveptr);
        if (!token) continue;
        p.year = atoi(token);
        /* append to list */
        if (!push_back(head, &p)) {
            /* allocation failed: clean up and return current count */
            fclose(f);
            return count;
        }
        count++;
    }
    fclose(f);
    return count;
}

/* Simple menu demonstration that uses the above functions. */
int main(void) {
    Node* head = NULL;
    for (;;) {
        printf("\nMenu:\n");
        printf("1 - Add person to start\n");
        printf("2 - Add person to end\n");
        printf("3 - Print list\n");
        printf("4 - Find by surname\n");
        printf("5 - Delete by surname\n");
        printf("6 - Insert after surname\n");
        printf("7 - Insert before surname\n");
        printf("8 - Sort by surname\n");
        printf("9 - Write list to file\n");
        printf("10 - Read list from file\n");
        printf("0 - Exit\n");
        printf("Choice: ");
        char choicebuf[10];
        read_line(choicebuf, sizeof(choicebuf));
        int choice = atoi(choicebuf);

        if (choice == 0) break;

        if (choice == 1) {
            Person p;
            input_person(&p);
            if (push_front(&head, &p)) printf("Added to start.\n");
            else printf("Allocation failed.\n");
        }
        else if (choice == 2) {
            Person p;
            input_person(&p);
            if (push_back(&head, &p)) printf("Added to end.\n");
            else printf("Allocation failed.\n");
        }
        else if (choice == 3) {
            print_list(head);
        }
        else if (choice == 4) {
            char s[MAX_SURNAME];
            printf("Enter surname to find: ");
            read_line(s, MAX_SURNAME);
            Node* found = find_by_surname(head, s);
            if (found) printf("Found: %s %s, %d\n", found->data.name, found->data.surname, found->data.year);
            else printf("Not found.\n");
        }
        else if (choice == 5) {
            char s[MAX_SURNAME];
            printf("Enter surname to delete: ");
            read_line(s, MAX_SURNAME);
            if (delete_by_surname(&head, s)) printf("Deleted.\n");
            else printf("Not found.\n");
        }
        else if (choice == 6) {
            char s[MAX_SURNAME];
            printf("Enter surname after which to insert: ");
            read_line(s, MAX_SURNAME);
            Person p;
            input_person(&p);
            if (insert_after(head, s, &p)) printf("Inserted after %s.\n", s);
            else printf("Insert failed or surname not found.\n");
        }
        else if (choice == 7) {
            char s[MAX_SURNAME];
            printf("Enter surname before which to insert: ");
            read_line(s, MAX_SURNAME);
            Person p;
            input_person(&p);
            if (insert_before(&head, s, &p)) printf("Inserted before %s.\n", s);
            else printf("Insert failed or surname not found.\n");
        }
        else if (choice == 8) {
            sort_by_surname(&head);
            printf("List sorted by surname.\n");
        }
        else if (choice == 9) {
            char fname[260];
            printf("Enter filename to write to: ");
            read_line(fname, sizeof(fname));
            if (write_list_to_file(head, fname)) printf("List written to %s\n", fname);
            else printf("Failed to write to %s\n", fname);
        }
        else if (choice == 10) {
            char fname[260];
            printf("Enter filename to read from: ");
            read_line(fname, sizeof(fname));
            int read = read_list_from_file(&head, fname);
            if (read >= 0) printf("Read %d records from %s\n", read, fname);
            else printf("Failed to read from %s\n", fname);
        }
        else {
            printf("Invalid choice.\n");
        }
    }

    free_list(&head);
    return 0;
}