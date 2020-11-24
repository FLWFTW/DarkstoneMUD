
#ifndef ITERATOR_HPP_08761A81_0F2B_4C82_B00D_DA36B94245A6
#define ITERATOR_HPP_08761A81_0F2B_4C82_B00D_DA36B94245A6


// Author: David Charles Haley
// (c) 2006 David Charles Haley

// Java-style iterator

namespace JavaIteratorPrivate
{

	template<typename ValueType>
	class IteratorImplBase
	{
		public:
                        virtual ~IteratorImplBase() { /* empty */ }
			virtual bool hasNext() const = 0;
			virtual ValueType next() = 0;
			virtual size_t size() const = 0;
			virtual IteratorImplBase<ValueType> * copy() const = 0;
	};

	template<typename ContainerType>
	class IteratorImpl : public IteratorImplBase<typename ContainerType::value_type>
	{
		public:
			IteratorImpl(const ContainerType & con)
				: container_(con)
                        {
                            it_ = container_.begin();
                        }

                        ~IteratorImpl() { /* empty */ }

			bool hasNext() const
			{
				return it_ != container_.end();
			}

			typename ContainerType::value_type next()
			{
				typename ContainerType::value_type val = *it_;
				it_++;
				return val;
			}

			size_t size() const
			{
				typename ContainerType::const_iterator it2 = it_;

				size_t count = 0;

				while ( it2++ != container_.end() )
					count++;

				return count;
			}

			IteratorImplBase<typename ContainerType::value_type> * copy() const
			{
				return new IteratorImpl<ContainerType>(container_);
			}

		private:
			const ContainerType & container_;
			typename ContainerType::const_iterator it_;
	};

} // namespace JavaIteratorPrivate

template<typename ValueType>
class Iterator
{
	public:
		Iterator(JavaIteratorPrivate::IteratorImplBase<ValueType> * impl = NULL)
		{
			impl_ = impl;
		}

		~Iterator()
		{
			delete impl_;
		}

		bool hasNext() const
		{
			return impl_->hasNext();
		}

		ValueType next()
		{
			return impl_->next();
		}

		/** \brief Return how many elements are left in this iterator.
		 *
		 * \return How many elements are yet to be iterated over.
		 */
		size_t size() const
		{
			return impl_->size();
		}

		Iterator<ValueType> & operator = (const Iterator<ValueType> & rhs)
		{
			// Do we have an implementation? If so, delete it
			if ( impl_ ) delete impl_;

			// Make a copy of the implementation
			impl_ = rhs.impl_->copy();

			return *this;
		}

	protected:
		JavaIteratorPrivate::IteratorImplBase<ValueType> * impl_;
};

template<typename ContainerType>
inline Iterator<typename ContainerType::value_type> MakeIterator(const ContainerType & container)
{
	return Iterator<typename ContainerType::value_type>( new JavaIteratorPrivate::IteratorImpl<ContainerType>(container) );
}

#endif // include guard

