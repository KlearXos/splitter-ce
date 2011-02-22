#ifndef __CIRCULAR_BUFFER_IMPLEMENTATION_H__
#define __CIRCULAR_BUFFER_IMPLEMENTATION_H__

// use allocator class
class CAlloc {
public:
	CAlloc(void) {};
	~CAlloc(void) {};

	void*	Allocate(unsigned long size);
	bool	Free(void *pointer);
};

class CCircBuffer {
private:
	unsigned char	*m_buffer;
	unsigned char	*m_head;
	unsigned char	*m_tail;

	unsigned long	m_buffer_size;
	bool		m_is_empty;

	static CAlloc	allocator;
public:
	CCircBuffer(void);
	CCircBuffer(unsigned long init_size);
	CCircBuffer(const CCircBuffer &copy_this);

	virtual ~CCircBuffer(void);

	// return data size stored in buffer
	unsigned long	get_data_size(void) const throw();

	unsigned long	get_free_space_size(void) const throw();
	// return current buffer size
	unsigned long	get_buffer_size(void) const throw();

	// set buffer size (all data will be erased)
	bool		set_buffer_size(unsigned long new_size) throw();

	// return true if empty
	bool		is_empty(void) const throw();
	// return true is data exist
	operator	bool(void) const throw();

	// push data to buffer. stored data size returned
	unsigned long	push(void *buffer, unsigned long size) throw();
	unsigned long	push(const CCircBuffer *buffer, unsigned long size) throw();

	// pop data from buffer. copied data size returned
	unsigned long	pop(void *buffer, unsigned long size) throw();
	unsigned long	pop(CCircBuffer *buffer, unsigned long size) throw();

	// copy data from buffer but not delete. copied data size returned
	unsigned long	copy(void *buffer, unsigned long size) const throw();
	unsigned long	copy(CCircBuffer *buffer, unsigned long size) const throw();

	// delete specefied data size. deleted data size returned
	unsigned long	del(unsigned long size);

	void		erase(void) throw();
protected:
	virtual unsigned long resize_buffer(unsigned long new_size) throw();
};

#endif