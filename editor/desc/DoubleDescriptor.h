#pragma once

#include "desc/DataPath.h"

namespace oly::editor
{
	struct IDoubleDescriptor
	{
		virtual ~IDoubleDescriptor() = default;
		virtual void* VisitPath(DataPath path, std::type_index type) = 0;
		virtual void PrintPath(std::ostream& os, DataPath path) const = 0;
		virtual bool DrawFinalize() = 0;
		virtual bool QueryDirty() = 0;
	};

	template<typename Descriptor>
	struct DoubleDescriptor : public IDoubleDescriptor
	{
		Descriptor scratch;
		Descriptor disk;

		void* VisitPath(DataPath path, std::type_index type) override
		{
			return scratch.VisitPath(path, type);
		}

		void PrintPath(std::ostream& os, DataPath path) const override
		{
			scratch.PrintPath(os, path);
		}

		bool DrawFinalize() override
		{
			return scratch.DrawFinalize(DataPath());
		}

		bool QueryDirty() override
		{
			return scratch.QueryDirty(disk);
		}

		void WriteToDisk()
		{
			disk = scratch;
		}

		void LoadFromDisk()
		{
			scratch = disk;
		}
	};
}
