#ifndef __MEM_H__
#define __MEM_H__

/* memory management */
/* these are weak symbols, can be override by 
 * user 
 */
void *hs_calloc(size_t, size_t);
void *hs_realloc(void *, size_t);
void  hs_free(void*);
void *hs_malloc(size_t size);




#endif
