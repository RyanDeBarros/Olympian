#pragma once

#include <vector>
#include <concepts>

namespace oly
{
	template<typename NodeType>
	class TreeNode
	{
		// TODO v5 index_in_parent
		NodeType* _parent = nullptr;
		std::vector<NodeType*> _children;

	public:
		TreeNode() = default;

		TreeNode(const TreeNode<NodeType>&) = delete;

		TreeNode(TreeNode<NodeType>&& other) noexcept
			: _parent(other._parent), _children(std::move(other._children))
		{
			other._parent = nullptr;
			if (_parent)
			{
				auto it = std::find(_parent->_children.begin(), _parent->_children.end(), other);
				*it = static_cast<NodeType*>(this);
			}
			for (TreeNode<NodeType>* child : _children)
				child->_parent = static_cast<NodeType*>(this);
		}

		virtual ~TreeNode()
		{
			detach();
			for (TreeNode* child : _children)
				child->_parent = nullptr;
		}

		TreeNode<NodeType>& operator=(TreeNode<NodeType>&& other) noexcept
		{
			if (this != &other)
			{
				if (_parent == other._parent)
					other.detach();
				else
					attach(static_cast<NodeType*>(other._parent));
				other._parent = nullptr;
				for (TreeNode<NodeType>* child : _children)
					child->_parent = nullptr;
				_children = std::move(other._children);
				for (TreeNode<NodeType>* child : _children)
					child->_parent = static_cast<NodeType*>(this);
			}
			return *this;
		}

		void attach(NodeType* parent)
		{
			if (_parent == parent)
				return;
			detach();
			_parent = parent;
			if (_parent)
				_parent->_children.push_back(static_cast<NodeType*>(this));
		}

		void detach()
		{
			if (!_parent)
				return;
			auto it = std::find(_parent->_children.begin(), _parent->_children.end(), static_cast<NodeType*>(this));
			// assume 'it' is valid
			_parent->_children.erase(it);
			_parent = nullptr;
		}

		class Iterator
		{
			const TreeNode<NodeType>* node;
			size_t pos;

			friend class TreeNode;
			Iterator(const TreeNode* node, size_t pos) : node(node), pos(pos) {}

		public:
			Iterator(const Iterator&) = default;
			Iterator(Iterator&&) = default;
			Iterator& operator=(const Iterator&) = default;
			Iterator& operator=(Iterator&&) = default;

			const NodeType* operator*() const { return static_cast<NodeType*>(node->_children[pos]); }
			NodeType* operator*() { return static_cast<NodeType*>(node->_children[pos]); }
			const NodeType* operator->() const { return static_cast<NodeType*>(node->_children[pos]); }
			NodeType* operator->() { return static_cast<NodeType*>(node->_children[pos]); }
			Iterator& operator++() { ++pos; return *this; }
			Iterator operator++(int) { Iterator it(*this); ++pos; return it; }
			bool operator==(const Iterator&) const = default;
			bool operator!=(const Iterator&) const = default;
		};

		Iterator begin() const { return Iterator(this, 0); }
		Iterator end() const { return Iterator(this, _children.size()); }
	};
}
