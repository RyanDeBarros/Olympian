#pragma once

#include "desc/FieldSetAction.h"

#include <imp/box.hpp>

namespace oly::editor
{
	struct IDoubleDescriptor
	{
		virtual ~IDoubleDescriptor() = default;
		virtual void* resolve(imtk::datapath_view path, std::type_index type) = 0;
		virtual void describe(std::ostream& os, imtk::datapath_view path) const = 0;
		virtual bool query_dirty() = 0;
		virtual imp::box CopyScratch() const = 0;
		virtual std::unique_ptr<UndoAction> ScratchUndoAction(imp::box original) const = 0;
		virtual bool ScratchUndoActionQuery(imp::box original, std::unique_ptr<UndoAction>& action) const = 0;
	};

	template<typename Descriptor>
	struct DoubleDescriptor : public IDoubleDescriptor
	{
		Descriptor scratch;
		Descriptor disk;

		DoubleDescriptor() = default;
		DoubleDescriptor(Descriptor scratch, Descriptor disk) : scratch(std::move(scratch)), disk(std::move(disk)) {}

		void* resolve(imtk::datapath_view path, std::type_index type) override
		{
			return scratch.resolve(path, type);
		}

		void describe(std::ostream& os, imtk::datapath_view path) const override
		{
			scratch.describe(os, path);
		}

		bool query_dirty() override
		{
			return scratch.query_dirty(disk);
		}

		imp::box CopyScratch() const
		{
			return imp::make_box<Descriptor>(imtk::desc::clone_data(scratch));
		}

		std::unique_ptr<UndoAction> ScratchUndoAction(imp::box original) const override
		{
			if (auto og = original.consume<Descriptor>())
				return std::make_unique<DescriptorSetAction<Descriptor, void>>(imtk::datapath_view(), std::move(*og), imtk::desc::clone_data(scratch));
			else
				return nullptr;
		}

		bool ScratchUndoActionQuery(imp::box original, std::unique_ptr<UndoAction>& action) const override
		{
			if (auto og = original.as<Descriptor>())
			{
				if (scratch.query_dirty(*og))
				{
					if (auto og = original.consume<Descriptor>())
						action = std::make_unique<DescriptorSetAction<Descriptor, void>>(imtk::datapath_view(), std::move(*og), imtk::desc::clone_data(scratch));
					else
						action = nullptr;

					return true;
				}
			}

			return false;
		}

		void WriteToDisk()
		{
			disk.copy_data(scratch);
		}

		void LoadFromDisk()
		{
			scratch.copy_data(disk);
		}

		void ResetScratch()
		{
			scratch = Descriptor();
		}
	};
}
