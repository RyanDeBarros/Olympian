#include "DataPath.h"

#include <stack>

namespace oly::editor
{
	DataPathSource::DataPathSource(DataPath path)
		: _path(path._path.begin(), path._path.end())
	{
	}

	DataPathSource DataPathSource::operator/(DataPathStep step) const
	{
		DataPathSource p = *this;
		p._path.push_back(step);
		return p;
	}

	DataPathSource& DataPathSource::operator/=(DataPathStep step)
	{
		_path.push_back(step);
		return *this;
	}

	DataPath::DataPath(const DataPathSource& source)
		: _path(source._path)
	{
	}

	DataPath& DataPath::operator=(const DataPathSource& source)
	{
		_path = source._path;
		return *this;
	}

	DataPathSource DataPath::Copy() const
	{
		DataPathSource source;
		source._path = std::vector(_path.begin(), _path.end());
		return source;
	}

	DataPathSource DataPath::operator/(DataPathStep step) const
	{
		return std::move(Copy() /= step);
	}

	bool DataPath::Empty() const
	{
		return _path.empty();
	}

	DataPathStep DataPath::Step() const
	{
		return _path.front();
	}
	
	DataPath DataPath::Next() const
	{
		DataPath next;
		next._path = _path.subspan<1>();
		return next;
	}

	std::ostream& operator<<(std::ostream& os, DataPath path)
	{
		os << "DataPath(";

		for (size_t i = 0; i < path._path.size(); ++i)
		{
			os << path._path[i].v;

			if (i + 1 < path._path.size())
				os << ", ";
		}

		return os << ")";
	}

	DataPathSource DataPathLink::Node::ComputePath() const
	{
		auto path = parent ? parent->ComputePath() : DataPathSource();
		if (step)
			path /= *step;
		return path;
	}

	DataPathLink::DataPathLink()
		: node(std::make_unique<Node>())
	{
	}

	DataPathLink::DataPathLink(DataPathLink& parent, DataPathStep step)
		: node(std::make_unique<Node>())
	{
		node->parent = parent.node;
		node->step = step;
	}

	std::optional<DataPathStep> DataPathLink::Step() const
	{
		return node->step;
	}

	void DataPathLink::SetStep(DataPathStep step)
	{
		node->step = step;
	}

	DataPathLink DataPathLink::Share() const
	{
		DataPathLink clone;
		clone.node = node;
		return clone;
	}

	DataPathSource DataPathLink::ComputePath() const
	{
		return node->ComputePath();
	}
}
