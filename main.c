#include "vma.h"

int main(void)
{
	char command[150];
	char perm_char[1000];
	arena_t *arena = NULL;
	uint64_t adress;
	uint64_t size;
	uint64_t arena_size;
	int gata = 0;
	int8_t *string = NULL;

	while (1) {
		scanf("%s", command);
		int nr_com = convert_command(command);

		switch (nr_com) {
		case 1:
			scanf("%lu", &arena_size);
			arena = alloc_arena(arena_size);
			break;
		case 2:
			dealloc_arena(arena);
			gata = 1;
			break;
		case 3:
			scanf("%lu %lu", &adress, &size);
			alloc_block(arena, adress, size);
			break;
		case 4:
			scanf("%lu", &adress);
			free_block(arena, adress);
			break;
		case 5:
			scanf("%lu %lu", &adress, &size);
			read(arena, adress, size);
			break;
		case 6:
			scanf("%lu %lu", &adress, &size);
			write(arena, adress, size, string);
			break;
		case 7:
			pmap(arena);
			break;
		case 8:
			scanf("%lu", &adress);
			fgets(perm_char, 1000, stdin);
			mprotect(arena, adress, (int8_t *)perm_char);
		break;
		case 9:
			printf("Invalid command. Please try again.\n");
		}

		if (gata == 1)
			break;
	}
	return 0;
}
