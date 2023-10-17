#pragma once

#include "Resources.h"

namespace Threadsafe
{
	template<typename T>
	class tsqueue
	{
	public:
		tsqueue() = default;
		tsqueue(const tsqueue<T>&) = delete;
		virtual ~tsqueue() { clear(); }

	public:
		

	protected:
		std::mutex MtxQueue;
		std::deque<T> DeqQueue;
		std::condition_variable CVBlocking;
		std::mutex MtxBlocking;
	};
}