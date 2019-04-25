/**
 * Buddy Allocator
 *
 * For the list library usage, see http://www.mcs.anl.gov/~kazutomo/list/
 */

/**************************************************************************
 * Conditional Compilation Options
 **************************************************************************/
#define USE_DEBUG 0

/**************************************************************************
 * Included Files
 **************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "buddy.h"
#include "list.h"

/**************************************************************************
 * Public Definitions
 **************************************************************************/
#define MIN_ORDER 12
#define MAX_ORDER 20

#define PAGE_SIZE (1<<MIN_ORDER)
/* page index to address */
#define PAGE_TO_ADDR(page_idx) (void *)((page_idx*PAGE_SIZE) + g_memory)

/* address to page index */
#define ADDR_TO_PAGE(addr) ((unsigned long)((void *)addr - (void *)g_memory) / PAGE_SIZE)

/* find buddy address */
#define BUDDY_ADDR(addr, o) (void *)((((unsigned long)addr - (unsigned long)g_memory) ^ (1<<o)) \
									 + (unsigned long)g_memory)

#if USE_DEBUG == 1
#  define PDEBUG(fmt, ...) \
	fprintf(stderr, "%s(), %s:%d: " fmt,			\
		__func__, __FILE__, __LINE__, ##__VA_ARGS__)
#  define IFDEBUG(x) x
#else
#  define PDEBUG(fmt, ...)
#  define IFDEBUG(x)
#endif

/**************************************************************************
 * Public Types
 **************************************************************************/
typedef struct {
	struct list_head list;
	/* TODO: DECLARE NECESSARY MEMBER VARIABLES */
	int order;
	int index;
	char* mem;
} page_t;

/**************************************************************************
 * Global Variables
 **************************************************************************/
/* free lists*/
struct list_head free_area[MAX_ORDER+1];

/* memory area */
char g_memory[1<<MAX_ORDER];

/* page structures */
page_t g_pages[(1<<MAX_ORDER)/PAGE_SIZE];

/**************************************************************************
 * Public Function Prototypes
 **************************************************************************/
int nextHighestTwo(int x);

int orderFor(int x);

void split(page_t* page, int i, int order);
/**************************************************************************
 * Local Functions
 **************************************************************************/

/**
 * Initialize the buddy system
 */
void buddy_init()
{
	int i;
	int n_pages = (1<<MAX_ORDER) / PAGE_SIZE;
	for (i = 0; i < n_pages; i++) {
		/* TODO: INITIALIZE PAGE STRUCTURES */
		//-1 order = free block
		g_pages[i].order = -1;
		g_pages[i].mem = PAGE_TO_ADDR(i);
	}

	/* initialize freelist */
	for (i = MIN_ORDER; i <= MAX_ORDER; i++) {
		INIT_LIST_HEAD(&free_area[i]);
	}

	/* add the entire memory as a freeblock */
	list_add(&g_pages[0].list, &free_area[MAX_ORDER]);
}

/**
 * Allocate a memory block.
 *
 * On a memory request, the allocator returns the head of a free-list of the
 * matching size (i.e., smallest block that satisfies the request). If the
 * free-list of the matching block size is empty, then a larger block size will
 * be selected. The selected (large) block is then splitted into two smaller
 * blocks. Among the two blocks, left block will be used for allocation or be
 * further splitted while the right block will be added to the appropriate
 * free-list.
 *
 * @param size size in bytes
 * @return memory block address
 */

void *buddy_alloc(int size)
{
	/* TODO: IMPLEMENT THIS FUNCTION */
	int order = orderFor(size);
	if (order != -1){
		for (int i = order; i < MAX_ORDER; i++){
			if (!list_empty(&free_area[i])){
				//allocate to appropriate mem block
				page_t* page = list_entry(free_area[i].next,page_t,list);
				list_del_init(&(page->list));
				split(page,i,order);
				page->order = order;
				return ((void*) (page->mem));
			}
		}
	}
	return NULL;
}

/**
 * Free an allocated memory block.
 *
 * Whenever a block is freed, the allocator checks its buddy. If the buddy is
 * free as well, then the two buddies are combined to form a bigger block. This
 * process continues until one of the buddies is not free.
 *
 * @param addr memory block address to be freed
 */
void buddy_free(void *addr)
{
	/* TODO: IMPLEMENT THIS FUNCTION */
	page_t* curr_page = &g_pages[ADDR_TO_PAGE(addr)];
	page_t* buddy = &g_pages[ADDR_TO_PAGE(BUDDY_ADDR(curr_page->mem, curr_page->order))];


	int is_free = 0;
	for (int i = curr_page->order; i < MAX_ORDER; i++){
		struct list_head *pos;

		list_for_each(pos, &free_area[i]) {
			if (list_entry(pos, page_t, list) == buddy)
				is_free = 1;
		}

		if(!is_free)
			break;

		list_del_init(&buddy->list);
		if (buddy < curr_page)
			curr_page = buddy;

		curr_page->order = i;
		list_add(&curr_page->list, &free_area[i]);
	}
}

/**
 * Print the buddy system status---order oriented
 *
 * print free pages in each order.
 */
void buddy_dump()
{
	int o;
	for (o = MIN_ORDER; o <= MAX_ORDER; o++) {
		struct list_head *pos;
		int cnt = 0;
		list_for_each(pos, &free_area[o]) {
			cnt++;
		}
		printf("%d:%dK ", cnt, (1<<o)/1024);
	}
	printf("\n");
}

void split(page_t* page, int i, int order){
	if (i != order){
		int new_page_order = i-1;
		page_t* buddy = &g_pages[ADDR_TO_PAGE(BUDDY_ADDR(page->mem,new_page_order))];
		buddy->order = new_page_order;
		list_add(&(buddy->list),&free_area[new_page_order]);
		split(page,new_page_order,order);
	}
}

int orderFor(int x){
 for (int order = MIN_ORDER; order <= MAX_ORDER; order++){
	 if (1 << order >= x){
		 return order;
	 }
 }
 return -1;
}
