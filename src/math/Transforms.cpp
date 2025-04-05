#include "Transforms.h"

oly::Transformer2D::Transformer2D(Transformer2D&& other) noexcept
	: local(other.local), modifier(std::move(other.modifier)), parent(other.parent), children(std::move(other.children)), _global(other._global), _dirty(other._dirty), _dirty_flush(other._dirty_flush)
{
	other.unparent();
	for (Transformer2D* child : children)
		child->parent = this;
}

oly::Transformer2D::~Transformer2D()
{
	unparent();
	clear_children();
}

oly::Transformer2D& oly::Transformer2D::operator=(Transformer2D&& other) noexcept
{
	if (this != &other)
	{
		unparent();
		clear_children();
		local = other.local;
		modifier = std::move(other.modifier);
		parent = other.parent;
		children = std::move(other.children);
		_global = other._global;
		_dirty = other._dirty;
		_dirty_flush = other._dirty_flush;
	}
	return *this;
}

void oly::Transformer2D::post_set() const
{
	if (!_dirty)
	{
		_dirty = true;
		_dirty_flush = true;
		for (Transformer2D* child : children)
			child->post_set();
	}
}

void oly::Transformer2D::pre_get() const
{
	if (_dirty)
	{
		_dirty = false;
		if (parent)
		{
			parent->pre_get();
			_global = parent->global() * local.matrix();
		}
		else
			_global = local.matrix();
	}
}

bool oly::Transformer2D::flush() const
{
	bool was_dirty = _dirty_flush;
	_dirty_flush = false;
	return was_dirty;
}

const oly::Transformer2D* oly::Transformer2D::top_level_parent() const
{
	const Transformer2D* top = this;
	while (top->parent)
		top = top->parent;
	return top;
}

oly::Transformer2D* oly::Transformer2D::top_level_parent()
{
	Transformer2D* top = this;
	while (top->parent)
		top = top->parent;
	return top;
}

void oly::Transformer2D::attach_parent(Transformer2D* new_parent)
{
	if (!new_parent)
		unparent();
	else if (parent != new_parent)
	{
		if (parent)
			parent->children.erase(this);
		parent = new_parent;
		parent->children.insert(this);
		post_set();
	}
}

void oly::Transformer2D::insert_chain(Transformer2D* parent_chain)
{
	if (!parent_chain)
		return;
	if (parent)
	{
		parent->children.erase(this);
		Transformer2D* chain_top = parent_chain->top_level_parent();
		parent->children.insert(chain_top);
		chain_top->parent = parent;
	}
	parent = parent_chain;
	parent->children.insert(this);
	post_set();
}

void oly::Transformer2D::unparent()
{
	if (parent)
		parent->children.erase(this);
	parent = nullptr;
	post_set();
}

void oly::Transformer2D::clear_children()
{
	for (Transformer2D* child : children)
	{
		child->parent = nullptr;
		child->post_set();
	}
	children.clear();
}

void oly::Transformer2D::pop_from_chain()
{
	if (parent)
	{
		for (Transformer2D* child : children)
			parent->children.insert(child);
	}
	for (Transformer2D* child : children)
	{
		child->parent = parent;
		child->post_set();
	}
	children.clear();
}

oly::Transformer3D::Transformer3D(Transformer3D&& other) noexcept
	: local(other.local), modifier(std::move(other.modifier)), parent(other.parent), children(std::move(other.children)), _global(other._global), _dirty(other._dirty), _dirty_flush(other._dirty_flush)
{
	other.unparent();
	for (Transformer3D* child : children)
		child->parent = this;
}

oly::Transformer3D::~Transformer3D()
{
	unparent();
	clear_children();
}

oly::Transformer3D& oly::Transformer3D::operator=(Transformer3D&& other) noexcept
{
	if (this != &other)
	{
		unparent();
		clear_children();
		local = other.local;
		modifier = std::move(other.modifier);
		parent = other.parent;
		children = std::move(other.children);
		_global = other._global;
		_dirty = other._dirty;
		_dirty_flush = other._dirty_flush;
	}
	return *this;
}

void oly::Transformer3D::post_set() const
{
	if (!_dirty)
	{
		_dirty = true;
		_dirty_flush = true;
		for (Transformer3D* child : children)
			child->post_set();
	}
}

void oly::Transformer3D::pre_get() const
{
	if (_dirty)
	{
		_dirty = false;
		if (parent)
		{
			parent->pre_get();
			_global = parent->global() * local.matrix();
		}
		else
			_global = local.matrix();
	}
}

bool oly::Transformer3D::flush() const
{
	bool was_dirty = _dirty_flush;
	_dirty_flush = false;
	return was_dirty;
}

const oly::Transformer3D* oly::Transformer3D::top_level_parent() const
{
	const Transformer3D* top = this;
	while (top->parent)
		top = top->parent;
	return top;
}

oly::Transformer3D* oly::Transformer3D::top_level_parent()
{
	Transformer3D* top = this;
	while (top->parent)
		top = top->parent;
	return top;
}

void oly::Transformer3D::attach_parent(Transformer3D* new_parent)
{
	if (!new_parent)
		unparent();
	else if (parent != new_parent)
	{
		if (parent)
			parent->children.erase(this);
		parent = new_parent;
		parent->children.insert(this);
		post_set();
	}
}

void oly::Transformer3D::insert_chain(Transformer3D* parent_chain)
{
	if (!parent_chain)
		return;
	if (parent)
	{
		parent->children.erase(this);
		Transformer3D* chain_top = parent_chain->top_level_parent();
		parent->children.insert(chain_top);
		chain_top->parent = parent;
	}
	parent = parent_chain;
	parent->children.insert(this);
	post_set();
}

void oly::Transformer3D::unparent()
{
	if (parent)
		parent->children.erase(this);
	parent = nullptr;
	post_set();
}

void oly::Transformer3D::clear_children()
{
	for (Transformer3D* child : children)
	{
		child->parent = nullptr;
		child->post_set();
	}
	children.clear();
}

void oly::Transformer3D::pop_from_chain()
{
	if (parent)
	{
		for (Transformer3D* child : children)
			parent->children.insert(child);
	}
	for (Transformer3D* child : children)
	{
		child->parent = parent;
		child->post_set();
	}
	children.clear();
}
