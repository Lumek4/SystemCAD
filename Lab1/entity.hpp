#pragma once
#include<memory>
#include<cassert>
#include"catalogue.hpp"
#include"component.h"
#include"exceptions.h"
#include<string_view>
#include<DirectXMath.h>
#include"event.hpp"

class EntityCollection;

class Entity
{
public:
	static Entity* New();
	void Delete();

	bool enabled = true;
	bool hideInList = false;

	Event<Entity*> preDelete{  };
	Event<Entity*> preSelect{  };
	Event<Entity*> onSelect{  };
	typedef std::vector<Entity*> Selection;
	static Event<void> preAnySelect;

	bool Merge(Entity* other);
private:
	std::vector<std::tuple<void*, void*, Event<void>::Reaction>> registered;
public:
	template<typename event, typename reaction>
	void Register(event& e, reaction& r)
	{
		e += r;
		registered.emplace_back(std::make_tuple(&e,&r,
			[&]() {
				e -= r;
			}));
	}
	template<typename event, typename reaction>
	void Unregister(event& e, reaction& r)
	{
		for (auto it = registered.begin(); it != registered.end(); it++)
		{
			auto& t = *it;
			if (t._Myfirst._Val == &e &&
				t._Get_rest()._Myfirst._Val == &r)
			{
//#ifndef __INTELLISENSE__ // vs doesn't like this
				t._Get_rest()._Get_rest()._Myfirst._Val();
//#endif
				registered.erase(it);
				return;
			}
		}
		assert(!"Unregistering an unregistered event");
	}

	void Select(bool selected); 
	void SetName(std::string_view name);
	template<typename T>
	T* AddComponent()
	{
		assert(!GetComponent<T>());

		T* p = new T(*this);
		components.emplace_back(p);
		return p;
	}


	bool Selected() { return selected; }
	std::string_view GetName();
	template<typename T>
	T* GetComponent()
	{
		if (this == nullptr)
			return nullptr;
		for (int i = 0; i < components.size(); i++)
		{
			T* c = dynamic_cast<T*>(components[i].get());
			if (c)
				return c;
		}
		return nullptr;
	}

	inline static void Clear() { objects.clear(); }
	template<typename T>
	static std::vector<T*> GetSelected()
	{
		std::vector<T*> components;
		for (int i = 0; i < selection.size(); i++)
		{
			T* t = selection[i]->GetComponent<T>();
			if (t)
				components.push_back(t);
		}
		return components;
	}
	static Selection selection;
private:
	static std::vector<std::unique_ptr<Entity>> objects;
	Entity() = default;
protected:
	bool selected = false;
	std::string name = "Default Object";
	std::vector<std::unique_ptr<Component>> components{ };
	friend class EntityCollection;
	std::vector<EntityCollection*> containingCollections;
};

