#pragma once

#include <vector>
#include <concepts>

#include "core/base/Errors.h"

namespace oly
{
	template<typename NodeType>
	class TreeNode
	{
		TreeNode* _parent = nullptr;
		// TODO v5 use doubly linked list for transforms as well or no?
		TreeNode* _left_sibling = nullptr;
		TreeNode* _right_sibling = nullptr;
		TreeNode* _children_root = nullptr;
		size_t _children_size = 0;

	public:
		TreeNode() = default;

		TreeNode(const TreeNode&) = delete;

		TreeNode(TreeNode&& other) noexcept
			: _parent(other._parent), _left_sibling(other._left_sibling), _right_sibling(other._right_sibling), _children_root(other._children_root), _children_size(other._children_size)
		{
			other._parent = nullptr;
			other._left_sibling = nullptr;
			other._right_sibling = nullptr;
			other._children_root = nullptr;
			other._children_size = 0;

			if (_left_sibling)
				_left_sibling->_right_sibling = this;
			if (_right_sibling)
				_right_sibling->_left_sibling = this;

			set_parent_of_children(this);
		}

		virtual ~TreeNode()
		{
			detach();
			clear_children();
		}

		TreeNode& operator=(const TreeNode&) = delete;

		TreeNode& operator=(TreeNode&& other) noexcept
		{
			if (this != &other)
			{
				detach();
				set_parent_of_children(nullptr);

				_parent = other._parent;
				_left_sibling = other._left_sibling;
				_right_sibling = other._right_sibling;
				_children_root = other._children_root;
				_children_size = other._children_size;

				other._parent = nullptr;
				other._left_sibling = nullptr;
				other._right_sibling = nullptr;
				other._children_root = nullptr;
				other._children_size = 0;

				if (_left_sibling)
					_left_sibling->_right_sibling = this;
				if (_right_sibling)
					_right_sibling->_left_sibling = this;

				set_parent_of_children(this);
			}
			return *this;
		}

	private:
		void set_parent_of_children(TreeNode* parent)
		{
			if (_children_root)
			{
				_children_root->_parent = parent;
				TreeNode* sibling = _children_root->_right_sibling;
				while (sibling != _children_root)
				{
					sibling->_parent = parent;
					sibling = sibling->_right_sibling;
				}
			}
		}

	protected:
		virtual void on_attach(NodeType* old_parent, NodeType* new_parent) {}

	private:
		void on_attach_call(TreeNode* old_parent)
		{
			on_attach(static_cast<NodeType*>(old_parent), static_cast<NodeType*>(_parent));
		}

	public:
		void attach(NodeType* parent)
		{
			if (_parent == parent)
				return;

			detach();
			TreeNode* old_parent = _parent;
			_parent = parent;
			if (!_parent)
				return;

			if (_parent->_children_root)
			{
				TreeNode* last = _parent->_children_root->_left_sibling;
				if (!last)
					last = _parent->_children_root;

				last->_right_sibling = this;
				_left_sibling = last;
				_parent->_children_root->_left_sibling = this;
				_right_sibling = _parent->_children_root;
			}
			else
				_parent->_children_root = this;

			++_parent->_children_size;
			on_attach_call(old_parent);
		}

		void attach_right(NodeType& left_sibling)
		{
			if (this == &left_sibling)
				throw Error(ErrorCode::CIRCULAR_REFERENCE);

			if (_parent == left_sibling._parent)
			{
				swap_with_sibling(*left_sibling._right_sibling);
				return;
			}

			detach();
			TreeNode* old_parent = _parent;
			_parent = left_sibling._parent;
			if (!_parent)
				return;

			TreeNode* right_sibling = left_sibling._right_sibling;
			if (!right_sibling)
				right_sibling = &left_sibling;

			left_sibling._right_sibling = this;
			_left_sibling = &left_sibling;
			right_sibling->_left_sibling = this;
			_right_sibling = right_sibling;

			++_parent->_children_size;
			on_attach_call(old_parent);
		}

		void attach_left(NodeType& right_sibling)
		{
			if (this == &right_sibling)
				throw Error(ErrorCode::CIRCULAR_REFERENCE);

			if (_parent == right_sibling._parent)
			{
				swap_with_sibling(*right_sibling._left_sibling);
				return;
			}

			detach();
			TreeNode* old_parent = _parent;
			_parent = right_sibling._parent;
			if (!_parent)
				return;

			TreeNode* left_sibling = right_sibling._left_sibling;
			if (!left_sibling)
				left_sibling = &right_sibling;

			right_sibling._left_sibling = this;
			_right_sibling = &right_sibling;
			left_sibling->_right_sibling = this;
			_left_sibling = left_sibling;

			if (_parent->_children_root == &right_sibling)
				_parent->_children_root = this;

			++_parent->_children_size;
			on_attach_call(old_parent);
		}

		void detach()
		{
			if (!_parent)
				return;

			if (_parent->_children_root == this)
				_parent->_children_root = _right_sibling;

			if (_left_sibling)
			{
				if (_left_sibling == _right_sibling)
				{
					_left_sibling->_right_sibling = nullptr;
					_right_sibling->_left_sibling = nullptr;
				}
				else
				{
					_left_sibling->_right_sibling = _right_sibling;
					_right_sibling->_left_sibling = _left_sibling;
				}

				_left_sibling = nullptr;
				_right_sibling = nullptr;
			}

			--_parent->_children_size;

			TreeNode* old_parent = _parent;
			_parent = nullptr;
			on_attach_call(old_parent);
		}

		void clear_children()
		{
			if (!_children_root)
				return;

			TreeNode* sibling = _children_root->_right_sibling;

			_children_root->_parent = nullptr;
			_children_root->on_attach_call(this);
			_children_root->_left_sibling = nullptr;
			_children_root->_right_sibling = nullptr;

			while (sibling && sibling != _children_root)
			{
				TreeNode* old = sibling;
				sibling = sibling->_right_sibling;

				old->_parent = nullptr;
				old->on_attach_call(this);
				old->_left_sibling = nullptr;
				old->_right_sibling = nullptr;
			}
			_children_root = nullptr;
			_children_size = 0;
		}

		size_t children_size() const
		{
			return _children_size;
		}

		void swap_with_sibling(TreeNode& sibling)
		{
			if (this == &sibling || !_parent || _parent != sibling._parent)
				return;

			if (_parent->_children_root == this)
				_parent->_children_root = &sibling;
			else if (_parent->_children_root == &sibling)
				_parent->_children_root = this;

			if (&sibling == _left_sibling)
			{
				if (&sibling != _right_sibling)
				{
					// We are not the only children of parent
					sibling._right_sibling = _right_sibling;
					_left_sibling = sibling._left_sibling;
					sibling._left_sibling = this;
					_right_sibling = &sibling;
				}
			}
			else if (&sibling == _right_sibling)
			{
				sibling._left_sibling = _left_sibling;
				_right_sibling = sibling._right_sibling;
				sibling._right_sibling = this;
				_left_sibling = &sibling;
			}
			else
			{
				TreeNode* my_left = _left_sibling;
				TreeNode* my_right = _right_sibling;
				TreeNode* their_left = sibling._left_sibling;
				TreeNode* their_right = sibling._right_sibling;

				_left_sibling = their_left;
				_right_sibling = their_right;
				sibling._left_sibling = my_left;
				sibling._right_sibling = my_right;
			}
		}

		const NodeType* get_parent() const
		{
			return static_cast<const NodeType*>(_parent);
		}

		NodeType* get_parent()
		{
			return static_cast<NodeType*>(_parent);
		}

		const NodeType* get_left_sibling() const
		{
			return static_cast<const NodeType*>(_left_sibling);
		}

		NodeType* get_left_sibling()
		{
			return static_cast<NodeType*>(_left_sibling);
		}

		const NodeType* get_right_sibling() const
		{
			return static_cast<const NodeType*>(_right_sibling);
		}

		NodeType* get_right_sibling()
		{
			return static_cast<NodeType*>(_right_sibling);
		}

		size_t get_position_in_parent() const
		{
			if (!_parent)
				throw Error(ErrorCode::NULL_POINTER);

			size_t index = 0;
			TreeNode* distance = this;
			while (distance != _parent->_children_root)
			{
				++index;
				distance = distance->_left_sibling;
			}
			return index;
		}

		void set_position_in_parent(size_t index)
		{
			if (!_parent)
				throw Error(ErrorCode::NULL_POINTER);

			if (index >= _parent->_children_size)
				throw Error(ErrorCode::INDEX_OUT_OF_RANGE);

			size_t current_index = get_position_in_parent();
			if (current_index == index)
				return;

			if (current_index < index)
			{
				while (current_index < index)
				{
					++current_index;
					swap_with_sibling(_right_sibling);
				}
			}
			else
			{
				while (current_index > index)
				{
					++index;
					swap_with_sibling(_left_sibling);
				}
			}
		}

		class Iterator
		{
			const TreeNode* node;
			TreeNode* child;

			friend class TreeNode;
			friend class ConstIterator;
			Iterator(const TreeNode* node, TreeNode* child) : node(node), child(child) {}

		public:
			Iterator(const Iterator&) = default;
			Iterator(Iterator&&) = default;
			Iterator& operator=(const Iterator&) = default;
			Iterator& operator=(Iterator&&) = default;

			const NodeType& operator*() const
			{
				if (!child)
					throw Error(ErrorCode::INVALID_ITERATOR);
				return *static_cast<const NodeType*>(child);
			}

			NodeType& operator*()
			{
				if (!child)
					throw Error(ErrorCode::INVALID_ITERATOR);
				return *static_cast<NodeType*>(child);
			}
			
			const NodeType* operator->() const
			{
				if (!child)
					throw Error(ErrorCode::INVALID_ITERATOR);
				return static_cast<const NodeType*>(child);
			}
			
			NodeType* operator->()
			{
				if (!child)
					throw Error(ErrorCode::INVALID_ITERATOR);
				return static_cast<NodeType*>(child);
			}

			Iterator& operator++()
			{
				if (!child)
					throw Error(ErrorCode::INVALID_ITERATOR);

				TreeNode* next = child->_right_sibling;
				if (next != node->_children_root)
					child = next;
				else
					child = nullptr;

				return *this;
			}

			Iterator operator++(int) { Iterator it(*this); ++*this; return it; }

			bool operator==(const Iterator&) const = default;
			bool operator!=(const Iterator&) const = default;
		};

		class ConstIterator
		{
			const TreeNode* node;
			const TreeNode* child;

			friend class TreeNode;
			ConstIterator(const TreeNode* node, const TreeNode* child) : node(node), child(child) {}

		public:
			ConstIterator(const Iterator& it) : node(it.node), child(it.child) {}
			ConstIterator(Iterator&& it) noexcept : node(it.node), child(it.child) {}

			ConstIterator(const ConstIterator&) = default;
			ConstIterator(ConstIterator&&) = default;
			ConstIterator& operator=(const ConstIterator&) = default;
			ConstIterator& operator=(ConstIterator&&) = default;

			const NodeType& operator*() const
			{
				if (!child)
					throw Error(ErrorCode::INVALID_ITERATOR);
				return *static_cast<const NodeType*>(child);
			}

			const NodeType* operator->() const
			{
				if (!child)
					throw Error(ErrorCode::INVALID_ITERATOR);
				return static_cast<const NodeType*>(child);
			}

			ConstIterator& operator++()
			{
				if (!child)
					throw Error(ErrorCode::INVALID_ITERATOR);

				const TreeNode* next = child->_right_sibling;
				if (next != node->_children_root)
					child = next;
				else
					child = nullptr;

				return *this;
			}

			ConstIterator operator++(int) { ConstIterator it(*this); ++*this; return it; }

			bool operator==(const ConstIterator&) const = default;
			bool operator!=(const ConstIterator&) const = default;
		};

		Iterator begin() { return Iterator(this, _children_root); }
		Iterator end() { return Iterator(this, nullptr); }
		ConstIterator cbegin() const { return ConstIterator(this, _children_root); }
		ConstIterator cend() const { return ConstIterator(this, nullptr); }
		ConstIterator begin() const { return ConstIterator(this, _children_root); }
		ConstIterator end() const { return ConstIterator(this, nullptr); }
	};
}
