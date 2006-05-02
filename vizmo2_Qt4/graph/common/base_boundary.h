#ifndef BASEBOUNDARY_H
#define BASEBOUNDARY_H

#include "Defines.h"
#include "runtime.h"

template<class Iterator>
class base_boundary {
public:
	typedef Iterator iterator;

  void define_type(stapl::typer &t) {
		stapl_assert(1,"pRange base boundary define_type used.\n");
	}

	/**
	 * Check if the argument points to the start of the boundary
	 * \param i Iterator pointing to an element
	 * \return bool indicating if the iterator is pointing to
	 *         the first element of the boundary
	 */
	virtual bool at_start(const iterator &i) const;

	/**
	 * Check if the argument points to the end of the boundary
	 * \param i Iterator pointing to an element
	 * \return bool indicating if the iterator is pointing to
	 *         the last element of the boundary
	 */
	virtual bool at_finish(const iterator &i) const;

	/** 
	 * Get the start iterator
	 * \return iterator pointing to the first element of the boundary
	 */
	virtual iterator start() const;

	/**
	 * Get the finish iterator
	 * \return iterator pointing to the last element of the boundary
	 */
	virtual const iterator finish() const;

	/** 
	 * Set the start iterator
	 * \param i iterator pointing to the new begin of the boundary
	 */
	virtual void start(const iterator &i);

	/** 
	 * Set the finish iterator
	 * \param i iterator pointing to the new end of the boundary
	 */
	virtual void finish(const iterator &i);


	// In addition to the member functions above the == operator should also be
	// defined.
};

#endif
