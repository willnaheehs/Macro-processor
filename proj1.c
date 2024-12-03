#define STRING_INITIAL_CAPACITY 10
#define RESIZE_FACTOR 2

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "proj1.h"

typedef struct{
    char *data;
    size_t sz;
    size_t max_cap;
} string; // string library

typedef struct node Node;
typedef struct list macro_list;

struct node{
    string *value;
    string *name;
    Node *next;
}; // node used for macro list

struct list{
    Node *head;
}; // llist for defined macros

typedef enum{
    PLAIN_TEXT,
    ESCAPE_SEQUENCE,
    MACRO
} parse_state; // States

parse_state state;

// forward declarations
string *new_string();
void free_s(string *s);
void read_from_stdin(string *in);
void read_from_file(const char *filename, string *in);
void state_machine(char c, string *in, string *output, macro_list *list);
macro_list *make_lst();
void free_lst(macro_list *list);
void expand_if_needed(string *s);
char next_c(string *in);

// driver
int main(int argc, char *argv[]){
    string *in = new_string();

    // no files are provided->read from stdin
    if (argc == 1){
        read_from_stdin(in);
    }

    // files provided->read into in master string
    for (int i = 1; i < argc; i++){
        read_from_file(argv[i], in);
    }

    state = PLAIN_TEXT;
    string *output = new_string(); //make output string
    macro_list *list = make_lst();

    // execute SM to process macros
    while (in->sz > 0){
        state_machine(next_c(in), in, output, list);
    }
    output->data[output->sz] = '\0';

    fprintf(stdout, "%s", output->data);

    // clean up
    free_lst(list);
    free_s(in);
    free_s(output);
}

// make new string
string *new_string(){
    string *s = malloc(sizeof(string));
    s->max_cap = STRING_INITIAL_CAPACITY;
    s->data = malloc(s->max_cap * sizeof(char));
    s->sz = 0;
    return s;
}

// free string
void free_s(string *s){
    free(s->data);
    free(s);
}

// doubles string size
void expand_if_needed(string *s){
    if(s->sz >= s->max_cap){
        s->data = realloc(s->data, RESIZE_FACTOR*s->max_cap);
        s->max_cap *= RESIZE_FACTOR;
    }
}

// add char to top of "stack"
void add_char_to_input(char c, string *in){
    in->data[in->sz] = c;
    in->sz++;
}

// add string to stack char by char
void add_string_to_input(string *s, string *in){
    for(int i = 0; i < s->sz; i++){
        add_char_to_input(s->data[s->sz-1-i], in);
        expand_if_needed(in);
    }
}

// pop next value in stack
char next_c(string *in){
    return in->data[(in->sz--) - 1];
}

// add char to output
void add_c(char c, string *s){
    s->data[s->sz] = c;
    s->sz++;
    expand_if_needed(s);
}

// add string to output
void add_s(string *s, string *output){
    for(int i = 0; i < s->sz; i++)
   {
        add_c(s->data[i], output);
    } 
}

// process the string, replace '#' with arg and handling escape sequences
string *process_string_with_arg(string *s, string *arg){
    int esc = 0; // 0 if plain text, 1 if escape
    string *result = new_string();
    for (int i = 0; i < s->sz; i++){
        char c = s->data[i];
        if (c == '\\' && esc == 0){
            add_c(c, result);
            esc = 1;
        }
        else if (c == '#' && esc == 0){
            add_s(arg, result);
        }
        else{
            add_c(c, result);
            esc = 0;
        }
        expand_if_needed(result);
    }
    return result;
}

// add processed string result to in
void add_processed_string_to_input(string *processed, string *in){
    add_string_to_input(processed, in);
    free_s(processed);  // free the processed string after using it
}

// process and add the string to in
void user_string_to_input(string *s, string *in, string *arg){
    string *processed_string = process_string_with_arg(s, arg);
    add_processed_string_to_input(processed_string, in);
}

// make new list
macro_list *make_lst(){
    macro_list *list = malloc(sizeof(macro_list));
    list->head = NULL;
    return list;
}

// make new node
Node *create_node(string *name, string *value){
    Node *macro = malloc(sizeof(Node));
    macro->name = name;
    macro->value = value;
    macro->next = NULL;
    return macro;
}

// free node
void delete_node(Node *macro){
    free_s(macro->name);
    free_s(macro->value);
    free(macro);
}

// insert node to list(front)
void insert_node(macro_list *list, string *name, string *value){
    Node *macro = create_node(name, value);
    macro->next = list->head;
    list->head = macro;
}

// free list
void free_lst(macro_list *list){
    Node *ptr = list->head;
    while(ptr){
        Node *temp = ptr;
        ptr = ptr->next;
        delete_node(temp);
    }
    free(list);
}

// if node not in list -> exit, remove if it is (by name)
void remove_node(string *name, macro_list *list){
    Node *prev = NULL;
    Node *ptr = list->head;
    if(!ptr){
        DIE("%s", "Macro not defined");
    }
    if(strcmp(name->data, ptr->name->data) == 0){
        list->head = ptr->next;
        delete_node(ptr);
    }
    else{
        while(strcmp(name->data, ptr->name->data) != 0){
            prev = ptr;
            ptr = ptr->next;
            if(!ptr){
                DIE("%s", "Macro undefined");
            }
        }
        prev->next = ptr->next;
        delete_node(ptr);
    }
}

//returns true if macro with name is in list
bool is_defined(string *name, macro_list *list){
    Node *ptr = list->head;
    if(!ptr){
        return false;
    }
    while(strcmp(name->data, ptr->name->data) != 0){
        ptr = ptr->next;
        if(!ptr){
            return false;
        }
    }
    return true;
}

//reads until brace and then returns macro name string
string *get_macro_name(string *in){
    string *name = new_string();
    char c;
    while((c = next_c(in)) != '{'){
        if(!isalnum(c)){
            DIE("%s", "Macro name not alphanumeric");
        }
        if(in->sz == 0){
            DIE("%s", "Missing args");
        }
        add_c(c, name);
    }
    name->data[name->sz] = '\0';
    in->sz++; // make next char the brace
    return name;
}

// read until brace balanced
// if doesnt start with brace->return error message
string *get_args(string *in){
    // If the in size is 0, there are no arguments
    if (in->sz == 0){
        DIE("%s", "Missing args");
    }

    char c;
    int b_balance = 0; 
    string *arg = new_string();  
    c = next_c(in);
    if (c != '{'){
        DIE("%s", "Missing args");
    }
    b_balance--;
    while (b_balance != 0){
        // if in is exhausted and braces are not balanced-> an error
        if (in->sz == 0){
            DIE("%s", "Braces unbalanced");
        }
        c = next_c(in);
        // decrement balance if opening
        if (c == '{'){
            b_balance--;
        }
        if (c == '}'){
            b_balance++;
        }
        // break when balanced
        if (b_balance == 0){
            break;
        }
        // add char to the argument string
        add_c(c, arg);

        // if the character is an escape characte->add the next char without interpreting it
        if (c == '\\'){
            add_c(next_c(in), arg);
        }
    }

    // null-terminate argument string
    arg->data[arg->sz] = '\0';
    return arg;
}

// called when macro state attained->expand macro on top of stack
void expand_macro(string *macro, string *in, macro_list *list){
    if(strcmp(macro->data, "if") == 0){
        string *COND = get_args(in);
        string *ELSE = get_args(in);
        string *THEN = get_args(in);
        if(strcmp(COND->data, "") == 0){
            add_string_to_input(ELSE, in);
        }
        else{
            add_string_to_input(THEN, in);
        }
        free_s(COND);
        free_s(ELSE);
        free_s(THEN);
    }
    else if(strcmp(macro->data, "def") == 0){
        string *NAME = get_args(in);
        string *VALUE = get_args(in);
        for(int i = 0; i < NAME->sz; i++){
            if(!isalnum(NAME->data[i])){
                DIE("%s", "Name is not alphanumeric");
            }
        }
        if(is_defined(NAME, list)){
            DIE("%s", "Macro already exists");
        }
        insert_node(list, NAME, VALUE);
    }
    else if(strcmp(macro->data, "undef") == 0){
        string *NAME = get_args(in);
        remove_node(NAME, list);
        free_s(NAME);
    }
    // Handle the \ifdef->conditionally expand based on macro existence
    else if(strcmp(macro->data, "ifdef") == 0){
        string *MACRO = get_args(in);
        string *THEN = get_args(in);
        string *ELSE = get_args(in);
        for(int i = 0; i < MACRO->sz; i++){
            if(!isalnum(MACRO->data[i])){
                DIE("%s", "Name is not alphanumeric");
            }
        }
        if(is_defined(MACRO, list)){
            add_string_to_input(THEN, in);
        }
        else{
            add_string_to_input(ELSE, in);
        }
        free_s(MACRO);
        free_s(THEN);
        free_s(ELSE);
    }
    // just doesnt work
    else if(strcmp(macro->data, "expandafter") == 0){
        string *BEFORE = get_args(in); 
        if(BEFORE == NULL)
            DIE("%s", "..."); 
        string *AFTER = get_args(in); 
        if(AFTER == NULL)
            DIE("%s", "...");
        string *expansion = new_string();
        while(AFTER->sz > 0) 
        add_string_to_input(expansion, in); 
        add_string_to_input(BEFORE, in);
        free_s(BEFORE); 
        }
    else{
        // user defined macros
        Node *ptr = list->head;
        if(!ptr){
            DIE("Macro '%s' DNE", macro->data);
        }
        // traverse list until pointer finds node w correct macro name
        while(strcmp(macro->data, ptr->name->data) != 0){
            ptr = ptr->next;
            if(!ptr){
                DIE("Macro '%s' DNE", macro->data);
            }
        }
        string *arg = get_args(in);
        user_string_to_input(ptr->value, in, arg);
        free_s(arg);
    }
    free_s(macro);
}

// Helper function to handle macro expansion
void handle_macro_expansion(string *in, string *output, macro_list *list){
    string *macro_name = get_macro_name(in);  // get  macro name
    expand_macro(macro_name, in, list);         // expand the macro
}                       

// state machine to process macros, plain text and esc sequences
void state_machine(char c, string *in, string *output, macro_list *list){
    // string *macro_name;
    switch(state){
        case PLAIN_TEXT:
            if(c == '\\'){
                state = ESCAPE_SEQUENCE;
            }
            else{
                add_c(c, output);
            }
            break;
        case ESCAPE_SEQUENCE:
            if(c == '\\' || c == '%' || c == '#' || c == '{' || c == '}'){
                add_c(c, output);
                state = PLAIN_TEXT;
                break;
            }
            else if(!isalnum(c)){
                add_c('\\', output);
                add_c(c, output);
                state = PLAIN_TEXT;
            }
            else{
                in->sz++;
                state = MACRO;
            }
        case MACRO:
            handle_macro_expansion(in, output, list);
            state = PLAIN_TEXT;
            break;
    }
}

// in processing implementation

// helper to read from stdin
void read_from_stdin(string *in){
    string *s = new_string();
    char c;
    while ((c = fgetc(stdin)) != EOF){
        add_c(c, s);
        expand_if_needed(s);
    }
    s->data[s->sz] = '\0';
    add_string_to_input(s, in);
    free_s(s);
}

// helper to read from a file
void read_from_file(const char *filename, string *in){
    FILE *file = fopen(filename, "r");
    if (!file){
        DIE("%s", "File does not exist");
    }

    string *s = new_string();
    char c;
    while ((c = getc(file)) != EOF){
        add_c(c, s);
        expand_if_needed(s);
    }
    s->data[s->sz] = '\0';
    fclose(file);
    
    add_string_to_input(s, in);
    free_s(s);
}
