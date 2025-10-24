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

	public:
		TreeNode() = default;

		TreeNode(const TreeNode&) = delete;

		TreeNode(TreeNode&& other) noexcept
			: _parent(other._parent), _left_sibling(other._left_sibling), _right_sibling(other._right_sibling), _children_root(other._children_root)
		{
			other._parent = nullptr;
			other._left_sibling = nullptr;
			other._right_sibling = nullptr;

			if (_left_sibling)
				_left_sibling->_right_sibling = this;
			if (_right_sibling)
				_right_sibling->_left_sibling = this;

			set_parent_of_children(this);
		}

		virtual ~TreeNode()
		{
			detach();
			set_parent_of_children(nullptr);
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

				other._parent = nullptr;
				other._left_sibling = nullptr;
				other._right_sibling = nullptr;

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

	public:
		void attach(NodeType* parent)
		{
			if (_parent == parent)
				return;

			detach();
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

			_parent = nullptr;
		}

		// TODO v5 methods to move order of or swap child nodes

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
