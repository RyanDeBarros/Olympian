from PySide6.QtWidgets import QWidget, QLineEdit, QSpinBox, QHeaderView, QPushButton

from editor import ui
from .Common import SettingsForm, SettingsParameter


class FontEditorWidget(QWidget):
	def __init__(self, win):
		super().__init__()
		self.win = win

		self.ui = ui.asset_editors.Font.Ui_Form()
		self.ui.setupUi(self)

		self.edit_tab = EditTab(self)


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
		self.ui.editFontAtlasCommonPreset.setDisabled(False)
		self.ui.editFontAtlasCommonBuffer.setDisabled(True)

	def select_manual_set(self):
		self.ui.editFontAtlasCommonPreset.setDisabled(True)
		self.ui.editFontAtlasCommonBuffer.setDisabled(False)

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
