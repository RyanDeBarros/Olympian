import os

import toml
from PySide6.QtWidgets import QWidget, QSpinBox, QHeaderView, QPushButton, QFileDialog

from editor import ui, MANIFEST, PARAM_LIST
from .Common import SettingsForm, SettingsParameter
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


# TODO add ? info buttons to UI, like for the syntax used by kerning pairs
class EditTab:
	def __init__(self, editor: FontEditorWidget):
		self.editor = editor
		self.ui = self.editor.ui

		self.ui.editFontBrowse.clicked.connect(self.browse_font)

		self.ui.newKerningPair.clicked.connect(self.new_kerning_pair)
		self.ui.clearKerningTable.clicked.connect(self.clear_kerning_table)
		self.ui.kerningTable.horizontalHeader().setSectionResizeMode(QHeaderView.ResizeMode.Stretch)
		self.ui.kerningTable.setColumnWidth(3, 40)
		self.ui.kerningTable.horizontalHeader().setSectionResizeMode(3, QHeaderView.ResizeMode.Fixed)

		self.ui.applyFontFaceSettings.clicked.connect(self.apply_font_face_settings)
		self.ui.cancelFontFaceSettings.clicked.connect(self.cancel_font_face_settings)
		self.ui.resetFontFaceSettings.clicked.connect(self.reset_font_face_settings)

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

	def browse_font(self):
		pass  # TODO

	def new_kerning_pair(self):
		row = self.ui.kerningTable.rowCount()
		self.ui.kerningTable.insertRow(row)
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

	def apply_font_face_settings(self):
		pass  # TODO

	def cancel_font_face_settings(self):
		pass  # TODO

	def reset_font_face_settings(self):
		pass  # TODO

	def apply_font_atlas_settings(self):
		pass  # TODO

	def cancel_font_atlas_settings(self):
		pass  # TODO

	def reset_font_atlas_settings(self):
		pass  # TODO


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

	def default_font_filepath(self):
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
		self.font_face_form.load_dict(d['font face'])
		font_atlas = d['font atlas'][0]
		self.font_atlas_form.load_dict(font_atlas)
		if 'common buffer preset' in font_atlas:
			self.ui.defaultRadioUsePreset.click()
			self.ui.defaultFontAtlasCommonPreset.setCurrentText(PARAM_LIST.get_name(font_atlas['common buffer preset']))
			self.ui.defaultFontAtlasCommonBuffer.clear()
		else:
			self.select_manual_set()
			self.ui.defaultRadioManualSet.click()
			self.ui.defaultFontAtlasCommonPreset.setCurrentIndex(0)
			self.ui.defaultFontAtlasCommonBuffer.setText(font_atlas['common buffer'])

	def save_defaults(self):
		with open(self.default_font_filepath(), 'w') as f:
			font_atlas = self.font_atlas_form.get_dict()
			if self.ui.defaultRadioUsePreset.isChecked():
				font_atlas['common buffer preset'] = PARAM_LIST.get_value(self.ui.defaultFontAtlasCommonPreset.currentText())
			else:
				font_atlas['common buffer'] = self.ui.defaultFontAtlasCommonBuffer.text()
			d = {
				'font face': self.font_face_form.get_dict(),
				'font atlas': [font_atlas]
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
				content_condition=lambda font: 'font face' in font and 'font atlas' in font and len(font['font atlas']) > 0,
				create_default_import=self.create_default_font_import
			)

	def create_default_font_import(self, filepath):
		FileIO.create_default_import_file(filepath, self.editor.defaults_tab.get_stored_default_dict())
