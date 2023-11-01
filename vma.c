#include "vma.h"
#include <stdlib.h>
#include <errno.h>

int convert_command(char *command)
{
	int x = strlen(command);
	if (command[x - 1] == ' ')
		command[x - 1] = '\0';
	if (strcmp(command, "ALLOC_ARENA") == 0)
		return 1;
	if (strcmp(command, "DEALLOC_ARENA") == 0)
		return 2;
	if (strcmp(command, "ALLOC_BLOCK") == 0)
		return 3;
	if (strcmp(command, "FREE_BLOCK") == 0)
		return 4;
	if (strcmp(command, "READ") == 0)
		return 5;
	if (strcmp(command, "WRITE") == 0)
		return 6;
	if (strcmp(command, "PMAP") == 0)
		return 7;
	if (strcmp(command, "MPROTECT") == 0)
		return 8;
	return 9;
}

/*
 * Functie care trebuie apelata pentru alocarea si initializarea unei liste.
 * (Setare valori initiale pentru campurile specifice structurii LinkedList).
 */
doubly_linked_list_t*
dll_create(unsigned int data_size)
{
	doubly_linked_list_t *my_list = malloc(sizeof(doubly_linked_list_t));
	DIE(!my_list, "malloc failed\n");

	my_list->head = NULL;
	my_list->tail = NULL;
	my_list->data_size = data_size;
	my_list->size = 0;
	return my_list;
}

/*
 * Functia intoarce un pointer la nodul de pe pozitia n din lista.
 * Pozitiile din lista sunt indexate incepand cu 0 (i.e. primul nod din lista se
 * afla pe pozitia n=0). Daca n >= nr_noduri, atunci se intoarce nodul de pe
 * pozitia rezultata daca am "cicla" (posibil de mai multe ori) pe lista si am
 * trece de la ultimul nod, inapoi la primul si am continua de acolo. Cum putem
 * afla pozitia dorita fara sa simulam intreaga parcurgere?
 * Atentie: n>=0 (nu trebuie tratat cazul in care n este negativ).
 */
dll_node_t*
dll_get_nth_node(doubly_linked_list_t *list, unsigned int n)
{
	if (!list)
		return NULL;

	if (n >= list->size)
		n = n % list->size;

	dll_node_t *aux = list->head;
	while (n > 0) {
		aux = aux->next;
		n--;
	}

	return aux;
}

/*
 * Pe baza datelor trimise prin pointerul new_data, se creeaza un nou nod care e
 * adaugat pe pozitia n a listei reprezentata de pointerul list. Pozitiile din
 * lista sunt indexate incepand cu 0 (i.e. primul nod din lista se afla pe
 * pozitia n=0). Cand indexam pozitiile nu "ciclam" pe lista circulara ca la
 * get, ci consideram nodurile in ordinea de la head la ultimul (adica acel nod
 * care pointeaza la head ca nod urmator in lista). Daca n >= nr_noduri, atunci
 * adaugam nodul nou la finalul listei.
 * Atentie: n>=0 (nu trebuie tratat cazul in care n este negativ).
 */
void
dll_add_nth_node(doubly_linked_list_t *list, unsigned int n,
				 const void *new_data)
{
	if (!list)
		return;

	if (n >= list->size)
		n = list->size;

	dll_node_t *new_node = malloc(sizeof(dll_node_t));
	DIE(!new_node, "malloc failed\n");

	new_node->data = malloc(list->data_size);
	DIE(!new_node->data, "malloc failed\n");

	memcpy(new_node->data, new_data, list->data_size);

	if (!list->head) {
		list->head = new_node;
		list->tail = new_node;
		new_node->next = NULL;
		new_node->prev = NULL;
	} else {
		if (n == 0) {
			new_node->next = list->head;
			new_node->prev = NULL;
			list->head->prev = new_node;
			list->head = new_node;
		} else {
			dll_node_t *aux = list->head;

			while (n > 1) {
				aux = aux->next;
				n--;
			}

			new_node->prev = aux;
			new_node->next = aux->next;
			aux->next = new_node;

			if (new_node->next)
				new_node->next->prev = new_node;

			if (!new_node->next)
				list->tail = new_node;
		}
	}

	list->size++;
}

/*
 * Elimina nodul de pe pozitia n din lista al carei pointer este trimis ca
 * parametru. Pozitiile din lista se indexeaza de la 0 (i.e. primul nod din
 * lista se afla pe pozitia n=0). Functia intoarce un pointer spre acest nod
 * proaspat eliminat din lista. Daca n >= nr_noduri - 1, se elimina nodul de la
 * finalul listei. Este responsabilitatea apelantului sa elibereze memoria
 * acestui nod.
 * Atentie: n>=0 (nu trebuie tratat cazul in care n este negativ).
 */
dll_node_t*
dll_remove_nth_node(doubly_linked_list_t *list, uint64_t n)
{
	if (!list || !list->head)
		return NULL;

	if (n >= list->size)
		n = list->size - 1;
	uint64_t copy = n;

	dll_node_t *aux = list->head;
	while (n > 0) {
		aux = aux->next;
		n--;
	}

	if (copy == 0 && list->size == 1) {
		list->head = NULL;
		list->tail = NULL;
		list->size--;
		return aux;
	}
	if (copy == 0) {
		list->head = aux->next;
		list->head->prev = NULL;
		list->size--;
		return aux;
	}
	if (aux == list->tail) {
		list->tail = aux->prev;
		list->tail->next = NULL;
		list->size--;
		return aux;
	}

	aux->next->prev = aux->prev;
	aux->prev->next = aux->next;

	list->size--;
	return aux;
}

/*
 * Procedura elibereaza memoria folosita de toate nodurile din lista, iar la
 * sfarsit, elibereaza memoria folosita de structura lista.
 */
void
dll_free(doubly_linked_list_t **pp_list)
{
	if (!(*pp_list))
		return;

	if (!(*pp_list)->head) {
		free(*pp_list);
		(*pp_list) = NULL;
		return;
	}

	dll_node_t *aux, *curr;
	aux = NULL;
	curr = (*pp_list)->head;
	while ((*pp_list)->size != 0) {
		aux = curr->next;
		free(curr->data);
		free(curr);
		curr = aux;
		(*pp_list)->head = aux;
		(*pp_list)->size--;
	}

	free(*pp_list);
	(*pp_list) = NULL;
}

arena_t *alloc_arena(const uint64_t size)
{
	/*aloc arena*/
	arena_t *arena = malloc(sizeof(*arena));
	if (!arena) {
		fprintf(stderr, "Malloc failed\n");
		exit(1);
	}

	arena->arena_size = size;
	arena->alloc_list = dll_create(sizeof(block_t));

	return arena;
}

void dealloc_arena(arena_t *arena)
{
	dll_node_t *current_block = arena->alloc_list->head;
	uint64_t arena_size = arena->alloc_list->size;

	/*Ma plimb in lista de blocuri, pentru a accesa listele de miniblocuri
	si a le da free*/
	for (uint64_t i = 0; i < arena_size; i++) {
		block_t *current_block_data = current_block->data;
		doubly_linked_list_t *mn_list = current_block_data->miniblock_list;

		dll_node_t *curr_mn = mn_list->head;
		uint64_t mn_size = mn_list->size;
		for (uint64_t j = 0; j < mn_size; j++) {
			miniblock_t *curr_mn_data = curr_mn->data;

			/*In cazul in care bufferele au fost alocate dinamic,
			le dau free*/
			if (curr_mn_data->rw_buffer)
				free(curr_mn_data->rw_buffer);

			curr_mn = curr_mn->next;
		}
		/*eliberez memoria pentru fiecare lista de miniblocuri*/
		dll_free(&(mn_list));
		current_block = current_block->next;
	}
	/*eliberez emoria pentru lista de blocuri si pentru arena*/
	dll_free(&arena->alloc_list);
	free(arena);
}

int is_val_block(arena_t *arena, const uint64_t address, const uint64_t size)
{
	/*verific daca adresa de inceput si size-ul blockului sunt valide
	si daca as putea sa adaug un nou block sau miniblock in lista*/
	if (address >= arena->arena_size) {
		printf("The allocated address is outside the size of arena\n");
		return 1;
	}

	if (address + size > arena->arena_size) {
		printf("The end address is past the size of the arena\n");
		return 1;
	}

	dll_node_t *curr = arena->alloc_list->head;
	while (curr) {
		block_t *curr_block = (block_t *)(curr->data);
		uint64_t end_address = curr_block->start_address + curr_block->size;
		uint64_t start_address = curr_block->start_address;

		if (start_address == address) {
			printf("This zone was already allocated.\n");
			return 1;
		}

		if ((start_address < address && end_address > address) ||
			(start_address < address + size && address + size < end_address) ||
			(start_address > address && end_address < address + size)) {
			printf("This zone was already allocated.\n");
			return 1;
		}

		curr = curr->next;
	}
	return 0;
}

/*functie auxiliara cu care creez un block si il adaug in arena*/
void create_block(arena_t *arena, const uint64_t address, const uint64_t size)
{
	block_t *new_data_block = malloc(sizeof(*new_data_block));
	DIE(!new_data_block, "malloc failed\n");

	/*initilizez data blockului*/
	new_data_block->size = size;
	new_data_block->start_address = address;
	new_data_block->miniblock_list = dll_create(sizeof(miniblock_t));

	doubly_linked_list_t *ml = new_data_block->miniblock_list;

	miniblock_t *new_data = malloc(sizeof(*new_data));
	DIE(!new_data, "malloc failed\n");

	/*initializez data unicului miniblock din block*/
	new_data->size = size;
	new_data->perm = 6;
	new_data->start_address = address;
	new_data->rw_buffer = NULL;

	/*daca nu am niciun block in arena, il adaug pe prima poz*/
	if (arena->alloc_list->size == 0) {
		dll_add_nth_node(arena->alloc_list, 0, new_data_block);
		dll_add_nth_node(ml, 0, new_data);
		free(new_data_block);
		free(new_data);
		return;
	}

	/*in caz contrat, adaug blockul in arena astfel incat adresele
	de inceput ale blocurilor din lista sa fie monoton crescatoare*/
	dll_node_t *node = arena->alloc_list->head;
	block_t *block = node->data;
	uint64_t i;
	for (i = 0; i < arena->alloc_list->size; i++) {
		block = node->data;

		if (new_data->start_address < block->start_address)
			break;
		node = node->next;
	}

	dll_add_nth_node(arena->alloc_list, i, new_data_block);
	dll_add_nth_node(ml, 0, new_data);

	free(new_data_block);
	free(new_data);
}

/*functie de concatenare a blocurilor*/
void concat_blocks(arena_t *arena, dll_node_t *node_i,
				   dll_node_t *node_j, uint64_t j, int *ok)
{
	block_t *node_i_data = node_i->data;
	block_t *node_j_data = node_j->data;

	/*verific daca este posibila concatenarea blocurilor*/
	if (node_j_data->start_address ==
		node_i_data->start_address + node_i_data->size ||
		node_j_data->start_address + node_j_data->size ==
		node_i_data->start_address) {
		*ok = 1;

		doubly_linked_list_t *mn_list_i = node_i_data->miniblock_list;
		doubly_linked_list_t *mn_list_j = node_j_data->miniblock_list;

		dll_node_t *curr_mn_j = mn_list_j->head;

		for (uint64_t k = 0; k < mn_list_j->size; k++) {
			/*transfer miniblockurile dintr-un block in altul
			dupa care eliberez mememoria*/
			if (node_j_data->start_address ==
				node_i_data->start_address + node_i_data->size)
				dll_add_nth_node(mn_list_i, mn_list_i->size, curr_mn_j->data);

			if (node_j_data->start_address + node_j_data->size ==
				node_i_data->start_address)
				dll_add_nth_node(mn_list_i, k, curr_mn_j->data);

			miniblock_t *mn_j_data = curr_mn_j->data;
			if (mn_j_data->rw_buffer)
				free(mn_j_data->rw_buffer);

			curr_mn_j = curr_mn_j->next;
		}
		dll_free(&mn_list_j);

		/*elimin blocul concatenat din arena*/
		dll_node_t *aux = dll_remove_nth_node(arena->alloc_list, j);
		free(aux->data);
		free(aux);

		miniblock_t *head_data = mn_list_i->head->data;
		miniblock_t *tail_data = mn_list_i->tail->data;

		/*modific noile dimensiuni ale blockului*/
		node_i_data->start_address = head_data->start_address;
		node_i_data->size = tail_data->start_address + tail_data->size -
		head_data->start_address;
	}
}

void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size)
{
	//verfic daca blockul citit este valid
	if (is_val_block(arena, address, size) == 1)
		return;

	dll_node_t *curr = arena->alloc_list->head;
	int self = 0;

	for (uint64_t i = 0; i < arena->alloc_list->size; i++) {
		block_t *curr_block = (block_t *)curr->data;

		//lipesc miniblock-ul la inceputul sau la sfarsitul miniblock-ului
		if (address == curr_block->start_address + curr_block->size ||
			address + size == curr_block->start_address) {
			doubly_linked_list_t *ml = curr_block->miniblock_list;

			miniblock_t *new_data = malloc(sizeof(*new_data));
			DIE(!new_data, "malloc failed\n");

			new_data->size = size;
			new_data->perm = 6;
			new_data->start_address = address;
			new_data->rw_buffer = NULL;

			/*verific pe ce pozitie ar trebui adaugat miniblockul in lista de
			miniblockuri*/
			if (address == curr_block->start_address + curr_block->size)
				dll_add_nth_node(ml, ml->size, new_data);
			else
				dll_add_nth_node(ml, 0, new_data);

			curr_block->size = curr_block->size + size;
			if (address + size == curr_block->start_address)
				curr_block->start_address = address;

			self = 1;

			free(new_data);
			break;
		}
		curr = curr->next;
	}

	//daca nu am lipit miniblock-ul si trebuie sa fac un block nou
	if (self == 0)
		create_block(arena, address, size);

	//daca am lipit miniblock-ul de un block si pot uni doua blockuri
	if (self == 1) {
		dll_node_t *curr = arena->alloc_list->head;
		uint64_t block_size = arena->alloc_list->size;
		int ok = 0;

		//ma plimb in lista de blocuri
		for (uint64_t i = 0; i < block_size - 1; i++) {
			dll_node_t *curr_j = curr->next;

			for (uint64_t j = i + 1; j < block_size; j++) {
				concat_blocks(arena, curr, curr_j, j, &ok);

				if (ok == 1)
					break;

				if (curr_j)
					curr_j = curr_j->next;
			}

			if (ok == 1)
				break;

			if (curr)
				curr = curr->next;
		}
	}
}

/*functie auxiliara pentru eliberarea memoriei ocupata de un miniblock*/
void free_miniblock(doubly_linked_list_t *mn, uint64_t j, uint64_t *size)
{
	dll_node_t *node = dll_remove_nth_node(mn, j);
	miniblock_t *mn_data = node->data;

	if (mn_data->rw_buffer)
		free(mn_data->rw_buffer);

	*size = mn_data->size;
	free(node->data);
	free(node);
}

void free_block(arena_t *arena, const uint64_t address)
{
	dll_node_t *current = arena->alloc_list->head;
	int ok = 0;

	for (uint64_t i = 0; i < arena->alloc_list->size; i++) {
		block_t *block = (block_t *)current->data;
		doubly_linked_list_t *mn = ((block_t *)current->data)->miniblock_list;
		dll_node_t *curr_mn = mn->head;

		for (uint64_t j = 0; j < mn->size; j++) {
			uint64_t start_adr = ((miniblock_t *)curr_mn->data)->start_address;
			uint64_t size;

			if (start_adr == address) {
				//elimin miniblockul de pe pozitia j
				ok = 1;
				curr_mn = curr_mn->next;
				free_miniblock(mn, j, &size);

				if (mn->size != 0 && (j == 0 || j == mn->size)) {
					/*modific noile dimensiuni ale blockului
					dupa stergerea unui miniblock*/
					((block_t *)current->data)->start_address =
					((miniblock_t *)mn->head->data)->start_address;
					((block_t *)current->data)->size -= size;
				}

				/*in cazul in care miniblocul nu se afla la inceputul sau la
				sf listei, sparg blocul in doua blocuri*/
				if (j != 0 && j != mn->size) {
					block_t *block_nou = malloc(sizeof(*block_nou));
					block_nou->miniblock_list = dll_create(sizeof(miniblock_t));

					for (uint64_t k = j; k < mn->size; k++) {
						dll_add_nth_node(block_nou->miniblock_list,
										 k - j, curr_mn->data);
						curr_mn = curr_mn->next;
					}
					for (uint64_t k = j; k <= mn->size; k++) {
						uint64_t n;
						free_miniblock(mn, k, &n);
					}

					//determin noile dimensiuni ale blocului
					block_nou->size = block->size -
					(address - block->start_address) - size;
					block_nou->start_address = address + size;

					block->size = address - block->start_address;

					dll_add_nth_node(arena->alloc_list, i + 1, block_nou);

					free(block_nou);
				}
				break;
			}
			curr_mn = curr_mn->next;
		}

		/*sterg blockul din lista de blocuri daca
		acesta nu contine niciun miniblock*/
		if (mn->size == 0) {
			dll_node_t *node = dll_remove_nth_node(arena->alloc_list, i);
			block_t *block_data = node->data;
			doubly_linked_list_t *mn = block_data->miniblock_list;

			dll_free(&mn);
			free(block_data);
			free(node);
			break;
		}

		current = current->next;
	}

	if (ok == 0)
		printf("Invalid address for free.\n");
}

void read_in_bl(doubly_linked_list_t *mn_list, uint64_t address,
				uint64_t size)
{
	dll_node_t *curr = mn_list->head;
	size_t copy = size;

	/*iterez prin lista de miniblockuri pana gasesc un mn de la care
	pot incepe citirea*/
	for (uint64_t i = 0; i < mn_list->size; i++) {
		miniblock_t *data_mn = curr->data;
		if (data_mn->start_address <= address &&
			data_mn->start_address + data_mn->size > address) {
			while (copy != 0 && curr) {
				miniblock_t *data = curr->data;
				int8_t *buffer = ((int8_t *)data->rw_buffer);

				uint64_t x = 0;
				/*afisez pe ecran in functie de 3 cazuri:
				daca adresa de la care incep scrierea se afla in
				in centrul miniblockului, daca pot afisa intreg sizeul
				miniblockului, si implicit tot bufferul corespendent, sau
				daca pot afisa doar cate prime caractere din buffer*/
				if (data->start_address < address) {
					x = address - data->start_address;
					for (uint64_t j = 0; j < size - x + 1; j++) {
						printf("%c", buffer[x + j]);
						copy--;
					}
					curr = curr->next;
					continue;
				}
				if (copy > data->size) {
					for (size_t j = 0; j < data->size; j++) {
						printf("%c", buffer[j]);
						copy--;
					}
					curr = curr->next;
					continue;
				}
				for (uint64_t j = 0; j < copy; j++)
					printf("%c", buffer[j]);
				copy = 0;

				curr = curr->next;
			}
			break;
		}

		curr = curr->next;
	}
}

int can_i_read(doubly_linked_list_t *mn_list, const uint64_t address,
			   long *copy, uint64_t *written)
{
	dll_node_t *curr = mn_list->head;
	/*iterez prin lista de miniblock-uri pana cand ajung la miniblockul
	din al carui buffer ar trebui sa citesc*/
	for (uint64_t i = 0; i < mn_list->size; i++) {
		miniblock_t *data_mn = curr->data;

		if (data_mn->start_address <= address &&
			data_mn->start_address + data_mn->size > address) {
			for (uint64_t k = i; k < mn_list->size && *copy > 0; k++) {
				data_mn = curr->data;
				/*calculez daca as putea citi cate caractere vreau, sau
				care ar fi numarul de caractere pe care le as putea citi*/
				if (k == i) {
					*copy -= data_mn->start_address + data_mn->size - address;
					*written += data_mn->start_address +
					data_mn->size - address;
				} else {
					*copy -= data_mn->size;
					*written += data_mn->size;
				}
				/*verific permisiunile*/
				if (data_mn->perm != 7 && data_mn->perm != 6 &&
					data_mn->perm != 5 && data_mn->perm != 4)
					return 0;
				curr = curr->next;
			}
			return 1;
		}

		curr = curr->next;
	}

	return 2;
}

/*functie cu care se realizeaza operatia de read*/
void read(arena_t *arena, uint64_t address, uint64_t size)
{
	int valid = 0;

	dll_node_t *curr_b = arena->alloc_list->head;
	for (uint64_t i = 0; i < arena->alloc_list->size; i++) {
		block_t *data_b = curr_b->data;
		doubly_linked_list_t *mn_list = data_b->miniblock_list;

		long copy = size;
		uint64_t written = 0;
		/*verific daca am permisiuni de citire sau daca adresa de la care se
		doreste citirea este una valida*/
		valid = can_i_read(mn_list, address, &copy, &written);

		if (valid == 1) {
			if (copy > 0) {
				printf("Warning: size was bigger than the block size. ");
				printf("Reading %lu characters.\n", written);
			}
			/*functie cu care fac citirea efectiva*/
			read_in_bl(mn_list, address, size);
			printf("\n");
			break;
		}
		curr_b = curr_b->next;
	}

	if (valid == 0 && arena->alloc_list->size != 0)
		printf("Invalid permissions for read.\n");

	if (valid == 2 || arena->alloc_list->size == 0)
		printf("Invalid address for read.\n");
}

void write_in_bl(doubly_linked_list_t *mn_list, uint64_t address,
				 uint64_t size, int8_t *text)
{
	dll_node_t *curr = mn_list->head;
	uint64_t copy = size;

	/*iterez prin lista de miniblock-uri pana gasesc un mn de la care pot
	incepe scrierea*/
	for (uint64_t i = 0; i < mn_list->size; i++) {
		miniblock_t *data_mn = curr->data;
		if (data_mn->start_address <= address &&
			data_mn->start_address + data_mn->size > address) {
			uint64_t wr = 0;
			for (uint64_t j = i; j < mn_list->size && copy > 0; j++) {
				miniblock_t *data = curr->data;

				/*aloc buffer-ul*/
				data->rw_buffer = (int8_t *)malloc((data->size + 1) *
				sizeof(int8_t));
				DIE(!data->rw_buffer, "malloc failled\n");

				int8_t *buffer = NULL;
				buffer = (int8_t *)data->rw_buffer;
				uint64_t x;

				/*copiez caractere din string-ul citit in bufferele din
				miniblockuri astfel: copiez in interiorul miniblockului,
				copiez in tot size-ul miniblockului, sau doar la inceputul
				acestuia pana cand termin de copiat toate caracterele din
				string*/
				if (data->start_address < address) {
					x = address - data->start_address;
					strncpy((char *)buffer + x, (const char *)text,
							data->size - x);
					copy -= data->size;
					wr += data->size - x;
				} else if (copy > data->size) {
					strncpy((char *)buffer, (const char *)text + wr,
							data->size);
					wr += data->size;
					copy -= data->size;
				} else {
					strncpy((char *)buffer, (const char *)text + wr, copy);
					wr += data->size;
					buffer[copy] = '\0';
					copy -= copy;
				}
				curr = curr->next;
			}
			break;
		}

		curr = curr->next;
	}
}

int can_i_write(doubly_linked_list_t *mn_list, const uint64_t address,
				long *copy, uint64_t *written)
{
	dll_node_t *curr = mn_list->head;
	/*iterez prin lista de miniblock-uri pana cand ajung la miniblockul
	in al carui buffer ar trebui sa scriu*/
	for (uint64_t i = 0; i < mn_list->size; i++) {
		miniblock_t *data_mn = curr->data;
		if (data_mn->start_address <= address &&
			data_mn->start_address + data_mn->size > address) {
			for (uint64_t k = i; k < mn_list->size; k++) {
				/*calculez daca as putea scrie cate caractere vreau, sau
				care ar fi numarul de caractere pe care le as putea scrie*/
				if (*copy <= 0)
					break;
				data_mn = curr->data;
				if (k == i) {
					*copy -= data_mn->start_address + data_mn->size - address;
					*written = *written + data_mn->start_address +
					data_mn->size - address;
				} else {
					*copy -= data_mn->size;
					*written += data_mn->size;
				}

				/*verific permisiunile*/
				if (data_mn->perm != 7 && data_mn->perm != 6 &&
					data_mn->perm != 3 && data_mn->perm != 2)
					return 0;
				curr = curr->next;
			}

			return 1;
		}

		curr = curr->next;
	}

	return 2;
}

/*functie cu care se realizeaza operatia de read*/
void write(arena_t *arena, const uint64_t address,
		   const uint64_t size, int8_t *text)
{
	int valid = 0;
	text = malloc((size + 1) * sizeof(int8_t));
	DIE(!text, "malloc faied.\n");

	for (uint64_t i = 0; i < size; i++) {
		char c = getchar();
		if (i == 0 && c == ' ')
			i--;
		else
			text[i] = c;
	}
	text[size] = '\0';

	dll_node_t *curr_b = arena->alloc_list->head;
	for (uint64_t i = 0; i < arena->alloc_list->size; i++) {
		block_t *data_b = curr_b->data;
		doubly_linked_list_t *mn_list = data_b->miniblock_list;

		long copy = size;
		uint64_t written = 0;

		/*verific daca am permisiuni de sciere sau daca adresa de la care se
		doreste scrierea este una valida*/
		valid = can_i_write(mn_list, address, &copy, &written);

		if (valid == 1) {
			if (copy > 0) {
				printf("Warning: size was bigger than the block size. ");
				printf("Writing %lu characters.\n", written);
			}
			/*functie in care realizez scrierea efectiva*/
			write_in_bl(mn_list, address, size, text);
			break;
		}
		curr_b = curr_b->next;
	}

	if (valid == 0 && arena->alloc_list->size != 0)
		printf("Invalid permissions for write.\n");

	if (valid == 2 || arena->alloc_list->size == 0)
		printf("Invalid address for write.\n");

	free(text);
}

/*functie de convertire a permisiunilor*/
char *convert_perm(uint64_t number)
{
	if (number == 7)
		return "RWX";
	if (number == 6)
		return "RW-";
	if (number == 5)
		return "R-X";
	if (number == 4)
		return "R--";
	if (number == 3)
		return "-WX";
	if (number == 2)
		return "-W-";
	if (number == 1)
		return "--X";
	if (number == 0)
		return "---";
	return "nu stiu";
}

void pmap(const arena_t *arena)
{
	printf("Total memory: 0x%lX bytes\n", arena->arena_size);

	uint64_t used_space = 0;
	dll_node_t *curr = arena->alloc_list->head;

	for (uint64_t i = 0; i < arena->alloc_list->size; i++) {
		block_t *data = curr->data;
		used_space = used_space + data->size;
		curr = curr->next;
	}
	uint64_t free_space = arena->arena_size - used_space;

	printf("Free memory: 0x%lX bytes\n", free_space);

	printf("Number of allocated blocks: %u\n", arena->alloc_list->size);

	uint64_t mn_nb = 0;
	dll_node_t *aux = arena->alloc_list->head;
	for (uint64_t i = 0; i < arena->alloc_list->size; i++) {
		block_t *data = aux->data;
		doubly_linked_list_t *mn = data->miniblock_list;
		mn_nb = mn->size + mn_nb;

		aux = aux->next;
	}

	printf("Number of allocated miniblocks: %lu\n", mn_nb);

	dll_node_t *node = arena->alloc_list->head;

	for (uint64_t i = 0; i < arena->alloc_list->size; i++) {
		printf("\n");
		block_t *curr_block_data = node->data;

		uint64_t start_add = curr_block_data->start_address;
		uint64_t size = curr_block_data->size;
		printf("Block %lu begin\nZone: 0x%lX - 0x%lX\n",
			   i + 1, start_add, start_add + size);

		doubly_linked_list_t *mn = curr_block_data->miniblock_list;

		dll_node_t *node_mn = mn->head;

		for (uint64_t j = 0; j < mn->size; j++) {
			miniblock_t *curr_mn_data = node_mn->data;

			uint64_t start_add_mn = curr_mn_data->start_address;
			uint64_t size_mn = curr_mn_data->size;
			uint64_t perm_mn = curr_mn_data->perm;

			char *string_perm = convert_perm(perm_mn);

			printf("Miniblock %lu:		0x%lX		-		0x%lX		| %s\n"
				   , j + 1, start_add_mn, start_add_mn + size_mn, string_perm);
			node_mn = node_mn->next;
		}

		printf("Block %lu end\n", i + 1);
		node = node->next;
	}
}

int convert_permission(char *perm)
{
	int fperm = 0;
	char *p;
	p = strtok(perm, " |");
	/*sparg sirul de comenzi de permisiuni*/
	while (p) {
		int x = strlen(p);
		if (p[x - 1] == '\n')
			p[x - 1] = '\0';
		if (strcmp(p, "PROT_NONE") == 0)
			fperm = 0;
		if (strcmp(p, "PROT_READ") == 0) {
			if (fperm <= 3)
				fperm = fperm + 4;
		}
		if (strcmp(p, "PROT_WRITE") == 0) {
			if (fperm < 2 || fperm == 4 || fperm == 5)
				fperm = fperm + 2;
		}
		if (strcmp(p, "PROT_EXEC") == 0) {
			if (fperm % 2 == 0)
				fperm = fperm + 1;
		}
		p = strtok(NULL, " |");
	}
	return fperm;
}

void mprotect(arena_t *arena, uint64_t address, int8_t *permission)
{
	int ok = 0;
	/*Convertesc permisiunile dintr-un string intr-un uint8_t*/
	uint8_t perm = convert_permission((char *)permission);
	dll_node_t *cur_bl = arena->alloc_list->head;

	/*parcurg lista de blocuri*/
	for (uint64_t i = 0; i < arena->alloc_list->size; i++) {
		block_t *bl_data = cur_bl->data;
		doubly_linked_list_t *mn_list = bl_data->miniblock_list;
		dll_node_t *curr_mn = mn_list->head;

		/*parcurg lista de miniblock-uri din blockul respectiv*/
		for (uint64_t j = 0; j < mn_list->size; j++) {
			miniblock_t *mn_data = curr_mn->data;

			/*in caz de egalitate, schimb permisiunile miniblockului*/
			if (mn_data->start_address == address) {
				mn_data->perm = perm;
				ok = 1;
				break;
			}
			curr_mn = curr_mn->next;
		}

		cur_bl = cur_bl->next;
	}

	if (ok == 0)
		printf("Invalid address for mprotect.\n");
}
