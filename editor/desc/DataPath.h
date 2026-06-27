#pragma once

#include <functional>
#include <ostream>
#include <span>
#include <typeindex>
#include <vector>

namespace oly::editor
{
	struct DataPathStep
	{
		int v;

		constexpr DataPathStep(int v) : v(v) {}
		
		constexpr bool operator==(const DataPathStep& o) const { return v == o.v; }
		constexpr bool operator==(int o) const { return v == o; }
	};

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

	// TODO v9.1 use ActiveDocument instead in separate file
	struct DataPathVisitor
	{
		std::function<void* (DataPath, std::type_index)> visit_path;
		std::function<bool()> is_dirty;
		std::function<void()> query_dirty;

		DataPathVisitor();
		DataPathVisitor(const DataPathVisitor&) = delete;
		DataPathVisitor(DataPathVisitor&&) noexcept;
		~DataPathVisitor();
		DataPathVisitor& operator=(DataPathVisitor&&) = delete;

		static DataPathVisitor& ActiveInstance();
	};
}
