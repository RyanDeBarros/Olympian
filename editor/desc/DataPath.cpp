#include "DataPath.h"

#include "core/Errors.h"

#include <stack>

namespace oly::editor
{
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

	DataPathVisitor* ACTIVE_DATA_PATH_VISITOR = nullptr;

	DataPathVisitor::DataPathVisitor()
	{
		if (ACTIVE_DATA_PATH_VISITOR)
			BreakoutError::Throw("DataPathVisitor::DataPathVisitor(): active data path visitor already exists");

		ACTIVE_DATA_PATH_VISITOR = this;
	}

	DataPathVisitor::DataPathVisitor(DataPathVisitor&& o) noexcept
		: visit_path(std::move(o.visit_path)), query_dirty(std::move(o.query_dirty))
	{
		if (ACTIVE_DATA_PATH_VISITOR == &o)
			ACTIVE_DATA_PATH_VISITOR = this;
	}

	DataPathVisitor::~DataPathVisitor()
	{
		if (ACTIVE_DATA_PATH_VISITOR == this)
			ACTIVE_DATA_PATH_VISITOR = nullptr;
	}
	
	DataPathVisitor& DataPathVisitor::ActiveInstance()
	{
		if (ACTIVE_DATA_PATH_VISITOR)
			return *ACTIVE_DATA_PATH_VISITOR;
		else
			BreakoutError::Throw("DataPathVisitor::ActiveInstance(): no active data path visitor");
	}
}
