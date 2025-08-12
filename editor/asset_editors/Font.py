import os

import toml
from PySide6.QtWidgets import QWidget, QSpinBox, QHeaderView, QPushButton

from editor import ui, MANIFEST, PARAM_LIST
from .Common import SettingsForm, SettingsParameter
from ..util import ProjectContext


class FontEditorWidget(QWidget):
	def __init__(self, win):
		super().__init__()
		self.win = win

		self.ui = ui.asset_editors.Font.Ui_Form()
		self.ui.setupUi(self)

		self.edit_tab = EditTab(self)
		self.defaults_tab = DefaultsTab(self)
		self.import_tab = ImportTab(self)


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
		self.font_atlas_form.load_dict(d['font atlas'])
		common_buffer = d['common buffer']
		if common_buffer['use preset']:
			self.ui.defaultRadioUsePreset.click()
			self.ui.defaultFontAtlasCommonPreset.setCurrentText(PARAM_LIST.get_name(common_buffer['common buffer preset']))
			self.ui.defaultFontAtlasCommonBuffer.clear()
		else:
			self.select_manual_set()
			self.ui.defaultRadioManualSet.click()
			self.ui.defaultFontAtlasCommonPreset.setCurrentIndex(0)
			self.ui.defaultFontAtlasCommonBuffer.setText(common_buffer['common buffer'])

	def save_defaults(self):
		with open(self.default_font_filepath(), 'w') as f:
			d = {
				'font face': self.font_face_form.get_dict(),
				'font atlas': self.font_atlas_form.get_dict(),
				'common buffer': {
					'use preset': self.ui.defaultRadioUsePreset.isChecked()
				}
			}
			if self.ui.defaultRadioUsePreset.isChecked():
				d['common buffer']['common buffer preset'] = PARAM_LIST.get_value(self.ui.defaultFontAtlasCommonPreset.currentText())
			else:
				d['common buffer']['common buffer'] = self.ui.defaultFontAtlasCommonBuffer.text()
			toml.dump(d, f)


class ImportTab:
	def __init__(self, editor: FontEditorWidget):
		self.editor = editor
		self.ui = self.editor.ui
