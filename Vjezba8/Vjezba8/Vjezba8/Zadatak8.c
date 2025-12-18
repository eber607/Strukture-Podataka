#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Basic binary search tree node */
typedef struct Node {
    int key;
    struct Node *left;
    struct Node *right;
} Node;

/* Simple queue node for level-order traversal */
typedef struct QNode {
    Node *treeNode;
    struct QNode *next;
} QNode;

/* Simple queue structure */
typedef struct Queue {
    QNode *front;
    QNode *rear;
} Queue;

/* Function declarations (prototypes) */

/* Create a new tree node with given key */
Node* create_node(int key);

/* Insert key into BST; returns new root */
Node* insert(Node *root, int key);

/* Find a node with given key; returns pointer or NULL */
Node* find(Node *root, int key);

/* Find node with minimum key in subtree */
Node* find_min(Node *root);

/* Delete a node with given key from BST; returns new root */
Node* delete_node(Node *root, int key);

/* Print nodes in inorder (LNR) */
void inorder(Node *root);

/* Print nodes in preorder (NLR) */
void preorder(Node *root);

/* Print nodes in postorder (LRN) */
void postorder(Node *root);

/* Print nodes in level order (breadth-first) */
void level_order(Node *root);

/* Free all nodes in the tree */
void free_tree(Node *root);

/* Queue helpers for level order traversal */
Queue* queue_create(void);
void enqueue(Queue *q, Node *node);
Node* dequeue(Queue *q);
bool queue_is_empty(Queue *q);
void queue_free(Queue *q);

/* Main: menu-driven interface for BST operations */
int main(void) {
    Node *root = NULL;
    int choice;
    int value;

    while (1) {
        printf("\nBinary Search Tree - Menu\n");
        printf("1. Insert\n");
        printf("2. Delete\n");
        printf("3. Find\n");
        printf("4. Print Inorder\n");
        printf("5. Print Preorder\n");
        printf("6. Print Postorder\n");
        printf("7. Print Level Order\n");
        printf("8. Exit\n");
        printf("Choose an option: ");
        if (scanf_s("%d", &choice) != 1) {
            /* Clear invalid input */
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            printf("Invalid input. Try again.\n");
            continue;
        }

        switch (choice) {
            case 1:
                printf("Enter integer to insert: ");
                if (scanf_s("%d", &value) == 1) {
                    root = insert(root, value);
                    printf("%d inserted.\n", value);
                } else {
                    printf("Invalid input.\n");
                    int c; while ((c = getchar()) != '\n' && c != EOF) {}
                }
                break;
            case 2:
                printf("Enter integer to delete: ");
                if (scanf_s("%d", &value) == 1) {
                    root = delete_node(root, value);
                } else {
                    printf("Invalid input.\n");
                    int c; while ((c = getchar()) != '\n' && c != EOF) {}
                }
                break;
            case 3:
                printf("Enter integer to find: ");
                if (scanf_s("%d", &value) == 1) {
                    Node *found = find(root, value);
                    if (found) printf("%d found in the tree.\n", value);
                    else printf("%d not found.\n", value);
                } else {
                    printf("Invalid input.\n");
                    int c; while ((c = getchar()) != '\n' && c != EOF) {}
                }
                break;
            case 4:
                printf("Inorder: ");
                inorder(root);
                printf("\n");
                break;
            case 5:
                printf("Preorder: ");
                preorder(root);
                printf("\n");
                break;
            case 6:
                printf("Postorder: ");
                postorder(root);
                printf("\n");
                break;
            case 7:
                printf("Level order: ");
                level_order(root);
                printf("\n");
                break;
            case 8:
                free_tree(root);
                printf("Exiting.\n");
                return 0;
            default:
                printf("Invalid choice. Try again.\n");
        }
    }
}

/* Function definitions */

/* Create a new tree node with given key */
Node* create_node(int key) {
    Node *n = (Node*)malloc(sizeof(Node));
    if (!n) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }
    n->key = key;
    n->left = n->right = NULL;
    return n;
}

/* Insert key into BST; returns new root */
Node* insert(Node *root, int key) {
    if (root == NULL) {
        return create_node(key);
    }
    if (key < root->key) {
        root->left = insert(root->left, key);
    } else if (key > root->key) {
        root->right = insert(root->right, key);
    } else {
        /* Duplicate keys are ignored */
    }
    return root;
}

/* Find a node with given key; returns pointer or NULL */
Node* find(Node *root, int key) {
    while (root != NULL) {
        if (key == root->key) return root;
        if (key < root->key) root = root->left;
        else root = root->right;
    }
    return NULL;
}

/* Find node with minimum key in subtree */
Node* find_min(Node *root) {
    if (root == NULL) return NULL;
    while (root->left != NULL) root = root->left;
    return root;
}

/* Delete a node with given key from BST; returns new root */
Node* delete_node(Node *root, int key) {
    if (root == NULL) {
        printf("%d not found (tree empty or key absent).\n", key);
        return NULL;
    }
    if (key < root->key) {
        root->left = delete_node(root->left, key);
    } else if (key > root->key) {
        root->right = delete_node(root->right, key);
    } else {
        /* Node found */
        if (root->left == NULL && root->right == NULL) {
            /* No children */
            free(root);
            printf("%d deleted.\n", key);
            return NULL;
        } else if (root->left == NULL) {
            Node *tmp = root->right;
            free(root);
            printf("%d deleted.\n", key);
            return tmp;
        } else if (root->right == NULL) {
            Node *tmp = root->left;
            free(root);
            printf("%d deleted.\n", key);
            return tmp;
        } else {
            /* Two children: replace with inorder successor */
            Node *succ = find_min(root->right);
            root->key = succ->key;
            root->right = delete_node(root->right, succ->key);
        }
    }
    return root;
}

/* Print nodes in inorder (LNR) */
void inorder(Node *root) {
    if (root == NULL) return;
    inorder(root->left);
    printf("%d ", root->key);
    inorder(root->right);
}

/* Print nodes in preorder (NLR) */
void preorder(Node *root) {
    if (root == NULL) return;
    printf("%d ", root->key);
    preorder(root->left);
    preorder(root->right);
}

/* Print nodes in postorder (LRN) */
void postorder(Node *root) {
    if (root == NULL) return;
    postorder(root->left);
    postorder(root->right);
    printf("%d ", root->key);
}

/* Print nodes in level order (breadth-first) */
void level_order(Node *root) {
    if (root == NULL) return;
    Queue *q = queue_create();
    enqueue(q, root);
    while (!queue_is_empty(q)) {
        Node *n = dequeue(q);
        if (n) {
            printf("%d ", n->key);
            if (n->left) enqueue(q, n->left);
            if (n->right) enqueue(q, n->right);
        }
    }
    queue_free(q);
}

/* Free all nodes in the tree (postorder) */
void free_tree(Node *root) {
    if (root == NULL) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

/* Queue functions */

/* Create an empty queue */
Queue* queue_create(void) {
    Queue *q = (Queue*)malloc(sizeof(Queue));
    if (!q) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }
    q->front = q->rear = NULL;
    return q;
}

/* Enqueue a tree node pointer */
void enqueue(Queue *q, Node *node) {
    QNode *qn = (QNode*)malloc(sizeof(QNode));
    if (!qn) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }
    qn->treeNode = node;
    qn->next = NULL;
    if (q->rear == NULL) {
        q->front = q->rear = qn;
    } else {
        q->rear->next = qn;
        q->rear = qn;
    }
}

/* Dequeue and return a tree node pointer (or NULL if empty) */
Node* dequeue(Queue *q) {
    if (q->front == NULL) return NULL;
    QNode *temp = q->front;
    Node *tn = temp->treeNode;
    q->front = q->front->next;
    if (q->front == NULL) q->rear = NULL;
    free(temp);
    return tn;
}

/* Return true if queue is empty */
bool queue_is_empty(Queue *q) {
    return q->front == NULL;
}

/* Free the queue structure (consumes remaining nodes if any) */
void queue_free(Queue *q) {
    while (!queue_is_empty(q)) {
        dequeue(q);
    }
    free(q);
}