#pragma once
#include <stdexcept>

using std::size_t;

template <typename T>
T* copy_array(T * array_, size_t size, size_t new_size); //strong
template <typename T1, typename T2>
auto construct(T1 * ptr, T2 const & value) -> void;
template <typename T>
auto destroy(T * ptr) noexcept -> void;
template <typename FwdIter>
auto destroy(FwdIter first, FwdIter last) noexcept -> void;

template <typename T>
class allocator {
protected:
	allocator(size_t size = 0); //noexcept
	~allocator(); //noexcept
	auto swap(allocator & other) noexcept -> void; //noexcept

	allocator(allocator const &) = delete;
	auto operator =(allocator const &) -> allocator & = delete;

	T * array_;
	size_t array_size_;
	size_t count_;
};

template <typename T>
class stack : private allocator<T> {
public:
	stack(size_t size = 0); //noexcept	
	stack(const stack<T> & stack_); //strong
	~stack(); //noexcept
	auto operator =(stack<T> & stack_) -> stack<T> &; //strong 
	auto count() const noexcept -> size_t; //noexcept
	auto size() const noexcept -> size_t; //noexcept
	auto push(T const & element) -> void; //strong
	auto empty() const noexcept -> bool; //noexcept
	auto top()  const throw(std::logic_error) -> T &; //strong
	auto pop()  throw(std::logic_error) -> void; //strong
};

template <typename T>
T* copy_array(T * array_, size_t size, size_t new_size) { //strong
	T * temp = new T[new_size];
	try {
		copy(array_, array_ + size, temp);
	}
	catch (std::exception &e) {
		delete[] temp;
		throw;
	}
	return temp;
}

template <typename T1, typename T2>
void construct(T1 * ptr, T2 const & value) {
	new(ptr) T1(value);
}

template <typename T>
void destroy(T * ptr) noexcept
{
	ptr->~T();
}

template <typename FwdIter>
void destroy(FwdIter first, FwdIter last) noexcept
{
	for (; first != last; ++first) {
		destroy(&*first);
	}
}

template <typename T>
allocator<T>::allocator(size_t size) : array_size_(size), count_(0), array_(static_cast<T *>(size == 0 ? nullptr : operator new(size * sizeof(T)))) {}; //noexcept

template<typename T>
allocator<T>::~allocator() { //noexcept
	operator delete (array_);
}

template<typename T>
auto allocator<T>::swap(allocator<T> & other) noexcept -> void { //noexcept
	std::swap(array_, other.array_);
	std::swap(count_, other.count_);
	std::swap(array_size_, other.array_size_);
};

template <typename T>
stack<T>::stack(size_t size) : allocator<T>(size) {};
template <typename T>
stack<T>::stack(stack const & stack_) : allocator<T>(stack_.array_size_) {
	for (size_t i = 0; i < stack_.count_; ++i) {
		construct(this->array_ + i, stack_.array_[i]);
	}
	this->count_ = stack_.count_;
}
template <typename T>
auto stack<T>::operator =(stack<T> & stack_) -> stack<T> & { //strong 
	if (this != &stack_) {
		stack(stack_).swap(*this);
	}
	return *this;
}
template <typename T>
auto stack<T>::count() const noexcept -> size_t {//noexcept
	return this->count_;
}
template <typename T>
auto stack<T>::size() const noexcept -> size_t {//noexcept
	return this->array_size_;
}
template <typename T>
auto stack<T>::push(T const & element) -> void {//strong
	if (this->count_ == this->array_size_) {
		size_t array_size = this->array_size_ * 2 + (this->array_size_ == 0);

		stack<T> temp{ array_size };
		while (temp.count() < this->count_) {
			temp.push(this->array_[temp.count()]);
		}

		this->swap(temp);
	}

	construct(this->array_ + this->count_, element);
	++this->count_;
}
template <typename T>
auto stack<T>::top() const throw(std::logic_error) -> T& {//strong
	if (this->count_ != 0) {
		return this->array_[this->count_ - 1];
	}
	else throw std::logic_error("Empty stack");
}
template <typename T>
auto stack<T>::pop() throw(std::logic_error)  -> void {//strong
	if (this->count_ != 0) {
		destroy(this->array_ + this->count_ - 1);
		--this->count_;
	}
	else throw std::logic_error("Empty stack");
}
template <typename T>
auto stack<T>::empty() const noexcept -> bool { //noexcept
	return (this->count_ == 0);
}
template<typename T>
stack<T>::~stack() {//noexcept
	destroy(this->array_, this->array_ + this->count_);
}
