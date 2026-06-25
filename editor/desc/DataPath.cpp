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

	static std::stack<DataPathVisitor*> DATA_PATH_VISITORS;

	DataPathVisitor::DataPathVisitor(DataPathVisitorFn fn)
		: _fn(fn)
	{
		DATA_PATH_VISITORS.push(this);
	}

	DataPathVisitor::~DataPathVisitor()
	{
		DATA_PATH_VISITORS.pop();
	}
	
	DataPathVisitor& DataPathVisitor::ActiveInstance()
	{
		if (DATA_PATH_VISITORS.empty())
			BreakoutError::Throw("DataPathVisitor::ActiveInstance(): data path visitor stack is empty");
		else
			return *DATA_PATH_VISITORS.top();
	}

	void* DataPathVisitor::operator()(DataPath path, std::type_index type)
	{
		return _fn(path, type);
	}
}
