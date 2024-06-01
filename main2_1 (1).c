#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define SDELIM "==STAGE %d============================\n" // stage delimiter
#define MDELIM "-------------------------------------\n" // delimiter of -'s
#define THEEND "==THE END============================\n" // end message
#define NOSFMT "Number of statements: %d\n" // no. of statements
#define NOCFMT "Number of characters: %d\n" // no. of chars
#define NPSFMT "Number of states: %d\n" // no. of states
#define TFQFMT "Total frequency: %d\n" // total frequency
#define ELLIPSES "..." // ellipses

typedef unsigned int uint;
typedef struct Arc Arc;
typedef struct Output Output;

typedef struct Node {
    uint id;
    uint freq;
    Output* outputs;
} Node;

typedef struct Arc {
    char* pattern;
    Node* node;
    Arc* next;
} Arc;

typedef struct Output {
    Arc* head;
    Arc* tail;
} Output;

typedef struct Graph {
    Node* root;
    uint nodes_count;
} Graph;

Node* node_init(uint id);
Arc* arc_init(char* pattern, Node* node);
Graph* graph_init();
void graph_free(Graph* graph);
Output* output_init();
void output_add(Output* output, Arc* arc);
Arc* node_find_arc(Node* node, char c, uint pos);
Arc* node_get_arc(Node* node);
void graph_expand(Graph* graph, char start_char);
uint graph_stats_freq(Node* node);
void compress(Graph* graph);
uint compression_hlper(Graph* graph, Node* x, uint times);

char* char_to_string(char c);
int is_leaf(Node* node);
int print_reamining(char* str, int reamining_len);
int compressible(Node* x);
char* concat_str(char* str1, char* str2);
void sort_arc(Arc** value, uint size);
Arc** output_to_array(Output* output, uint len);

Graph* train();
void generate(Graph* graph);
int infer(Graph* graph, char c);

int main()
{
    // Stage 0
    printf(SDELIM, 0);
    Graph* graph = train();

    printf(NOSFMT, graph->root->freq);
    printf(NOCFMT, graph_stats_freq(graph->root));
    printf(NPSFMT, graph->nodes_count);

    // Stage 1
    printf(SDELIM, 1);
    generate(graph);

    // Stage 2
    printf(SDELIM, 2);
    compress(graph);

    printf(NPSFMT, graph->nodes_count);
    printf(TFQFMT, graph_stats_freq(graph->root));
    printf(MDELIM);

    generate(graph);

    printf(THEEND);
    // End
    graph_free(graph);
    return EXIT_SUCCESS;
}

// Build a graph based on the statements in part 1
Graph* train()
{
    Graph* graph = graph_init();
    char c;
    // The \n is end of the Stage
    while ((c = getchar()) != EOF && c != '\n') {
        // Expand the graph with one statment
        // c is the first char of the statment
        graph_expand(graph, c);
    }

    return graph;
}

void generate(Graph* graph)
{
    char c;
    // The \n is end of the Stage
    while ((c = getchar()) != EOF && c != '\n') {
        // If the prompt is not supported
        if (!infer(graph, c)) {
            // Skip current line
            // The \n is end of the line
            while ((c = getchar()) != EOF && c != '\n') { }
        }
    }
}

void compress(Graph* graph)
{
    uint times;
    scanf("%d\n", &times);

    compression_hlper(graph, graph->root, times);
}

// Expand the graph with one statment
void graph_expand(Graph* graph, char c)
{
    Node* curr_node = graph->root;
    int is_first = 1; // Flag for the first char
    // The \n is end of the statment
    while (is_first || (c = getchar()) != '\n') {
        // Through node
        curr_node->freq++;

        // Find arc
        Arc* arc = node_find_arc(curr_node, c, 0);
        if (arc == NULL) {
            char* c_str = char_to_string(c);
            // Create new arc
            Node* new_node = node_init(graph->nodes_count++);
            arc = arc_init(c_str, new_node);
            output_add(curr_node->outputs, arc);
            curr_node = new_node; // Move to new node
        }
        // Move to next node
        else {
            curr_node = arc->node;
        }
        is_first = 0;
    }
}

// Read one line of prompt and infer the rest of the string
// Return 1 if reaches the end of the input prompt
// Return 0 if not reaches the end of the input prompt
int infer(Graph* graph, char c) {
    int remaining_len = 37; // Max length of the output string
    Node* curr_node = graph->root;
    int is_first = 1; // Flag for the first char
    uint pos = 0; // The pos where pattern has been matched
    while (is_first || (c = getchar()) != '\n') {
        is_first = 0;
        putchar(c);
        // truncate to 37 chars
        if (--remaining_len == 0) {
            printf("\n");
            return 0;
        }

        // Find arc
        Arc* arc = node_find_arc(curr_node, c, pos);

        // If the prompt is not supported
        if (arc == NULL) {
            print_reamining(ELLIPSES, remaining_len);
            printf("\n");
            return 0;
        }

        // The current node has not completed match
        if (pos < strlen(arc->pattern) - 1) {
            pos++;
        } else {
            curr_node = arc->node;
            pos = 0;
        }
    }

    remaining_len -= print_reamining(ELLIPSES, remaining_len);

    // Generate the rest of the string
    while (!is_leaf(curr_node) && remaining_len > 0) {
        Arc* arc = node_get_arc(curr_node);
        curr_node = arc->node;
        // Print the pattern starting from pos
        remaining_len -= print_reamining(arc->pattern + pos, remaining_len);
        pos = 0; // Reset pos for subsequent patterns
    }
    printf("\n");
    return 1;
}

uint graph_stats_freq(Node* node)
{
    if (is_leaf(node)) {
        return 0;
    }
    uint freq = node->freq;

    // DFS
    Arc* curr_arc = node->outputs->head;
    while (curr_arc != NULL) {
        // current node freq += the total freq of the sub-graph under the arc
        freq += graph_stats_freq(curr_arc->node);
        curr_arc = curr_arc->next;
    }
    return freq;
}

Arc* arc_init(char* pattern, Node* node)
{
    Arc* arc = malloc(sizeof(Arc));
    arc->pattern = pattern;
    arc->node = node;
    arc->next = NULL;
    return arc;
}

void arc_free(Arc* arc)
{
    free(arc->pattern);
    free(arc);
}

Output* output_init()
{
    Output* output = malloc(sizeof(Output));
    output->head = NULL;
    output->tail = NULL;
    return output;
}

void output_free(Output* output)
{
    Arc* curr_arc = output->head;
    while (curr_arc != NULL) {
        Arc* next_arc = curr_arc->next;
        arc_free(curr_arc);
        curr_arc = next_arc;
    }
    free(output);
}

Node* node_init(uint id)
{
    Node* node = malloc(sizeof(Node));
    node->id = id;
    node->freq = 0;
    node->outputs = output_init();
    return node;
}

void node_free(Node* node)
{
    output_free(node->outputs);
    free(node);
}
Graph* graph_init()
{
    Graph* graph = malloc(sizeof(Graph));
    graph->root = node_init(0);
    graph->nodes_count = 1;
    return graph;
}

// Recursively free subgraphs
void graph_free_helper(Node* node)
{
    // Free all the subgraphs under the node
    Arc* curr_arc = node->outputs->head;
    while (curr_arc != NULL) {
        graph_free_helper(curr_arc->node);
        curr_arc = curr_arc->next;
    }
    node_free(node);
}

void graph_free(Graph* graph)
{
    graph_free_helper(graph->root);
    free(graph);
}

void output_add(Output* output, Arc* arc)
{
    if (output->head == NULL) {
        output->head = arc;
        output->tail = arc;
    } else {
        output->tail->next = arc;
        output->tail = arc;
    }
}

uint output_len(Output* output)
{
    uint len = 0;
    Arc* curr_arc = output->head;
    while (curr_arc != NULL) {
        len++;
        curr_arc = curr_arc->next;
    }
    return len;
}

// Find the arc according to the pos of the pattern
Arc* node_find_arc(Node* node, char c, uint pos)
{
    Arc* curr_arc = node->outputs->head;

    while (curr_arc != NULL) {
        char* pattern = curr_arc->pattern;
        // If the pattern matches
        if (pos < strlen(pattern) && pattern[pos] == c) {
            return curr_arc;
        }
        curr_arc = curr_arc->next;
    }
    return NULL;
}

// Get the next arc with the highest freq
Arc* node_get_arc(Node* node)
{
    Output* output = node->outputs;
    Arc* curr_arc = output->head;

    Arc* max_arc = curr_arc;
    uint max_freq = curr_arc->node->freq;
    char* max_pattern = curr_arc->pattern;
    while (curr_arc != NULL) {
        if (curr_arc->node->freq > max_freq) {
            max_arc = curr_arc;
            max_freq = curr_arc->node->freq;
            max_pattern = curr_arc->pattern;
        }
        // If the freq is the same, compare the ASCII value
        else if (curr_arc->node->freq == max_freq) {
            // Choose the larger one
            if (strcmp(curr_arc->pattern, max_pattern) > 0) {
                max_arc = curr_arc;
                max_freq = curr_arc->node->freq;
                max_pattern = curr_arc->pattern;
            }
        }

        curr_arc = curr_arc->next;
    }
    return max_arc;
}

// Compress the sub-graph under the node x
// times: the number of times to compress
// Return the remaining times to compress
uint compression_hlper(Graph* graph, Node* x, uint times)
{
    // No need to compress
    if (times == 0) {
        return 0;
    }

    if (is_leaf(x)) {
        return times;
    }
    Node* y = x->outputs->head->node;

    if (compressible(x)) {
        char* x_str = x->outputs->head->pattern;
        Arc* y_curr_arc = y->outputs->head;
        while (y_curr_arc != NULL) {
            char* comb_str = concat_str(x_str, y_curr_arc->pattern);
            Arc* new_arc = arc_init(comb_str, y_curr_arc->node);
            output_add(x->outputs, new_arc);
            Arc* next_arc = y_curr_arc->next;
            y_curr_arc = next_arc;
        }
        // Remove the first arc
        Arc* first_arc = x->outputs->head;
        x->outputs->head = first_arc->next;
        arc_free(first_arc);

        // Remove the node y
        node_free(y);
        graph->nodes_count--;

        times--;
        // Recheck if the node x is compressible
        times = compression_hlper(graph, x, times);
    }

    // DFS
    uint len_output = output_len(x->outputs);
    Arc** arcs = output_to_array(x->outputs, len_output);
    sort_arc(arcs, len_output);
    for (int i = 0; i < len_output; i++) {
        times = compression_hlper(graph, arcs[i]->node, times);
    }
    free(arcs);
    return times;
}

/* Utils Function*/
char* char_to_string(char c)
{
    char* str = malloc(sizeof(char) * 2);
    str[0] = c;
    str[1] = '\0';
    return str;
}

int is_leaf(Node* node) {
    // If a node has no outputs, it's a leaf
    return node->outputs->head == NULL;
}

// Print within the remaining length
int print_reamining(char* str, int reamining_len)
{
    if (reamining_len == 0) {
        return 0;
    }
    // len = min(strlen(str), reamining_len)
    int len = strlen(str) < reamining_len ? strlen(str) : reamining_len;
    for (int i = 0; i < len; i++) {
        putchar(str[i]);
    }
    return len;
}

// x only has one output
// y has one or more outputs
int compressible(Node* x)
{
    if (is_leaf(x)) {
        return 0;
    }

    // x has more than one output
    if (x->outputs->head != x->outputs->tail) {
        return 0;
    }

    Node* y = x->outputs->head->node;
    // y has no output
    if (is_leaf(y)) {
        return 0;
    }

    return 1;
}

char* concat_str(char* str1, char* str2)
{
    char* str = malloc(sizeof(char) * (strlen(str1) + strlen(str2) + 1));
    strcpy(str, str1);
    strcat(str, str2);
    return str;
}

// Sort the array of Arcs by pattern in accoding to ASCII value
// Use insert sort
int compareArcPatterns(const void* a, const void* b) {
    return strcmp((*(Arc**)a)->pattern, (*(Arc**)b)->pattern);
}

void sort_arc(Arc** value, uint size) {
    qsort(value, size, sizeof(Arc*), compareArcPatterns);
}

Arc** output_to_array(Output* output, uint len)
{
    if (len == 0) {
        return NULL;
    }
    Arc** arcs_list = malloc(sizeof(Arc*) * len);
    Arc* curr_arc = output->head;
    for (int i = 0; i < len; i++) {
        arcs_list[i] = curr_arc;
        curr_arc = curr_arc->next;
    }
    return arcs_list;
}