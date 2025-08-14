import os
from typing import Optional

import toml
from PySide6.QtWidgets import QWidget, QSpinBox, QHeaderView, QPushButton, QFileDialog, QMessageBox, QTableWidgetItem

from editor import ui, MANIFEST, PARAM_LIST
from .Common import SettingsForm, SettingsParameter, fit_with_defaults
from ..util import ProjectContext, FileIO


class FontEditorWidget(QWidget):
	def __init__(self, win):
		super().__init__()
		self.win = win

		self.ui = ui.asset_editors.Font.Ui_Form()
		self.ui.setupUi(self)

		self.edit_tab = EditTab(self)
		self.defaults_tab = DefaultsTab(self)
		self.import_tab = ImportTab(self)

		self.last_file_dialog_dir = ProjectContext.project_resource_folder()


FONT_FILE_EXTENSIONS = [
	".ttf",
	".otf"
]


class Font:
	def __init__(self, filepath):
		self.filepath = filepath
		with open(f"{filepath}.oly", 'r') as f:
			self.content = toml.load(f)
		self.font_face = self.content['font_face']
		self.font_atlases = self.content['font_atlas']
		assert len(self.font_atlases) > 0

	def dump(self):
		with open(f"{self.filepath}.oly", 'w') as f:
			toml.dump(self.content, f)


# TODO v3 add ? info buttons to UI, like for the syntax used by kerning pairs
class EditTab:
	def __init__(self, editor: FontEditorWidget):
		self.editor = editor
		self.ui = self.editor.ui
		self.font: Optional[Font] = None

		self.ui.editFontBrowse.clicked.connect(self.browse_font)

		self.ui.newKerningPair.clicked.connect(self.new_kerning_pair)
		self.ui.clearKerningTable.clicked.connect(self.clear_kerning_table)
		self.ui.kerningTable.horizontalHeader().setSectionResizeMode(QHeaderView.ResizeMode.Stretch)
		self.ui.kerningTable.setColumnWidth(3, 40)
		self.ui.kerningTable.horizontalHeader().setSectionResizeMode(3, QHeaderView.ResizeMode.Fixed)

		self.ui.applyFontFaceSettings.clicked.connect(self.apply_font_face_settings)
		self.ui.cancelFontFaceSettings.clicked.connect(self.cancel_font_face_settings)
		self.ui.resetFontFaceSettings.clicked.connect(self.reset_font_face_settings)

		self.ui.editAtlasSlotCombo.currentIndexChanged.connect(self.load_font_atlas_slot)
		self.ui.editAtlasNewSlot.clicked.connect(self.add_new_slot)
		self.ui.editAtlasDeleteSlot.clicked.connect(self.delete_slot)

		self.font_face_form = SettingsForm([
			SettingsParameter('storage', self.ui.editFontFaceStorage)
		])

		self.font_atlas_form = SettingsForm([
			SettingsParameter('font size', self.ui.editFontSize),
			SettingsParameter('storage', self.ui.editFontAtlasStorage),
			SettingsParameter('min filter', self.ui.editFontMinFilter),
			SettingsParameter('mag filter', self.ui.editFontMagFilter),
			SettingsParameter('generate mipmaps', self.ui.editFontMipmaps),
		])

		self.ui.editRadioUsePreset.clicked.connect(self.select_use_preset)
		self.ui.editRadioManualSet.clicked.connect(self.select_manual_set)
		self.select_use_preset()

		self.ui.applyAtlasSettings.clicked.connect(self.apply_font_atlas_settings)
		self.ui.cancelAtlasSettings.clicked.connect(self.cancel_font_atlas_settings)
		self.ui.resetAtlasSettings.clicked.connect(self.reset_font_atlas_settings)

		self.font_filepath_changed()

	def browse_font(self):
		filter = FileIO.file_extension_filter("Font files", FONT_FILE_EXTENSIONS)
		filepath, _ = QFileDialog.getOpenFileName(self.editor, "Select File", self.editor.last_file_dialog_dir, filter=filter)
		if filepath:
			self.editor.last_file_dialog_dir = os.path.dirname(filepath)
			self.ui.editFontFilepath.setText(filepath)
			self.font_filepath_changed()

	def font_filepath_changed(self):
		filepath = self.ui.editFontFilepath.text()
		if len(filepath) > 0:
			if not os.path.exists(f"{filepath}.oly"):
				self.editor.import_tab.create_default_font_import(filepath)

			try:
				self.font = Font(filepath)
			except Exception:
				self.font = None
				self.ui.paramsLayout.hide()
				self.ui.editAtlasDeleteSlot.setDisabled(True)
				QMessageBox.warning(self.editor, "Error", f"Could not load font {filepath} - the import file is corrupted.")
				return

			self.ui.paramsLayout.show()
			self.ui.editAtlasSlotCombo.blockSignals(True)
			self.ui.editAtlasSlotCombo.clear()
			for i in range(len(self.font.font_atlases)):
				self.ui.editAtlasSlotCombo.addItem(f"{i}")
			self.ui.editAtlasSlotCombo.blockSignals(False)

			self.ui.editAtlasDeleteSlot.setDisabled(self.ui.editAtlasSlotCombo.count() == 1)

			self.load_font_face()
			if self.ui.editAtlasSlotCombo.currentIndex() == 0:
				self.load_font_atlas_slot()
			else:
				self.ui.editAtlasSlotCombo.setCurrentIndex(0)
		else:
			self.ui.paramsLayout.hide()
			self.ui.editAtlasDeleteSlot.setDisabled(True)

	def new_kerning_pair(self):
		row = self.ui.kerningTable.rowCount()
		self.ui.kerningTable.insertRow(row)
		self.ui.kerningTable.setItem(row, 0, QTableWidgetItem(""))
		self.ui.kerningTable.setItem(row, 1, QTableWidgetItem(""))
		kerning = QSpinBox()
		kerning.setMaximum(2048)
		kerning.setMinimum(-2048)
		self.ui.kerningTable.setCellWidget(row, 2, kerning)
		delete_button = QPushButton("🗑")
		delete_button.clicked.connect(lambda checked, b=delete_button: self.on_kerning_deleted(b))
		self.ui.kerningTable.setCellWidget(row, 3, delete_button)

	def on_kerning_deleted(self, button: QPushButton):
		index = self.ui.kerningTable.indexAt(button.pos())
		assert index.isValid()
		self.ui.kerningTable.removeRow(index.row())

	def clear_kerning_table(self):
		self.ui.kerningTable.setRowCount(0)

	def select_use_preset(self):
		self.ui.editFontAtlasCommonPreset.show()
		self.ui.editFontAtlasCommonBuffer.hide()

	def select_manual_set(self):
		self.ui.editFontAtlasCommonPreset.hide()
		self.ui.editFontAtlasCommonBuffer.show()

	def get_kerning_array(self):
		return [
			{
				'pair': [
					self.ui.kerningTable.item(row, 0).text(),
					self.ui.kerningTable.item(row, 1).text()
				],
				'dist': self.ui.kerningTable.cellWidget(row, 2).value()
			} for row in range(self.ui.kerningTable.rowCount())
		]

	def load_font_face(self):
		if self.font is not None:
			self.load_dict_into_font_face(self.font.font_face)

	def load_dict_into_font_face(self, font_face):
		self.font_face_form.load_dict(font_face, self.editor.defaults_tab.get_stored_default_dict()['font_face'])
		self.ui.kerningTable.setRowCount(0)
		if 'kerning' in font_face:
			kerning = font_face['kerning']
			for i in range(len(kerning)):
				self.new_kerning_pair()
				k = kerning[i]
				self.ui.kerningTable.item(i, 0).setText(k['pair'][0])
				self.ui.kerningTable.item(i, 1).setText(k['pair'][1])
				self.ui.kerningTable.cellWidget(i, 2).setValue(k['dist'])

	def apply_font_face_settings(self):
		if self.font is not None:
			ft = self.font.font_face
			ft.update(self.font_face_form.get_dict())
			if 'kerning' in ft:
				del ft['kerning']
			ft['kerning'] = self.get_kerning_array()
			self.font.dump()

	def cancel_font_face_settings(self):
		if self.font is not None:
			self.load_font_face()

	def reset_font_face_settings(self):
		if self.font is not None:
			self.load_dict_into_font_face(self.editor.defaults_tab.get_stored_default_dict())

	def load_font_atlas_slot(self):
		if self.font is not None:
			slot = self.ui.editAtlasSlotCombo.currentIndex()
			self.load_dict_into_font_atlas(self.font.font_atlases[slot])

	def load_dict_into_font_atlas(self, font_atlas):
		fit_with_defaults(font_atlas, self.editor.defaults_tab.get_stored_default_dict()['font_atlas'][0])
		self.font_atlas_form.load_dict(font_atlas)

		if 'common buffer preset' in font_atlas:
			self.ui.editRadioUsePreset.click()
			self.ui.editFontAtlasCommonPreset.setCurrentText(PARAM_LIST.get_name(font_atlas['common buffer preset']))
			self.ui.editFontAtlasCommonBuffer.clear()
		else:
			self.ui.editRadioManualSet.click()
			self.ui.editFontAtlasCommonPreset.setCurrentIndex(0)
			self.ui.editFontAtlasCommonBuffer.setText(font_atlas['common buffer'])

	def add_new_slot(self):
		if self.font is not None:
			slot = self.ui.editAtlasSlotCombo.count()
			self.ui.editAtlasSlotCombo.addItem(f"{slot}")
			self.font.font_atlases.append(self.editor.defaults_tab.get_stored_default_dict())
			self.ui.editAtlasSlotCombo.setCurrentIndex(slot)
			self.font.dump()
			self.ui.editAtlasDeleteSlot.setDisabled(False)

	def delete_slot(self):
		if self.font is not None and len(self.font.font_atlases) > 1:
			slot = self.ui.editAtlasSlotCombo.currentIndex()
			self.font.font_atlases.pop(slot)
			self.font.dump()

			for i in range(slot + 1, self.ui.editAtlasSlotCombo.count()):
				self.ui.editAtlasSlotCombo.setItemText(i, f"{i - 1}")
			self.ui.editAtlasSlotCombo.removeItem(slot)
			if self.ui.editAtlasSlotCombo.count() == 1:
				self.ui.editAtlasDeleteSlot.setDisabled(True)

	def apply_font_atlas_settings(self):
		if self.font is not None:
			slot = self.ui.editAtlasSlotCombo.currentIndex()
			ft = self.font.font_atlases[slot]
			ft.update(self.font_atlas_form.get_dict())

			if self.ui.editRadioUsePreset.isChecked():
				if 'common buffer' in ft:
					del ft['common buffer']
				ft['common buffer preset'] = PARAM_LIST.get_value(self.ui.editFontAtlasCommonPreset.currentText())
			else:
				if 'common buffer preset' in ft:
					del ft['common buffer preset']
				ft['common buffer'] = self.ui.editFontAtlasCommonBuffer.text()

			self.font.dump()

	def cancel_font_atlas_settings(self):
		if self.font is not None:
			self.load_font_atlas_slot()

	def reset_font_atlas_settings(self):
		if self.font is not None:
			self.load_dict_into_font_atlas(self.editor.defaults_tab.get_stored_default_dict())


class DefaultsTab:
	def __init__(self, editor: FontEditorWidget):
		self.editor = editor
		self.ui = self.editor.ui

		self.ui.defaultRadioUsePreset.clicked.connect(self.select_use_preset)
		self.ui.defaultRadioManualSet.clicked.connect(self.select_manual_set)
		self.select_use_preset()

		self.font_face_form = SettingsForm([
			SettingsParameter('storage', self.ui.defaultFontFaceStorage)
		])

		self.font_atlas_form = SettingsForm([
			SettingsParameter('font size', self.ui.defaultFontSize),
			SettingsParameter('storage', self.ui.defaultFontAtlasStorage),
			SettingsParameter('min filter', self.ui.defaultFontMinFilter),
			SettingsParameter('mag filter', self.ui.defaultFontMagFilter),
			SettingsParameter('generate mipmaps', self.ui.defaultFontMipmaps),
		])

		self.ui.saveDefaultsButton.clicked.connect(self.save_defaults)
		self.ui.cancelDefaultsButton.clicked.connect(self.load_defaults)
		self.validate_project_filepaths()
		self.load_defaults()

	def select_use_preset(self):
		self.ui.defaultFontAtlasCommonPreset.show()
		self.ui.defaultFontAtlasCommonBuffer.hide()

	def select_manual_set(self):
		self.ui.defaultFontAtlasCommonPreset.hide()
		self.ui.defaultFontAtlasCommonBuffer.show()

	@staticmethod
	def default_font_filepath():
		project_id = MANIFEST.get_project_id(ProjectContext.PROJECT_FILE)
		return f'projects/{project_id}/asset_defaults/font.toml'

	def validate_project_filepaths(self):
		filepath = self.default_font_filepath()
		os.makedirs(os.path.dirname(filepath), exist_ok=True)
		if not os.path.exists(filepath):
			self.save_defaults()

	def get_stored_default_dict(self):
		with open(self.default_font_filepath(), 'r') as f:
			return toml.load(f)

	def load_defaults(self):
		d = self.get_stored_default_dict()
		self.font_face_form.load_dict(d['font_face'])
		font_atlas = d['font_atlas'][0]
		self.font_atlas_form.load_dict(font_atlas)
		if 'common buffer preset' in font_atlas:
			self.ui.defaultRadioUsePreset.click()
			self.ui.defaultFontAtlasCommonPreset.setCurrentText(PARAM_LIST.get_name(font_atlas['common buffer preset']))
			self.ui.defaultFontAtlasCommonBuffer.clear()
		else:
			self.ui.defaultRadioManualSet.click()
			self.ui.defaultFontAtlasCommonPreset.setCurrentIndex(0)
			self.ui.defaultFontAtlasCommonBuffer.setText(font_atlas['common buffer'])

	def save_defaults(self):
		with open(self.default_font_filepath(), 'w') as f:
			font_atlas = self.font_atlas_form.get_dict()
			if self.ui.defaultRadioUsePreset.isChecked():
				font_atlas['common buffer preset'] = PARAM_LIST.get_value(
					self.ui.defaultFontAtlasCommonPreset.currentText())
			else:
				font_atlas['common buffer'] = self.ui.defaultFontAtlasCommonBuffer.text()
			d = {
				'font_face': self.font_face_form.get_dict(),
				'font_atlas': [font_atlas]
			}
			toml.dump(d, f)


class ImportTab:
	def __init__(self, editor: FontEditorWidget):
		self.editor = editor
		self.ui = self.editor.ui

		self.ui.importBrowseButton.clicked.connect(self.browse_folder)
		self.ui.executeImportButton.clicked.connect(self.execute)

	def browse_folder(self):
		folder = QFileDialog.getExistingDirectory(self.editor, "Select Folder", self.editor.last_file_dialog_dir)
		if folder:
			self.editor.last_file_dialog_dir = folder
			self.ui.importFolder.setText(folder)

	def execute(self):
		root_folder = self.ui.importFolder.text()
		if len(root_folder) > 0:
			FileIO.execute_standard_import_on_folder(
				root_folder=root_folder,
				recursive=self.ui.importRecursiveSearch.isChecked(),
				clear=self.ui.importClear.isChecked(),
				clean_unused=self.ui.importCleanUnused.isChecked(),
				import_unimported=self.ui.importUnimported.isChecked(),
				extensions=FONT_FILE_EXTENSIONS,
				content_condition=lambda font: 'font_face' in font and 'font_atlas' in font and len(font['font_atlas']) > 0,
				create_default_import=self.create_default_font_import
			)

	def create_default_font_import(self, filepath):
		FileIO.create_default_import_file(filepath, self.editor.defaults_tab.get_stored_default_dict())
