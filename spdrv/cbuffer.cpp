#include "cbuffer.h"

#include <windows.h>

CAlloc	CCircBuffer::allocator;

CCircBuffer::CCircBuffer(void) :m_buffer(NULL), m_head(NULL), m_tail(NULL), m_buffer_size(0), m_is_empty(TRUE) {
	//m_buffer = m_head = m_tail = NULL;
	// m_buffer_size = 0;
	//m_is_empty = TRUE;
}

CCircBuffer::CCircBuffer(unsigned long init_size) {
	m_is_empty = TRUE;

	m_buffer = (unsigned char*)allocator.Allocate(init_size);
	if (m_buffer != NULL) {
		m_head = m_tail = m_buffer;
		m_buffer_size = init_size;
	} else {
		// you can raise exception
		m_head = m_tail = NULL;
		m_buffer_size = 0;
	}
}
CCircBuffer::CCircBuffer(const CCircBuffer &copy_this) {
	m_is_empty = TRUE;

	m_buffer = (unsigned char*)allocator.Allocate(copy_this.m_buffer_size);
	if (m_buffer != NULL) {
		m_head = m_tail = m_buffer;
		m_buffer_size = copy_this.m_buffer_size;

		push(&copy_this, m_buffer_size);
	} else {
		m_tail = m_head = NULL;
		m_buffer_size = 0;
	}
}

CCircBuffer::~CCircBuffer(void) {
	if (m_buffer != NULL) allocator.Free(m_buffer);
}

// return data size stored in buffer
unsigned long
CCircBuffer::get_data_size(void) const throw() {
	if (m_buffer_size == 0) return 0;
	if (m_is_empty) return 0;

	return (m_head >= m_tail
	    ? m_buffer_size + m_tail - m_head
	    : m_tail - m_head);
}

unsigned long
CCircBuffer::get_free_space_size(void) const throw() {
	if (m_buffer_size == 0) return 0;
	if (m_is_empty) return m_buffer_size;

	return (m_head >= m_tail
	    ? m_head - m_tail
	    : m_buffer_size + m_head - m_tail);
}

// return current buffer size
unsigned long
CCircBuffer::get_buffer_size(void) const throw() {
	return m_buffer_size;
}

// return true if empty
bool
CCircBuffer::is_empty(void) const throw() {
	return m_is_empty;
}
// return true is data exist
CCircBuffer::operator bool(void) const throw() {
	return (!m_is_empty);
}

// push data to buffer. stored data size returned
unsigned long
CCircBuffer::push(void *push_buffer, unsigned long size) throw() {
	unsigned long	free_space;
	unsigned long	result(0);

	if ((push_buffer == NULL) || (size == 0)) return 0;

	free_space = get_free_space_size();
	if (free_space < size) {
		if (resize_buffer(m_buffer_size + size - free_space) == 0) return 0;
	}

	result = m_buffer_size - (m_tail - m_buffer);
	if (result > size) result = size;

	memcpy(m_tail, push_buffer, result);
	m_tail += result;

	if (m_tail == m_buffer + m_buffer_size) m_tail = m_buffer;

	if (result != size) {
		memcpy(m_tail, (unsigned char*)push_buffer + result, size - result);
		m_tail += size - result;
	}

	m_is_empty = FALSE;

	return size;
}

unsigned long
CCircBuffer::push(const CCircBuffer *push_buffer, unsigned long size) throw() {
	unsigned long	free_space;
	unsigned long	result(0);
	unsigned long	to_copy;
	unsigned long	temp;
	unsigned long	current_size;
	unsigned char	*source;

	if ((push_buffer == NULL) || (size == 0)) return 0;
	if (push_buffer->m_is_empty) return 0;

	current_size = push_buffer->get_data_size();
	if (current_size > size ) current_size = size;

	free_space = get_free_space_size();
	if (free_space < current_size) {
		resize_buffer(m_buffer_size + current_size - free_space);
	}

	source = push_buffer->m_head;
	while (result != current_size) {
		to_copy = current_size - result;

		temp = push_buffer->m_buffer_size - (source - push_buffer->m_buffer);
		if (to_copy > temp) to_copy = temp;

		temp = m_buffer_size - (m_tail - m_buffer);
		if (to_copy > temp) to_copy = temp;

		memcpy(m_tail, source, to_copy);
		result += to_copy;

		source += to_copy;
		if (source == push_buffer->m_buffer + push_buffer->m_buffer_size) source = push_buffer->m_buffer;

		m_tail += to_copy;
		if (m_tail == m_buffer + m_buffer_size) m_tail = m_buffer;
	}

	m_is_empty = FALSE;
	return current_size;
}

// pop data from buffer. copied data size returned
unsigned long
CCircBuffer::pop(void *pop_buffer, unsigned long size) throw() {
	unsigned long	result(0);
	unsigned long	data_size = get_data_size();

	if ((pop_buffer == NULL) || (size == 0)) return 0;
	if (data_size == 0) return 0;

	if (size < data_size) {
		data_size = size;
	} else {
		m_is_empty = TRUE;
	}

	result = m_buffer_size - (m_head - m_buffer);
	if (result > data_size) result = data_size;
	memcpy(pop_buffer, m_head, result);
	m_head += result;
	if (m_head == m_buffer + m_buffer_size) m_head = m_buffer;

	if (result != data_size) {
		m_head = m_buffer + (data_size - result);
		memcpy((unsigned char *)pop_buffer + result, m_buffer, data_size - result);
	}

	if (m_is_empty) {
		m_head = m_tail = m_buffer;
	}

	return data_size;
}

unsigned long
CCircBuffer::pop(CCircBuffer *buffer, unsigned long size) throw() {
	unsigned long	result;

	if ((buffer == NULL) || (size == 0)) return 0;

	result = buffer->push(this, size);
	del(result);

	return result;
}

// copy data from buffer but not delete. copied data size returned
unsigned long
CCircBuffer::copy(void *copy_buffer, unsigned long size) const throw() {
	unsigned long	result(0);
	unsigned long	data_size = get_data_size();

	if ((copy_buffer == NULL) || (size == 0)) return 0;
	if (data_size == 0) return 0;

	if (size < data_size) data_size = size;

	if (m_tail <= m_head) {
		result = m_buffer_size - (m_head - m_buffer);
		if (result > data_size) result = data_size;
	} else {
		result = data_size;
	}

	memcpy(copy_buffer, m_head, result);
	if (result != data_size) {
		memcpy((unsigned char*)copy_buffer+ result, m_buffer, data_size - result);
	}

	return data_size;
}

unsigned long
CCircBuffer::copy(CCircBuffer *buffer, unsigned long size) const throw() {
	if ((buffer == NULL) || (size == 0)) return 0;

	return buffer->push(this, size);
}

// delete specefied data size. deleted data size returned
unsigned long
CCircBuffer::del(unsigned long size) {
	unsigned long	data_size;

	data_size = get_data_size();
	if (size >= data_size) {
		erase();
	} else {
		data_size = size;

		m_head += data_size;
		if (m_head >= m_buffer + m_buffer_size) m_head -= m_buffer_size;
	}

	return data_size;
}

void
CCircBuffer::erase(void) throw() {
	m_head = m_tail = m_buffer;
	m_is_empty = TRUE;
}

unsigned long
CCircBuffer::resize_buffer(unsigned long new_size) throw() {
	unsigned char	*temp_buffer;
	unsigned long	data_size;

	temp_buffer = (unsigned char*)allocator.Allocate(new_size);
	if (temp_buffer == NULL) {
		return 0;
	}

	data_size = get_data_size();

	copy(temp_buffer, data_size);

	allocator.Free(m_buffer);

	m_buffer = m_head = temp_buffer;
	m_tail = m_buffer + data_size;
	m_buffer_size = new_size;

	if (m_tail == m_buffer + m_buffer_size) m_tail = m_buffer;

	return new_size;
}

bool
CCircBuffer::set_buffer_size(unsigned long new_size) throw () {
	bool	result;

	if (m_buffer != NULL) {
		allocator.Free(m_buffer);
	}

	m_is_empty = TRUE;

	m_buffer = (unsigned char*)allocator.Allocate(new_size);
	if (m_buffer != NULL) {
		m_head = m_tail = m_buffer;
		m_buffer_size = new_size;

		result = true;
	} else {
		// you can raise exception
		m_head = m_tail = NULL;
		m_buffer_size = 0;
		result = false;
	}

	return result;
}

void*
CAlloc::Allocate(unsigned long size) {
	return LocalAlloc(LMEM_ZEROINIT, size);
}

bool
CAlloc::Free(void *pointer) {
	return (LocalFree(pointer) == NULL);
}