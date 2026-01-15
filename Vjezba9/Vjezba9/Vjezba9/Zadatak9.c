#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 10


/* Node of a binary search tree */
typedef struct Node {
    int data;
    struct Node* left;
    struct Node* right;
} Node;

/* Inserts a value into a binary search tree.
   Duplicates are inserted to the right subtree. Returns new root. */
Node* insert(Node* root, int value);

/* Replaces each node's value with the sum of values in its left and right
   subtrees (sum of all descendants before any replacements). Returns the total
   original sum of the subtree rooted at node. */
int replace(Node* root);

/* Writes inorder traversal of tree to the given file stream. */
void inorder_write(FILE* f, Node* root);

/* Frees all nodes in the tree. */
void free_tree(Node* root);

int main(void)
{
    int i;
    int values[N];
    Node* root = NULL;
    FILE* out;

    /* Seed random generator */
    srand((unsigned)time(NULL));

    /* Generate N random integers in range [10, 90] */
    for (i = 0; i < N; ++i) {
        values[i] = rand() % (90 - 10 + 1) + 10; /* 10..90 inclusive */
    }

    /* Build tree by inserting generated values in order */
    for (i = 0; i < N; ++i) {
        root = insert(root, values[i]);
    }

    /* Open output file and write inorder after insertion */
    out = fopen("inorder_results.txt", "w");
    if (!out) {
        perror("Failed to open output file");
        free_tree(root);
        return EXIT_FAILURE;
    }

    fprintf(out, "Inorder after insert:\n");
    inorder_write(out, root);
    fprintf(out, "\n");

    /* Apply replace operation */
    replace(root);

    /* Write inorder after replace */
    fprintf(out, "Inorder after replace:\n");
    inorder_write(out, root);
    fprintf(out, "\n");

    fclose(out);

    /* Also print to stdout for convenience */
    printf("Generated values:\n");
    for (i = 0; i < N; ++i) printf("%d ", values[i]);
    printf("\nWrote inorder traversals to 'inorder_results.txt'.\n");

    free_tree(root);
    return 0;
}

/* Definitions */

/* insert
   Inserts a value into the binary search tree rooted at `root`.
   If root is NULL, allocates a new node. Duplicates go to the right. */
Node* insert(Node* root, int value)
{
    if (root == NULL) {
        Node* n = (Node*)malloc(sizeof(Node));
        if (!n) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        n->data = value;
        n->left = n->right = NULL;
        return n;
    }

    if (value < root->data) {
        root->left = insert(root->left, value);
    }
    else {
        /* value >= root->data : put duplicates to the right */
        root->right = insert(root->right, value);
    }
    return root;
}

/* replace
   For each node, replaces node->data with the sum of all values in its left
   and right subtrees using the original values (i.e., sum of descendants
   before their values are replaced). Returns the total original sum of the
   subtree rooted at `root`. Implementation uses post-order recursion. */
int replace(Node* root)
{
    if (root == NULL) return 0;

    /* Get total original sums of left and right subtrees */
    int left_sum = replace(root->left);
    int right_sum = replace(root->right);

    int original = root->data;
    /* Set node's value to sum of original descendants */
    root->data = left_sum + right_sum;

    /* Return total original sum of this subtree (original + descendants) */
    return original + root->data;
}

/* inorder_write
   Writes node values in inorder to the given file stream, separated by spaces. */
void inorder_write(FILE* f, Node* root)
{
    if (root == NULL) return;
    inorder_write(f, root->left);
    fprintf(f, "%d ", root->data);
    inorder_write(f, root->right);
}

/* free_tree
   Frees all nodes in the tree to avoid memory leaks. */
void free_tree(Node* root)
{
    if (root == NULL) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}