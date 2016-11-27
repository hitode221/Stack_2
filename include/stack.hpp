#pragma once
#include <algorithm>
#include <stdexcept>
#include <new>
#include <mutex>
#include <iostream>

using std::size_t;

template <class T>
T* copy_array(T * array_, size_t size, size_t new_size) { //strong
	T * temp = new T[new_size];
	try {
		std::copy(array_, array_ + size, temp);
	}
	catch (std::exception &e) {
		delete[] temp;
		throw;
	}
	return temp;
}

class bitset {
public:
	bitset(size_t size_ = 0);
	bitset(const bitset& other);
	~bitset();
	auto set(size_t index) -> void;
	auto reset(size_t index) -> void;
	auto test(size_t index) const -> bool;
	auto size()->size_t;
	auto operator [](size_t index) -> bool&;
	auto swap(bitset & other) -> void;
private:
	size_t size_;
	bool *set_;
};

template <class T>
class allocator {
public:
	explicit allocator(size_t size = 0);
	allocator(allocator const & other);
	auto operator =(allocator const & other)->allocator & = delete;
	~allocator();
	auto resize() -> void;
	auto swap(allocator & other) -> void;
	auto construct(T * ptr, T const & value) -> void;
	auto destroy(T * ptr) -> void;
	auto get()->T *;
	auto get() const->T const *;
	auto count() const->size_t;
	auto full() const -> bool;
	auto empty() const -> bool;
private:
	size_t size_;
	size_t count_;
	T* ptr_;
	bitset bitset_;
};

template <class T>
class stack {
public:
	explicit stack(size_t size = 0); /*noexcept*/
	stack(stack const & other) = default; /*strong*/
	~stack(); /*noexcept*/
	auto count() const noexcept->size_t; /*noexcept*/
	auto empty() const noexcept -> bool; /*noexcept*/
	auto top() const -> const T&; /*strong*/
	auto pop() -> void; /*strong*/
	auto push(T const & value) -> void; /*strong*/
	auto operator=(stack const & other)->stack &; /*strong*/
private:
	allocator<T> allocator_;
	mutable std::mutex mutex_;
};
bitset::bitset(size_t size) : size_(size), set_(new bool[size_]) {
	for (size_t i = 0; i < size; ++i) {
		set_[i] = 0;
	}
};

bitset::bitset(const bitset & other) : size_(other.size_), set_(copy_array(other.set_, size_, size_)) {}

bitset::~bitset() {
	delete[] set_;
}

auto bitset::set(size_t index) -> void {
	set_[index] = 1;
}

auto bitset::reset(size_t index) -> void {
	set_[index] = 0;
}

auto bitset::test(size_t index) const -> bool {
	if (index >= 0 && index < size_)
		return set_[index];
	else throw std::range_error("out of range");
}

auto bitset::size() -> size_t {
	return size_;
}

auto bitset::operator [](size_t index) ->bool& {
	if (index >= 0 && index < size_) {
		return set_[index];
	}
	else throw (std::range_error("out of range"));
}

auto bitset::swap(bitset & other) -> void {
	std::swap(size_, other.size_);
	std::swap(set_, other.set_);
}

template<class T>
allocator<T>::allocator(size_t size) : size_(size), count_(0), ptr_((static_cast<T *>(size == 0 ? nullptr : operator new(size * sizeof(T))))), bitset_(size_) {}

template<class T>
allocator<T>::allocator(allocator const & other) : allocator<T>(other.size_) {
	for (size_t i = 0; i < other.size_; ++i) {
		if (other.bitset_.test(i)) {
			construct(ptr_ + i, other.get()[i]);
		}
	}
}

template<class T>
allocator<T>::~allocator() {
	for (T* first = ptr_; first != ptr_ + size_; ++first) {
		if (bitset_.test(first - ptr_))
			destroy(&*first);
	}
	operator delete(ptr_);
}

template<class T>
auto allocator<T>::resize() -> void {
	size_t array_size = size_ * 2 + (size_ == 0);
	allocator<T> temp(array_size);
	for (size_t i = 0; i < size_; ++i) {
		if (bitset_.test(i)) {
			temp.construct(temp.ptr_ + i, ptr_[i]);
		}
	}
	swap(temp);
}

template<class T>
auto allocator<T>::swap(allocator & other) -> void {
	std::swap(ptr_, other.ptr_);
	std::swap(count_, other.count_);
	std::swap(size_, other.size_);
	bitset_.swap(other.bitset_);
}

template<class T>
auto allocator<T>::construct(T * ptr, T const & value) -> void {
	if (ptr < ptr_ || ptr >= ptr_ + size_) {
		throw std::out_of_range("out of range");
	}
	if (bitset_.test(ptr - ptr_)) {
		throw std::bad_alloc();
	}
	new(ptr) T(value);
	++count_;
	bitset_.set(ptr - ptr_);
}

template<class T>
auto allocator<T>::destroy(T * ptr) -> void {
	if (ptr < ptr_ || ptr >= ptr_ + size_) {
		throw std::out_of_range("out of range");
	}
	if (!bitset_.test(ptr - ptr_)) {
		throw std::bad_alloc();
	}
	ptr->~T();
	--count_;
	bitset_.reset(ptr - ptr_);
}

template<class T>
auto allocator<T>::get() -> T * {
	return ptr_;
}

template<class T>
auto allocator<T>::get() const -> T const * {
	return ptr_;
}

template<class T>
auto allocator<T>::count() const -> size_t {
	return count_;
}

template<class T>
auto allocator<T>::full() const -> bool {
	return count_ == size_;
}

template<class T>
auto allocator<T>::empty() const -> bool {
	return count_ == 0;
}

template<class T>
stack<T>::stack(size_t size) : allocator_(size), mutex_() {}

template<class T>
stack<T>::~stack() {
}

template<class T>
auto stack<T>::count() const noexcept -> size_t {
	std::lock_guard<std::mutex> lock(mutex_);
	return allocator_.count();
}

template<class T>
auto stack<T>::empty() const noexcept -> bool {
	std::lock_guard<std::mutex> lock(mutex_);
	return allocator_.empty();
}

template<class T>
auto stack<T>::top() const -> const T &{
	std::lock_guard<std::mutex> lock(mutex_);
	if (allocator_.empty()) {
		throw std::logic_error("empty stack");
	}
return allocator_.get()[allocator_.count() - 1];
}

template<class T>
auto stack<T>::pop() -> void {
	std::lock_guard<std::mutex> lock(mutex_);
	if (allocator_.empty()) {
		throw std::logic_error("empty stack");
	}
	allocator_.destroy(allocator_.get() + allocator_.count() - 1);
}

template<class T>
auto stack<T>::push(T const & value) -> void {
	std::lock_guard<std::mutex> lock(mutex_);
	if (allocator_.full()) {
		allocator_.resize();
	}
	allocator_.construct(allocator_.get() + allocator_.count(), value);
}

template<class T>
auto stack<T>::operator=(stack const & other) -> stack &
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (this != &other) {
		(allocator<T>(other.allocator_)).swap(allocator_);
	}
	return *this;
}
