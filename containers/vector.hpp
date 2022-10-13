#ifndef VECTOR_HPP
# define VECTOR_HPP

# include <memory>
# include <limits>
# include <algorithm>
# include <iterator>

# include "../utils/algorithm.hpp"
# include "../utils/iterator.hpp"
# include "../utils/random_access_iterator.hpp"
# include "../utils/util.hpp"

namespace ft {

template <typename T, typename Allocator = std::allocator<T> >
class vector
{
	public:
		// Member types
		typedef T														value_type;
		typedef Allocator												allocator_type;
		typedef std::size_t												size_type;
		typedef std::ptrdiff_t 											difference_type;
		typedef value_type&												reference;
		typedef const value_type&										const_reference;
		typedef typename Allocator::pointer								pointer;
		typedef typename Allocator::const_pointer						const_pointer;
		typedef typename ft::random_access_iterator<value_type>			iterator;
		typedef typename ft::random_access_iterator<const value_type>	const_iterator;
		typedef typename ft::reverse_iterator<iterator>					reverse_iterator;
		typedef typename ft::reverse_iterator<const_iterator>			const_reverse_iterator;

		// Member functions
		// constructor
		vector()
			: first_(NULL)
			, last_(NULL)
			, reserved_last_(NULL)
			, alloc_(allocator_type())
		{
		}

		explicit vector(const allocator_type &alloc)
			: first_(NULL)
			, last_(NULL)
			, reserved_last_(NULL)
			, alloc_(alloc)
		{
		}

		explicit vector(size_type count,
						const T& value = T(),
						const Allocator& alloc = Allocator())
			: first_(NULL)
			, last_(NULL)
			, reserved_last_(NULL)
			, alloc_(alloc)
		{
			resize(count, value);
		}

		template <typename InputIt>
		vector(InputIt first, InputIt last,
				const Allocator& alloc = Allocator(),
				typename ft::enable_if<!ft::is_integral<InputIt>::value, InputIt>::type* = NULL)
			: first_(NULL)
			, last_(NULL)
			, reserved_last_(NULL)
			, alloc_(alloc)
		{
			reserve(ft::distance(first, last));
			for (InputIt ite = first; ite != last; ++ite, ++last_)
			{
				construct(last_, *ite);
			}
		}

		vector(const vector &r)
			: first_(NULL)
			, last_(NULL)
			, reserved_last_(NULL)
			, alloc_((r.alloc_))
		{
			reserve(r.size());
			pointer dest = first_;
			for (pointer src = r.first_, last_ = r.last_; src != last_; ++dest, ++src)
			{
				construct(dest, *src);
			}
			last_ = first_ + r.size();
		}

		// destructor
		~vector()
		{
			clear();
			deallocate();
		}

		vector &operator=(const vector &r)
		{
			if (this == &r)
				return *this;

			if (size() == r.size())
			{
				std::copy(r.begin(), r.end(), begin());
			}
			else
			{
				if (capacity() >= r.size())
				{
					std::copy(r.begin(), r.begin() + r.size(), begin());
					last_ = first_ + r.size();
					for (pointer src_ptr = r.first_ + r.size(), src_end = r.last_; src_ptr != src_end; ++src_ptr, ++last_)
					{
						construct(last_, *src_ptr);
					}
				}
				else
				{
					clear();
					//destroy_until();
					reserve(r.size());
					for (pointer src_ptr = r.first_, src_end = r.last_; src_ptr != src_end; ++src_ptr, ++last_)
					{
						construct(last_, *src_ptr);
					}
				}
			}
			return *this;
		}

		// operator
		reference operator [](size_type i)				{ return first_[i]; }
		const_reference operator [](size_type i) const	{ return first_[i]; }

		// iterator
		iterator begin() 							{ return first_; }
		const_iterator begin() const 				{ return first_; }
		iterator end() 								{ return last_; }
		const_iterator end() const					{ return last_; }
		reverse_iterator rbegin()					{ return reverse_iterator(last_); }
		const_reverse_iterator rbegin() const		{ return reverse_iterator(last_); }
		reverse_iterator rend()						{ return reverse_iterator(first_); }
		const_reverse_iterator rend() const			{ return reverse_iterator(first_); }

		// function
		size_type size() const 			{ return end() - begin(); }
		bool empty() const				{ return begin() == end(); }
		size_type capacity() const 		{ return reserved_last_ - first_; }
		size_type max_size() const
		{
			size_t div = sizeof(value_type);
			if (div == 1)
				++div;
			return std::numeric_limits<size_type>::max() / div;
		}
		reference at(size_type i)
		{
			if (i >= size())
				throw std::out_of_range("index is out of range.");

			return first_[i];
		}
		const_reference at(size_type i) const
		{
			if (i >= size())
				throw std::out_of_range("index is out of range.");

			return first_[i];
		}
		reference front() 					{ return *first_; }
		const_reference front() const		{ return *first_; }
		reference back() 					{ return *(last_ - 1); }
		const_reference back() const 		{ return *(last_ - 1); }
		pointer data() 						{ return first_; }
		const_pointer data() const 			{ return first_; }
		void clear() 						{ destroy_until(first_); }
		void reserve(size_type sz)
		{
			if (sz <= capacity())
				return;

			pointer old_first = first_;
			pointer old_last = last_;
			size_type old_capacity = capacity();

			pointer ptr = allocate(sz);
			first_ = ptr;
			last_ = first_;
			reserved_last_ = first_ + sz;

			for (pointer old_iter = old_first; old_iter != old_last; ++old_iter, ++last_)
			{
				construct(last_, *old_iter);
			}

			for (pointer ptr = old_last - 1, rend = old_first - 1; ptr != rend; --ptr)
			{
				destroy(ptr);
			}

			alloc_.deallocate(old_first, old_capacity);
		}

		void resize(size_type sz)
		{
			if (sz < size())
			{
				// size_type diff = size() - sz;
				destroy_until(first_ + sz);
				last_ = first_ + sz;
			}
			else if (sz > size())
			{
				reserve(sz);
				for (; last_ != reserved_last_; ++last_)
				{
					construct(last_);
				}
			}
		}

		void resize(size_type sz, const_reference value)
		{
			if (sz < size())
			{
				// size_type diff = size() - sz;
				destroy_until(first_ + sz);
				last_ = first_ + sz;
			}
			else if (sz > size())
			{
				reserve(sz);
				for (; last_ != reserved_last_; ++last_)
				{
					construct(last_, value);
				}
			}
		}

		void push_back(const_reference value)
		{
			if (size() + 1 > capacity())
			{
				size_type c = size();
				if (c == 0)
					c = 1;
				else
					c *= 2;

				reserve(c);
			}
			construct(last_, value);
			++last_;
		}

		void pop_back(void)
		{
			destroy(--last_);
		}

		void assign(size_type count, const T &value)
		{
			if (count > capacity())
			{
				clear();
				deallocate();

				first_ = allocate(count);
				last_ = first_;
				reserved_last_ = first_ + count;

				for (size_type i = 0; i < count; ++i)
					construct(last_++, value);
			}
			else if (count > size())
			{
				pointer ptr = first_;
				for (size_type i = 0; i < count; ++i)
				{
					if (i < size())
						*(ptr++) = value;
					else
						construct(last_++, value);
				}
			}
			else
			{
				pointer new_last = first_ + count;
				std::fill(first_, new_last, value);
				pointer ptr = new_last;
				for (; ptr != last_; ++ptr)
				{
					destroy(ptr);
				}
				last_ = new_last;
			}
		}

		template <class InputIt>
		void assign(InputIt src_first, InputIt src_last,
					typename ft::enable_if<!ft::is_integral<InputIt>::value,
											InputIt>::type * = NULL)
		{
			//size_type count = std::distance(src_first, src_last);
			size_type count = src_last.base() - src_first.base();
			size_type s = size();
			if (count > capacity())
			{
				clear();
				deallocate();

				first_ = allocate(count);
				last_ = first_;
				reserved_last_ = first_ + count;

				for (pointer head = src_first.base(), tail = src_last.base(); head != tail; ++head)
					construct(last_++, *head);

				/*
				for (InputIt head = src_first; head != src_last; ++head)
					construct(last_++, *head);
				*/
			}
			else if (count > s)
			{
				pointer ptr = first_;
				pointer ptr2 = src_first.base();

				for (size_type i = 0; i < count; ++i)
				{
					if (i < s)
						*(ptr++) = *(ptr2++);
					else
						construct(last_++, *(ptr2++));
				}
			}
			else
			{
				pointer new_last = first_ + count;
				//std::copy(src_first, src_last, first_);

				pointer head = src_first.base();
				pointer tail = src_last.base();
				pointer ptr1 = first_;
				for (; head != tail; ++head)
				{
					*ptr1 = *head;
				}

				pointer ptr2 = new_last;
				for (; ptr2 != last_; ++ptr2)
				{
					destroy(ptr2);
				}
				last_ = new_last;
			}
		}

		iterator insert(iterator pos, const T &value)
		{
			size_type insert_size = 1;
			size_type new_size = size() + insert_size;
			size_type diff = pos.base() - first_;

			reserve(new_size);
			construct(last_);

			if (size() == 0)
			{
				*first_ = value;
				last_++;
				return (first_);
			}
			else if (diff == size())
			{
				*last_ = value;
				return (last_++);
			}
			else
			{
				pointer ptr = last_ - 1;
				for (; ptr != first_ + diff - 1; --ptr)
				{
					*(ptr + 1) = *ptr;
				}
				*(ptr++) = value;
				last_++;
				return (ptr);
			}
		}

		/*
		iterator insert(iterator pos, const T &value)
		{
			size_type count = 1;
			difference_type offset = pos - begin();

			size_type c = capacity();
			size_type pre_c = c;
			size_type new_size = size() + count;
			while (new_size > c)
			{
				if (c == 0)
					c = 1;
				else
					c = c << 1;
				if ((c >> 1) != pre_c)
					throw std::overflow_error("vector::insert");
				pre_c = c;
			}
			reserve(c);
			for (; last_ != first_ + new_size; ++last_)
				construct(last_);

			iterator tail = last_ - 1;
			iterator range_end = begin() + offset + count - 1;

			for (; tail > range_end; --tail)
				*tail = *(tail - count);


			iterator range_begin = begin() + offset - 1;
			for (; tail > range_begin; --tail)
				*tail = value;

			return begin() + offset;
		}
		*/

		void insert(iterator pos, size_type count, const T &value)
		{
			if (count < 0)
				throw std::length_error("negative length.");
			if (count == 0)
				return;

			difference_type offset = pos - begin();
			size_type c = capacity();
			size_type pre_c = c;
			size_type new_size = size() + count;
			while (c < new_size)
			{
				if (c == 0)
					c = 1;
				else
					c = c << 1;
				if ((c >> 1) != pre_c)
					throw std::overflow_error("vector::insert");
				pre_c = c;
			}
			reserve(c);
			for (; last_ != first_ + new_size; ++last_)
				construct(last_);

			iterator tail = last_ - 1;
			iterator range_end = begin() + offset + count - 1;
			for (; tail > range_end; --tail)
				*tail = *(tail - count);
			iterator range_begin = begin() + offset - 1;
			for (; tail > range_begin; --tail)
				*tail = value;
		}

		template <class InputIt>
		void insert(iterator pos, InputIt src_first, InputIt src_last,
					typename ft::enable_if<!ft::is_integral<InputIt>::value,
											InputIt>::type * = NULL)
		{
			difference_type count = src_last - src_first;
			if (count < 0)
				throw std::length_error("negative length.");
			if (count == 0)
				return;

			difference_type offset = pos - begin();
			size_type c = capacity();
			size_type pre_c = c;
			size_type new_size = size() + count;
			while (c < new_size)
			{
				if (c == 0)
					c = 1;
				else
					c = c << 1;
				if ((c >> 1) != pre_c)
					throw std::overflow_error("vector::insert");
				pre_c = c;
			}
			reserve(c);
			for (; last_ != first_ + new_size; ++last_)
				construct(last_);

			iterator tail = last_ - 1;
			iterator range_end = begin() + offset + count - 1;
			for (; tail > range_end; --tail)
				*tail = *(tail - count);
			iterator range_begin = begin() + offset - 1;
			--src_last;
			for (; src_last > src_first - 1; --src_last)
				*tail-- = *src_last;
		}

		iterator erase(iterator pos)
		{
			if (first_ == last_)
				return NULL;

			difference_type offset = pos - begin();

			for (pointer src = pos.base() + 1; src < last_; ++src)
			{
					*(src - 1) = *src;
			}

			/*
			for (iterator src = pos + 1; src < end(); ++src)
			{
				*(src - 1) = *src;
			}
			*/
			destroy(--last_);
			return (begin() + offset);
		}

		iterator erase(iterator first, iterator last)
		{
			if (first_ == last_)
				return NULL;

			difference_type destroy_diff = last - first;
			std::copy(last.base(), last_, first.base());
			/*
			for (iterator dst = first, src = last; src < end(); ++dst, ++src)
			{
				*dst = *src;
			}
			*/
			destroy_until(last_ - destroy_diff);
			//destroy_until(rbegin() + destroy_diff);

			return (first);
		}

		void swap(vector &other)
		{
			pointer save_first = other.first_;
			pointer save_last = other.last_;
			pointer save_reserved_last = other.reserved_last_;
			allocator_type save_alloc = other.alloc_;

			other.first_ = this->first_;
			other.last_ = this->last_;
			other.reserved_last_ = this->reserved_last_;
			other.alloc_ = this->alloc_;

			this->first_ = save_first;
			this->last_ = save_last;
			this->reserved_last_ = save_reserved_last;
			this->alloc_ = save_alloc;
		}

		allocator_type get_allocator() const
		{
			return (alloc_);
		}

	private:

		// function
		pointer allocate(size_type n)
		{
			if (n > max_size())
				throw std::length_error("vector");
			return alloc_.allocate(n);
		}

		void deallocate()
		{
			if (first_ == NULL)
				return;
			alloc_.deallocate(first_, capacity());
			first_ = NULL;
			last_ = NULL;
			reserved_last_ = NULL;
		}

		void construct(pointer ptr) { alloc_.construct(ptr, value_type()); }
		void construct(pointer ptr, const_reference value) { alloc_.construct(ptr, value); }
		void destroy(pointer ptr) { alloc_.destroy(ptr); }

		void destroy_until(pointer new_last)
		{
			if (new_last == last_)
				return;

			pointer ptr = last_ - 1;
			for (; ptr != new_last; --ptr, --last_)
			{
				destroy(ptr);
			}
			destroy(ptr);
			--last_;
		}
		/*
		void destroy_until(reverse_iterator rend)
		{
			for (reverse_iterator riter = rbegin(); riter != rend; ++riter, --last_)
			{
				destroy(&*riter);
			}
		}
		*/

	protected:
		// data member
		pointer first_;
		pointer last_;
		pointer reserved_last_;
		allocator_type alloc_;

}; // class vector

// Nom-member function
template <typename T, typename Allocator>
bool operator==(const ft::vector<T, Allocator>& x, const ft::vector<T, Allocator>& y)
{
	return x.size() == y.size() && ft::equal(x.begin(), x.end(), y.begin());
}

template <typename T, typename Allocator>
bool operator!=(const ft::vector<T, Allocator>& x, const ft::vector<T, Allocator>& y)
{
	return !(x == y);
}

template <typename T, typename Allocator>
bool operator<(const ft::vector<T, Allocator>& x, const ft::vector<T, Allocator>& y)
{
	return ft::lexicographical_compare(x.begin(), x.end(), y.begin(), y.end());
}

template <typename T, typename Allocator>
bool operator>(const ft::vector<T, Allocator>& x, const ft::vector<T, Allocator>& y)
{
	return y < x;
}

template <typename T, typename Allocator>
bool operator>=(const ft::vector<T, Allocator>& x, const ft::vector<T, Allocator>& y)
{
	return !(x < y);
}

template <typename T, typename Allocator>
bool operator<=(const ft::vector<T, Allocator>& x, const ft::vector<T, Allocator>& y)
{
	return !(y < x);
}

template <class T, class Alloc>
void swap(ft::vector<T, Alloc> &x, ft::vector<T, Alloc> &y)
{
	x.swap(y);
}

} // namespace ft

#endif
