#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME 50
#define MAX_SURNAME 50

/* Structure that holds person's data */
typedef struct {
    char name[MAX_NAME];
    char surname[MAX_SURNAME];
    int year;
} Person;

/* Node of singly linked list that stores a Person */
typedef struct Node {
    Person data;
    struct Node *next;
} Node;

/* Create a new node on the heap copying the given person and return its pointer. */
Node* create_node(const Person *p) {
    Node *n = (Node*)malloc(sizeof(Node));
    if (!n) return NULL;
    n->data = *p;
    n->next = NULL;
    return n;
}

/* Add a new element to the beginning of the list. */
int push_front(Node **head, const Person *p) {
    Node *n = create_node(p);
    if (!n) return 0;
    n->next = *head;
    *head = n;
    return 1;
}

/* Add a new element to the end of the list. */
int push_back(Node **head, const Person *p) {
    Node *n = create_node(p);
    if (!n) return 0;
    if (*head == NULL) {
        *head = n;
        return 1;
    }
    Node *cur = *head;
    while (cur->next) cur = cur->next;
    cur->next = n;
    return 1;
}

/* Print the whole list to stdout. */
void print_list(const Node *head) {
    const Node *cur = head;
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
Node* find_by_surname(Node *head, const char *surname) {
    Node *cur = head;
    while (cur) {
        if (strcmp(cur->data.surname, surname) == 0) return cur;
        cur = cur->next;
    }
    return NULL;
}

/* Delete the first element with the given surname from the list and free its memory. */
int delete_by_surname(Node **head, const char *surname) {
    Node *cur = *head;
    Node *prev = NULL;
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
void free_list(Node **head) {
    Node *cur = *head;
    while (cur) {
        Node *tmp = cur;
        cur = cur->next;
        free(tmp);
    }
    *head = NULL;
}

/* Read a line from stdin into buffer. */
void read_line(char *buf, size_t size) {
    if (fgets(buf, (int)size, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';
}

/* Read a Person from stdin by prompting user for name, surname and year. */
void input_person(Person *p) {
    printf("Enter name: ");
    read_line(p->name, MAX_NAME);
    printf("Enter surname: ");
    read_line(p->surname, MAX_SURNAME);
    char yearbuf[20];
    printf("Enter year of birth: ");
    read_line(yearbuf, sizeof(yearbuf));
    p->year = atoi(yearbuf);
}

/* Simple menu demonstration that uses the above functions. */
int main(void) {
    Node *head = NULL;
    for (;;) {
        printf("\nMenu:\n");
        printf("1 - Add person to start\n");
        printf("2 - Add person to end\n");
        printf("3 - Print list\n");
        printf("4 - Find by surname\n");
        printf("5 - Delete by surname\n");
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
        } else if (choice == 2) {
            Person p;
            input_person(&p);
            if (push_back(&head, &p)) printf("Added to end.\n");
            else printf("Allocation failed.\n");
        } else if (choice == 3) {
            print_list(head);
        } else if (choice == 4) {
            char s[MAX_SURNAME];
            printf("Enter surname to find: ");
            read_line(s, MAX_SURNAME);
            Node *found = find_by_surname(head, s);
            if (found) printf("Found: %s %s, %d\n", found->data.name, found->data.surname, found->data.year);
            else printf("Not found.\n");
        } else if (choice == 5) {
            char s[MAX_SURNAME];
            printf("Enter surname to delete: ");
            read_line(s, MAX_SURNAME);
            if (delete_by_surname(&head, s)) printf("Deleted.\n");
            else printf("Not found.\n");
        } else {
            printf("Invalid choice.\n");
        }
    }

    free_list(&head);
    return 0;
}