#pragma once
#include<vector>
#include<functional>
#include<cassert>

template<typename T>
class Event
{
public:
	Event() = default;
	void Notify(T);
	struct Reaction
	{
		typedef void(Function)(void*, T);
		Reaction(void* arg, void(&func)(void*, T)) 
			:arg(arg), func(func) {}
		void operator()(T arg2)
		{ func(arg, arg2); }
	private:
		void* const arg;
		void(&func)(void*, T);
	};
	static void Unregisterer(void* event, void* reaction);
	template<typename checked>
	struct is_reaction { static constexpr bool value = std::is_same<checked, Reaction>::value; };
	Event& operator+=(Reaction& observer);
	Event& operator-=(const Reaction& observer);
	bool enabled = true;

private:
	//T& owner;
	std::vector<Reaction*> observers;
};


template<typename T>
void Event<T>::Notify(T arg)
{
	if(enabled)
		for (int i = observers.size() - 1; i >= 0; i--)
		{
			(*observers[i])(arg);
			i = (i < observers.size()) ? i : observers.size();
		}
}

template<typename T>
inline void Event<T>::Unregisterer(void* event, void* reaction)
{
	auto ev = (Event<T>*)event;
	auto re = (Event<T>::Reaction*)reaction;
	*ev -= *re;
}


template<typename T>
Event<T>& Event<T>::operator+=(Reaction& observer)
{
	observers.push_back(&observer);
	return *this;
}

template<typename T>
Event<T>& Event<T>::operator-=(const Reaction& observer)
{
	for (auto it = observers.begin(); it != observers.end(); it++)
		if (&observer == *it)
		{
			observers.erase(it);
			return *this;
		}
	assert(0 && "Unregistering an observer that wasn't registered");
}

template<>
class Event<void>
{
public:
	Event() = default;
	void Notify();
	struct Reaction
	{
		typedef void(Function)(void*);
		Reaction(void* arg, void(&func)(void*))
			:arg(arg), func(func) {}
		void operator()()
		{
			func(arg);
		}
	private:
		void* const arg;
		void(&func)(void*);
	};
	static void Unregisterer(void* event, void* reaction);
	template<typename checked>
	struct is_reaction { static constexpr bool value = std::is_same<checked, Reaction>::value; };
	Event& operator+=(Reaction& observer);
	Event& operator-=(const Reaction& observer);
private:
	bool enabled = true;
	std::vector<Reaction*> observers;
};

