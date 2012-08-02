#include "cb_c.h"

#include <windows.h>

#include "allocation.h"

struct __cb {
	PUCHAR	buffer;
	PUCHAR	head;
	PUCHAR	tail;

	size_t	size;
	BOOL	empty;

	_on_overwrite	function;
};

void*
cb_alloc(size_t _size, _on_overwrite on_overwrite)
{
	struct __cb *ctx;

	ctx = (struct __cb *)AllocateMemory(sizeof(*ctx));

	if (ctx != NULL) {
		ctx->buffer = (PUCHAR)AllocateMemory(_size);
		if (ctx->buffer == NULL) {
			LocalFree(ctx);
			ctx = NULL;
		} else {
			ctx->function = on_overwrite;
			ctx->size = _size;
			ctx->head = ctx->tail = ctx->buffer;
			ctx->empty = TRUE;
		}
	}

	return ctx;
}

void
cb_free(void *cb_ctx)
{
	struct __cb *ctx = (struct __cb *)cb_ctx;

	if (ctx == NULL) 
		return;

	FreeMemory(ctx->buffer);
	FreeMemory(ctx);
}

size_t
cb_buffersize(void *cb_ctx)
{
	return (cb_ctx == NULL ? 0 : ((struct __cb*)cb_ctx)->size);
}

void
cb_erase(void *cb_ctx)
{
	struct __cb *ctx;

	ctx = (struct __cb*)cb_ctx;
	if (ctx != NULL) {
		ctx->head = ctx->tail = ctx->buffer;
		ctx->empty = TRUE;
	}
}

size_t
cb_datasize(void *cb_ctx)
{
	struct __cb *ctx;

	ctx = (struct __cb*)cb_ctx;

	if (ctx == NULL)
		return 0;

	if (ctx->empty)
		return 0;

	return (ctx->tail <= ctx->head 
		? (ctx->size - (ULONG)ctx->head + (ULONG)ctx->tail) 
		: ctx->tail - ctx->head);
}

size_t
cb_pop(void *cb_ctx, PUCHAR buffer, size_t size)
{
	size_t	result;
	size_t	to_copy;
	size_t	temp;
	struct __cb *ctx;
	BOOL	loop;

	PUCHAR	source;

	ctx = (struct __cb *)cb_ctx;

	if ((ctx == NULL) || 
	    (buffer == NULL)) {
			return 0;
	}
	if ((ctx->empty) || (size == 0)) {
		return 0;
	}

	result = 0;
	loop = TRUE;

	do {
		source = ctx->head;
		temp = size - result;

		if (ctx->tail <= ctx->head) {
			to_copy = ctx->buffer + ctx->size - ctx->head;
			if (temp >= to_copy) {
				ctx->head = ctx->buffer;
			} else {
				to_copy = temp;
				ctx->head = ctx->head + temp;
			}
		} else {
			to_copy = ctx->tail - ctx->head;

			if (temp >= to_copy ) {
				/* buffer contains less date than requested */
				loop = FALSE;
				ctx->head = ctx->tail = ctx->buffer;
				ctx->empty = TRUE;
			} else {
				to_copy = temp;
				ctx->head = ctx->head + temp;
			}
		}

		memcpy(buffer + result, source, to_copy);
		result += to_copy;
		if (result == size) loop = FALSE;
	} while (loop);

	return result;
}

size_t
cb_push(void *cb_ctx, const unsigned char *buffer, size_t size)
{
	size_t		result;
	size_t		to_copy;
	size_t		temp;
	struct __cb	*ctx;
	const unsigned char	*source;

	ctx = (struct __cb*)cb_ctx;
	if ((ctx == NULL) ||
	    (buffer == NULL) || 
	    (size == 0))
		return 0;

	if (size >= ctx->size) {
		result = ctx->size;

		if (ctx->function != NULL) {
			if (!ctx->function(size - ctx->size)) {
				result = 0;
			}
		}

		memcpy(ctx->buffer, buffer + size - ctx->size, result);
		if (result) {
			ctx->head = ctx->tail = ctx->buffer;
		}

	} else {
		/* data saved without overwriting */
		result = 0;
		if (ctx->function != NULL) {
			int	diff;
			
			if (ctx->empty) {
				diff = ctx->size;
			} else {
				/* free space */
				diff = (ctx->tail <= ctx->head
					? ctx->head - ctx->tail
					: ctx->size + ctx->head - ctx->tail);
				diff = (int)size - (int)diff;
				if (diff > 0) {
					if (!ctx->function (diff)) {
						return 0;
					}
				}
			}
		}
		do {
			temp = size - result;
			to_copy = ctx->buffer + ctx->size - 
			    ctx->tail;

			if (temp < to_copy )
				to_copy = temp;

			source = buffer + result;

			memcpy(ctx->tail, source, to_copy);
			result += to_copy;

			source = ctx->tail;
			ctx->tail = ctx->tail + to_copy;
			if (ctx->tail == ctx->buffer + ctx->size)
				ctx->tail = ctx->buffer;

			if (ctx->head == source) {
				if (!ctx->empty)
					ctx->head = ctx->tail;
			} else if (ctx->head > source) {
				/* overwrite possible */
				if (ctx->head < source + to_copy) {
					/* calculate overwrite */
					ctx->head = ctx->tail;
				}
			}
		} while (result != size);
	}

	if (result != 0)
		ctx->empty = FALSE;
	return result;
}

size_t
cb_copy(void *cb_ctx, unsigned char *buffer, size_t size) {
	size_t	result;
	size_t	to_copy;
	size_t	temp;
	struct __cb *ctx;
	BOOL	loop;

	PUCHAR	source;
	PUCHAR	head;
	PUCHAR	tail;

	ctx = (struct __cb *)cb_ctx;

	if ((ctx == NULL) || 
	    (buffer == NULL)) {
			return 0;
	}
	if ((ctx->empty) || (size == 0)) {
		return 0;
	}

	result = 0;
	loop = TRUE;

	head = ctx->head;
	tail = ctx->tail;

	do {
		source = head;
		temp = size - result;

		if (tail <= head) {
			to_copy = ctx->buffer + ctx->size - head;
			if (temp >= to_copy) {
				head = buffer;
			} else {
				to_copy = temp;
				head = head + temp;
			}
		} else {
			to_copy = tail - head;

			if (temp >= to_copy ) {
				/* buffer contains less date than requested */
				loop = FALSE;
				head = tail = ctx->buffer;
			} else {
				to_copy = temp;
				head = head + temp;
			}
		}

		memcpy(buffer + result, source, to_copy);
		result += to_copy;
		if (result == size) loop = FALSE;
	} while (loop);

	return result;
}

size_t
cb_del(void *cb_ctx, size_t size) {
	struct __cb *ctx;
	size_t	data_size;

	ctx = (struct __cb*)cb_ctx;

	if (ctx == NULL)
		return 0;

	if (ctx->empty)
		return 0;

	data_size = (ctx->tail <= ctx->head 
	    ? (ctx->size - (ULONG)ctx->head + (ULONG)ctx->tail) 
	    : ctx->tail - ctx->head);
	if (data_size <= size) {
		ctx->head = ctx->tail = ctx->buffer;
		ctx->empty = TRUE;

	} else {
		// move head pointer
		ctx->head += size;
		if (ctx->head >= ctx->buffer + ctx->size) { // XXX - > or >= ?..
			ctx->head -= ctx->size;
		}
		data_size = size;
	}
	return data_size;
}

size_t
cb_freespace(void *cb_ctx)
{
	struct __cb *ctx;

	ctx = (struct __cb*)cb_ctx;

	if (ctx == NULL)
		return 0;

	if (ctx->empty)
		return ctx->size;

	return (ctx->tail <= ctx->head
		? ctx->head - ctx->tail
		: ctx->size + ctx->head - ctx->tail);
}

int
cb_isempty(void *cb_ctx)
{
	return ((struct __cb *)cb_ctx)->empty;
}