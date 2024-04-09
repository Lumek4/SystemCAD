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
	typedef std::function<void(T)> Reaction;
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
	for (int i = observers.size()-1; i >= 0; i--)
		(*observers[i])(arg);
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
	typedef std::function<void()> Reaction;
	template<typename checked>
	struct is_reaction { static constexpr bool value = std::is_same<checked, Reaction>::value; };
	Event& operator+=(Reaction& observer);
	Event& operator-=(const Reaction& observer);
private:
	bool enabled = true;
	std::vector<Reaction*> observers;
};

