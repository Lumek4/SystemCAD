#pragma once

#define __VECEQNEQ2DECL(type)\
bool operator==(const type& l, const  type& r);\
bool operator!=(const type& l, const  type& r);

#define __VECEQNEQ2(type)\
bool operator==(const type& l, const  type& r)\
{\
	return l.x == r.x && l.y == r.y;\
}\
bool operator!=(const type& l, const  type& r)\
{\
	return l.x != r.x || l.y != r.y;\
}