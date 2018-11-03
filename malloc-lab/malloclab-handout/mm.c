/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Prabh Simran Singh Baweja",
    /* First member's email address */
    "prabh.baweja@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define WSIZE 4  /* word and header/footer size(bytes) */
#define DSIZE 8 /* Double word size (bytes) */

#define CHUNKSIZE (1<<20)

#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define PACK(size, alloc) ((size) | (alloc)) /* Pack size and allocated bit into a word */

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */ 
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

#define NEXT_PTR(p)  (*(char **)(p + WSIZE))
#define PREV_PTR(p)  (*(char **)(p))

static char *free_listp = NULL;

static char *heap_listp;

void *coalesce(void *bp) {
    // printf("Coalescing\n");

    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {   /* Case 1 */
        // printf("Case 1\n");
        free_list_add(bp);
        return bp;
    }
    else if (prev_alloc && !next_alloc) {   /* Case 2 */
        // printf("Case 2\n");
        free_list_delete(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        free_list_add(bp);

    }
    else if (!prev_alloc && next_alloc) {  /* Case 3 */
        // printf("Case 3\n");
        free_list_delete(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
        free_list_add(bp);
    }
    else {                /* Case 4 */
        // printf("Case 4\n");
        free_list_delete(PREV_BLKP(bp));
        free_list_delete(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
            GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
        free_list_add(bp);
    }
    // mm_check();
    return bp;
}

void *extend_heap(size_t words) {
    // printf("\n");
    char *bp;
    size_t size;
    // printf("Extending heap by %zu\n", words);
    /* Allocate an even number of words to maintain alignment */ 
    size = ALIGN(words + SIZE_T_SIZE);
    // printf("Heap Extension: After alignment size is %zu\n", size);
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0)); /* Free block header */
    PUT(FTRP(bp), PACK(size, 0)); /* Free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */

    // printf("Heap Extension: Size of bp after extension: %u\n", GET_SIZE(HDRP(bp))); 

    /* Coalesce if the previous block was free */
    return coalesce(bp);

}

void *find_fit(size_t asize) {
    // printf("\n");
    /* First fit */
    void *bp;
    for (bp = free_listp; bp != NULL; bp = NEXT_PTR(bp)) {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
            // printf("Found the best fit of bp with size remaining %u\n", GET_SIZE(HDRP(bp)));
            // printf("Found the best fit of bp with alloc %u\n", GET_ALLOC(HDRP(bp)));
            return bp;
        }
    }
    return NULL;
}

void place (void *bp, size_t asize) {
    // printf("\n");
    size_t csize = GET_SIZE(HDRP(bp));
    // printf("Placing the block in heap: Size = %zu\n", csize);

    if ((csize - asize) >= (2*DSIZE)) {
        // printf("Place the block: Coming in the loop to split\n");
        free_list_delete(bp);
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(FTRP(bp), PACK(csize - asize, 0));
        free_list_add(bp);
    }
    else {
        // printf("Place the block: NOT Coming in the loop to split\n");
        free_list_delete(bp);
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
    // printf("Placing the block: size after placing: %u\n", GET_SIZE(HDRP(bp)));
    // printf("Placing the block: alloc after placing %u\n", GET_ALLOC(HDRP(bp)));
}

/* Checks for heap consistency */
void mm_check() {
    printf("Heap Consistency Check\n");
    printf("Heap size: %zu\n", mem_heapsize());
    void *bp;
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        printf("Block Size: %u\n", GET_SIZE(HDRP(bp)));
        printf("Block allocation : %u\n", GET_ALLOC(HDRP(bp)));
        printf("\n");
    }

    printf("Free list check \n");
    int i = 0;
    for (bp = free_listp; bp != NULL; bp = NEXT_PTR(bp)) {
        i++;
        printf("Free block size %u\n", GET_SIZE(HDRP(bp)));
        printf("\n");
    }
    return;
}


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    // printf("Calling init package\n");
    // printf("\n");
    /* initial empty heap */ 
    heap_listp = NULL;
    free_listp = NULL;
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;
    PUT(heap_listp, 0); /* Alignment padding */
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1)); /*Prologue header */
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1)); /* Prologue footer */
    PUT(heap_listp + (3*WSIZE), PACK(0, 1)); /* Epilogue header */
    heap_listp += (2*WSIZE);

    if ((heap_listp = extend_heap(CHUNKSIZE/WSIZE)) == NULL)
        return -1;

    // printf("Heap initialized with size = %d\n", GET_SIZE(HDRP(heap_listp)));
    // printf("Heap size according to memsize %zu\n", mem_heapsize() );
    // printf("\n");

    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;      /* Adjusted block size */
    size_t extendsize;   /* Amount to extend if no fit */ 
    char *bp;
    // printf("Malloc call of size: %zu\n", size);
    // printf("\n");

    // printf("Malloc request with size = %zu\n", size);
    if (size == 0)
        return NULL;
    asize = ALIGN(size + SIZE_T_SIZE);
    // printf("Malloc Request: After alignment size of the new block is %zu\n", asize);

    if ((bp = find_fit(asize)) != NULL) {
        // printf("Block after returning from find_fit %u\n", GET_SIZE(HDRP(bp)));
        place(bp, asize);
        // printf("Block after returning from place %u\n", GET_SIZE(HDRP(bp)));
        // mm_check();
        return bp;
    }

    // printf("No fit found, going for extension\n");
    /* No fit found. Get more memory and place the block */
    extendsize = MAX(CHUNKSIZE, asize);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    // mm_check();
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    // printf("Free call for pointer of size: %u\n", GET_SIZE(HDRP(bp)));
    // mm_check();
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
    // mm_check();
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

void free_list_add(void *bp) {
    if(free_listp == NULL) {
        NEXT_PTR(bp) = NULL;
        PREV_PTR(bp) = NULL;
        free_listp = bp;
        return;
    }
    NEXT_PTR(bp) = free_listp;
    PREV_PTR(bp) = NULL;
    PREV_PTR(free_listp) = bp;
    free_listp = bp;
    return;
}

void free_list_delete(void *bp) {
    // printf("Deleting from free list\n");
    if((PREV_PTR(bp) == NULL) && (NEXT_PTR(bp) == NULL)) {
        // printf("Deleting case 4\n");
        free_listp = NULL;
    }
    else if((PREV_PTR(bp) != NULL) && (NEXT_PTR(bp) != NULL)) {
        // printf("Delete Case 1\n");
        NEXT_PTR(PREV_PTR(bp)) = NEXT_PTR(bp);
        PREV_PTR(NEXT_PTR(bp)) = PREV_PTR(bp);
    }
    else if ((PREV_PTR(bp) == NULL) && (NEXT_PTR(bp) != NULL)) {
        // printf("Delete Case 2\n");
        PREV_PTR(NEXT_PTR(bp)) = NULL;
        free_listp = NEXT_PTR(bp);
    }
    else {
        // printf("Delete Case 3\n");
        NEXT_PTR(PREV_PTR(bp)) = NULL;
    }
    // mm_check();
    return;
}











