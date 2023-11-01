#pragma once
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(errno);				        \
		}							\
	} while (0)

#define MAX_STRING_SIZE 64

/* TODO : add your implementation for doubly-linked list */

typedef struct dll_node_t dll_node_t;

struct dll_node_t {
	void *data; /* Pentru ca datele stocate sa poata avea orice tip, folosim un
				pointer la void. */
	dll_node_t *prev, *next;
};

typedef struct doubly_linked_list_t doubly_linked_list_t;

struct doubly_linked_list_t {
	dll_node_t *head;
	dll_node_t *tail;
	unsigned int data_size;
	unsigned int size;
};

typedef struct {
	uint64_t start_address; //adresa de inceput a zonei, un indice din arena
	size_t size; //dimensiunea totala a zonei, suma size-urilor miniblock-urilor
	void *miniblock_list; //lista de miniblock-uri adiacente
} block_t;

typedef struct {
	uint64_t start_address; //adresa de inceput a zonei, un indice din arena
	size_t size; //size-ul miniblock-ului
	uint8_t perm; //permisiunile asociate zonei, by default RW-
	void *rw_buffer; //buffer-ul de date, folosit pentru operatiile
						//de read() si write()
} miniblock_t;

typedef struct {
	uint64_t arena_size;
	doubly_linked_list_t *alloc_list;
} arena_t;

int convert_command(char *command);

doubly_linked_list_t *dll_create(unsigned int data_size);
dll_node_t *dll_get_nth_node(doubly_linked_list_t *list, unsigned int n);
void dll_add_nth_node(doubly_linked_list_t *list, unsigned int n,
					  const void *new_data);
dll_node_t *dll_remove_nth_node(doubly_linked_list_t *list, uint64_t n);
unsigned int dll_get_size(doubly_linked_list_t *list);
void dll_free(doubly_linked_list_t **pp_list);

arena_t *alloc_arena(const uint64_t size);
void dealloc_arena(arena_t *arena);

void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size);
int is_val_block(arena_t *arena, const uint64_t address, const uint64_t size);

void free_miniblock(doubly_linked_list_t *mn, uint64_t j, uint64_t *size);
void free_block(arena_t *arena, const uint64_t address);

void read(arena_t *arena, uint64_t address, uint64_t size);
void write(arena_t *arena, const uint64_t address,
		   const uint64_t size, int8_t *data);
void pmap(const arena_t *arena);
void mprotect(arena_t *arena, uint64_t address, int8_t *permission);
