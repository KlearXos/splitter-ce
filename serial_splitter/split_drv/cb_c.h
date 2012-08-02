/* DESCRIPTION
 *	circular buffer implementation
 * LANG
 *	C
 * Autor
 *	ation
 */

#ifndef _CB_C_HEADER_
#define _CB_C_HEADER_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif //__cpluspluss

typedef int (*_on_overwrite) (size_t bytes);

/*
 * cb_alloc - allocate context of circular buffer
 *	_size - size of buffer
 *	on_overwrite -	routine wich will be called od
 *			data overwriting
 */
void*	cb_alloc(size_t _size, _on_overwrite on_overwrite);
/*
 * cb_resize - set new buffer size
 */
void*	cb_resize(void *cb_ctx, size_t new_size);
/*
 * set_function - set new overwrite notification function
 */
void	set_function(_on_overwrite on_overwrite);
/*
 * cb_free - free context
 */
void	cb_free(void *cb_ctx);


int cb_isempty(void *cb_ctx);
/*
 * cb_pop - get data from buffer
 *	buffer - target buffer
 *	size - size of data to be copied to buffer
 * Return - size of data actually copied
 */
size_t	cb_pop(void *cb_ctx, unsigned char *buffer, size_t size);
/*
 * cb_push - push data in circular buffer
 *	buffer - source buffer
 *	size - size of data to be copied to circular buffer
 * return - size of data pushed in circular buffer
 */
size_t	cb_push(void *cb_ctx, const unsigned char *buffer, size_t size);

/*
 * cb_copy - copy data without deleting from circular buffer
 *	buffer - target buffer
 *	size - size of data to be copied to buffer
 * return - size of data copied to buffer
 */ 
size_t	cb_copy(void *cb_ctx, unsigned char *buffer, size_t size);

/*
 * cb_del - delete data from buffer
 *	size - size of data for delete
 * return - size of deleted data
 */
size_t	cb_del(void *cb_ctx, size_t size);

/* return size of data in circular buffer */
size_t	cb_datasize(void *cb_ctx);
/* return size of circular buffer */
size_t	cb_buffersize(void *cb_ctx);
/* return size of free space*/
size_t	cb_freespace(void *cb_ctx);

/* erase all data in buffer */
void	cb_erase(void *cb_ctx);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif