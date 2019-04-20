#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

template <typename T>
struct List;

template <typename T>
struct List {
	T * arr;
	size_t size;
	size_t capacity;
	Allocator allocator;
	
	void alloc(Allocator allocator = default_allocator);
	T * get_raw();
	List<T> copy();
	void dealloc();
	void resize(size_t new_capacity);
	void possibly_grow_to_size(size_t new_size);
	void push(T to_push);
	void possibly_shrink_to_size(size_t query_size);
	T pop();
	T at(size_t index);
	T& operator[](size_t index);
	static constexpr int   initial_size  = 4;
	static constexpr float grow_factor   = 2.0;
	static constexpr float shrink_line   = 0.25;
	static constexpr float shrink_factor = 0.5;
};

template <typename T>
void List<T>::alloc(Allocator allocator)
{
	size = 0;
	capacity = List::initial_size;
	this->allocator = allocator;
	arr = (T*) allocator.__malloc(sizeof(T) * capacity);
}

template <typename T>
T * List<T>::get_raw()
{
	return arr;
}

template <typename T>
List<T> List<T>::copy()
{
	List<T> list;
	list.size = size;
	list.capacity = capacity;
	list.arr = (T*) allocator.__malloc(sizeof(T) * capacity);
	memcpy(list.arr, arr, sizeof(T) * size);
	return list;
}

template <typename T>
void List<T>::dealloc()
{
	allocator.__free(arr);
	size = 0;
	capacity = 0;
}

template <typename T>
void List<T>::resize(size_t new_capacity)
{
	assert(new_capacity >= size);
	T * new_arr = (T*) allocator.__malloc(sizeof(T) * new_capacity);
	memcpy(new_arr, arr, sizeof(T) * size);
	allocator.__free(arr);
	arr = new_arr;
	capacity = new_capacity;
}

template <typename T>
void List<T>::possibly_grow_to_size(size_t query_size)
{
	if (query_size > capacity) {
		resize(capacity * List::grow_factor);
	}
}

template <typename T>
void List<T>::push(T to_push)
{
	possibly_grow_to_size(size + 1);
	arr[size++] = to_push;
}

template <typename T>
void List<T>::possibly_shrink_to_size(size_t query_size)
{
	if (query_size < (int) (capacity * List::shrink_line)) {
		resize((int) capacity * List::shrink_factor);
	}
}

template <typename T>
T List<T>::pop()
{
	assert(size > 0);
	possibly_shrink_to_size(size - 1);
	return arr[--size];
}

template <typename T>
T List<T>::at(size_t index)
{
	assert(index < size);
	return arr[index];
}

template <typename T>
T& List<T>::operator[](size_t index)
{
	assert(index < size);
	return arr[index];
}
