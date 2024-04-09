#pragma once
#include<vector>
#include<cassert>
#include"component.h"

class CatalogueBase
{
public:
	virtual void Register(Component* component) = 0;
	virtual void Unregister(Component* component) = 0;
};

template<class T>
class Catalogue : public CatalogueBase
{
public:
	static Catalogue<T> Instance;
public:
	void Register(Component* component) override
	{
		static_assert(std::is_base_of<Component, T>::value,
			"Registered object must be derived from Component.");
		auto* p = static_cast<T*>(component);
		assert(p && "Registering in the wrong catalogue.");
		components.push_back(p);
	}
	void Unregister(Component* component) override
	{
		static_assert(std::is_base_of<Component, T>::value,
			"Unregistered object must be derived from Component.");
		auto* p = static_cast<T*>(component);
		assert(p && "Unregistering from the wrong catalogue.");
		for(auto i = components.begin(); i!=components.end(); i++)
			if (p == static_cast<Component*>(*i))
			{
				components.erase(i);
				return;
			}
		assert(!"Unregistering an unregistered element");
	}

	const std::vector<T*>& GetAll()
	{
		static_assert(std::is_base_of<Component, T>::value,
			"Template argument must be derived from Component.");
		return components;
	}
private:
	Catalogue<T>() { assert(this == &Instance); }
	std::vector<T*> components;
};


template<typename T>
Catalogue<T> Catalogue<T>::Instance{};