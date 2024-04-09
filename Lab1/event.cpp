#include"event.hpp"
void Event<void>::Notify()
{
	if (enabled)
		for (int i = observers.size() - 1; i >= 0; i--)
			(*observers[i])();
}
Event<void>& Event<void>::operator+=(Reaction& observer)
{
	observers.push_back(&observer);
	return *this;
}

Event<void>& Event<void>::operator-=(const Reaction& observer)
{
	for (auto it = observers.begin(); it != observers.end(); it++)
		if (&observer == *it)
		{
			observers.erase(it);
			return *this;
		}
	assert(0 && "Unregistering an observer that wasn't registered");
}