#include "Geometry.h"

template<typename Child>
constexpr auto get_child(const std::vector<Child*> children, Child* child)
{
	return std::find(children.begin(), children.end(), child);
}

oly::math::Transformer2D::Transformer2D(Transformer2D&& other) noexcept
	: local(std::move(other.local)), parent(other.parent), children(std::move(other.children)), _global(other._global), _dirty(other._dirty), _dirty_flush(other._dirty_flush)
{
	unparent(&other);
	for (Transformer2D* child : children)
		child->parent = this;
}

oly::math::Transformer2D::~Transformer2D()
{
	clear_children(this);
}

void oly::math::Transformer2D::post_set() const
{
	if (!_dirty)
	{
		_dirty = true;
		_dirty_flush = true;
		for (Transformer2D* child : children)
			child->post_set();
	}
}

void oly::math::Transformer2D::pre_get() const
{
	if (_dirty)
	{
		_dirty = false;
		if (parent)
		{
			parent->pre_get();
			_global = parent->_global * local.matrix();
		}
		else
			_global = local.matrix();
	}
}

bool oly::math::Transformer2D::flush() const
{
	bool was_dirty = _dirty_flush;
	_dirty_flush = false;
	return was_dirty;
}

void oly::math::attach(Transformer2D* parent, Transformer2D* child)
{
	assert(parent && child);
	if (child->parent != parent)
	{
		if (child->parent)
			child->parent->children.erase(get_child(child->parent->children, child));
		child->parent = parent;
		parent->children.push_back(child);
		child->post_set();
	}
}

void oly::math::insert(Transformer2D* parent, Transformer2D* child, size_t pos)
{
	assert(parent && child);
	if (child->parent != parent)
	{
		if (child->parent)
			child->parent->children.erase(get_child(child->parent->children, child));
		child->parent = parent;
		if (pos >= parent->children.size())
			parent->children.push_back(child);
		else
			parent->children.insert(parent->children.begin() + pos, child);
		child->post_set();
	}
}

void oly::math::clear_children(Transformer2D* parent)
{
	assert(parent);
	for (Transformer2D* child : parent->children)
	{
		child->parent = nullptr;
		child->post_set();
	}
	parent->children.clear();
}

void oly::math::unparent(Transformer2D* child)
{
	assert(child);
	if (child->parent)
		child->parent->children.erase(get_child(child->parent->children, child));
	child->parent = nullptr;
	child->post_set();
}

oly::math::Transformer3D::~Transformer3D()
{
	clear_children(this);
}

void oly::math::Transformer3D::post_set() const
{
	if (!_dirty)
	{
		_dirty = true;
		_dirty_flush = true;
		for (Transformer3D* child : children)
			child->post_set();
	}
}

void oly::math::Transformer3D::pre_get() const
{
	if (_dirty)
	{
		_dirty = false;
		if (parent)
		{
			parent->pre_get();
			_global = parent->_global * local.matrix();
		}
		else
			_global = local.matrix();
	}
}

bool oly::math::Transformer3D::flush() const
{
	bool was_dirty = _dirty_flush;
	_dirty_flush = false;
	return was_dirty;
}

void oly::math::attach(Transformer3D* parent, Transformer3D* child)
{
	assert(parent && child);
	if (child->parent != parent)
	{
		if (child->parent)
			child->parent->children.erase(get_child(child->parent->children, child));
		child->parent = parent;
		parent->children.push_back(child);
		child->post_set();
	}
}

void oly::math::insert(Transformer3D* parent, Transformer3D* child, size_t pos)
{
	assert(parent && child);
	if (child->parent != parent)
	{
		if (child->parent)
			child->parent->children.erase(get_child(child->parent->children, child));
		child->parent = parent;
		if (pos >= parent->children.size())
			parent->children.push_back(child);
		else
			parent->children.insert(parent->children.begin() + pos, child);
		child->post_set();
	}
}

void oly::math::clear_children(Transformer3D* parent)
{
	assert(parent);
	for (Transformer3D* child : parent->children)
	{
		child->parent = nullptr;
		child->post_set();
	}
	parent->children.clear();
}

void oly::math::unparent(Transformer3D* child)
{
	assert(child);
	if (child->parent)
		child->parent->children.erase(get_child(child->parent->children, child));
	child->parent = nullptr;
	child->post_set();
}
