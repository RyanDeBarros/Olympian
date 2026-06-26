#pragma once

#include <functional>
#include <ostream>
#include <span>
#include <typeindex>
#include <vector>

namespace oly::editor
{
	using DataPathStep = int;

	class DataPath;

	class DataPathSource
	{
		friend class DataPath;
		std::vector<DataPathStep> _path;

	public:
		DataPathSource operator/(DataPathStep step) const;
		DataPathSource& operator/=(DataPathStep step);
	};

	class DataPath
	{
		std::span<const DataPathStep> _path;

	public:
		DataPath() = default;
		DataPath(const DataPathSource& source);
		DataPath& operator=(const DataPathSource& source);

		DataPathSource Copy() const;
		DataPathSource operator/(DataPathStep step) const;

		bool Empty() const;
		DataPathStep Step() const;
		DataPath Next() const;

		friend std::ostream& operator<<(std::ostream& os, DataPath path);
	};

	using DataPathVisitorFn = std::function<void*(DataPath, std::type_index)>;

	class DataPathVisitor
	{
		DataPathVisitorFn _fn;

	public:
		DataPathVisitor(DataPathVisitorFn fn);
		DataPathVisitor(const DataPathVisitor&) = delete;
		DataPathVisitor(DataPathVisitor&&) = delete;
		~DataPathVisitor();

		static DataPathVisitor& ActiveInstance();

		void* operator()(DataPath path, std::type_index type);
	};
}
