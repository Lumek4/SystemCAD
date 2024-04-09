#pragma once
#include<type_traits>
#include<vector>

class Entity;
class CatalogueBase;

class Component
{
public:
	~Component();
	Entity& owner;
protected:
	Component(Entity& owner, CatalogueBase& catalogue);
	CatalogueBase& catalogue;
	virtual const char* Name() const = 0;
	//virtual void ListGui() {}
private:
};

#define ComponentConstructorArgs(className) owner, Catalogue<className>::Instance

#define RequireComponent(componentType)\
*([&]() { auto val = owner.GetComponent<componentType>();\
if (val)return val;\
else assert(0 && #componentType " is required, but not attached"); }())

