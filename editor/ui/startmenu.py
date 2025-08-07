# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'startmenu.ui'
##
## Created by: Qt User Interface Compiler version 6.9.1
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from PySide6.QtCore import (QCoreApplication, QDate, QDateTime, QLocale,
    QMetaObject, QObject, QPoint, QRect,
    QSize, QTime, QUrl, Qt)
from PySide6.QtGui import (QBrush, QColor, QConicalGradient, QCursor,
    QFont, QFontDatabase, QGradient, QIcon,
    QImage, QKeySequence, QLinearGradient, QPainter,
    QPalette, QPixmap, QRadialGradient, QTransform)
from PySide6.QtWidgets import (QApplication, QCheckBox, QComboBox, QFormLayout,
    QHBoxLayout, QLabel, QLineEdit, QPushButton,
    QSizePolicy, QSpacerItem, QTabWidget, QVBoxLayout,
    QWidget)

class Ui_Form(object):
    def setupUi(self, Form):
        if not Form.objectName():
            Form.setObjectName(u"Form")
        Form.resize(500, 300)
        self.verticalLayout = QVBoxLayout(Form)
        self.verticalLayout.setObjectName(u"verticalLayout")
        self.tabWidget = QTabWidget(Form)
        self.tabWidget.setObjectName(u"tabWidget")
        self.newTab = QWidget()
        self.newTab.setObjectName(u"newTab")
        self.formLayout = QFormLayout(self.newTab)
        self.formLayout.setObjectName(u"formLayout")
        self.label = QLabel(self.newTab)
        self.label.setObjectName(u"label")

        self.formLayout.setWidget(0, QFormLayout.ItemRole.LabelRole, self.label)

        self.label_2 = QLabel(self.newTab)
        self.label_2.setObjectName(u"label_2")

        self.formLayout.setWidget(2, QFormLayout.ItemRole.LabelRole, self.label_2)

        self.horizontalLayout_2 = QHBoxLayout()
        self.horizontalLayout_2.setObjectName(u"horizontalLayout_2")
        self.folderName = QLineEdit(self.newTab)
        self.folderName.setObjectName(u"folderName")
        self.folderName.setMinimumSize(QSize(300, 0))
        self.folderName.setReadOnly(True)

        self.horizontalLayout_2.addWidget(self.folderName)

        self.newBrowseButton = QPushButton(self.newTab)
        self.newBrowseButton.setObjectName(u"newBrowseButton")
        self.newBrowseButton.setMinimumSize(QSize(0, 0))

        self.horizontalLayout_2.addWidget(self.newBrowseButton)


        self.formLayout.setLayout(2, QFormLayout.ItemRole.FieldRole, self.horizontalLayout_2)

        self.label_3 = QLabel(self.newTab)
        self.label_3.setObjectName(u"label_3")

        self.formLayout.setWidget(4, QFormLayout.ItemRole.LabelRole, self.label_3)

        self.createProjectFolderButton = QCheckBox(self.newTab)
        self.createProjectFolderButton.setObjectName(u"createProjectFolderButton")

        self.formLayout.setWidget(4, QFormLayout.ItemRole.FieldRole, self.createProjectFolderButton)

        self.label_5 = QLabel(self.newTab)
        self.label_5.setObjectName(u"label_5")

        self.formLayout.setWidget(6, QFormLayout.ItemRole.LabelRole, self.label_5)

        self.srcFolder = QLineEdit(self.newTab)
        self.srcFolder.setObjectName(u"srcFolder")
        self.srcFolder.setReadOnly(True)

        self.formLayout.setWidget(6, QFormLayout.ItemRole.FieldRole, self.srcFolder)

        self.label_6 = QLabel(self.newTab)
        self.label_6.setObjectName(u"label_6")

        self.formLayout.setWidget(8, QFormLayout.ItemRole.LabelRole, self.label_6)

        self.resFolder = QLineEdit(self.newTab)
        self.resFolder.setObjectName(u"resFolder")
        self.resFolder.setReadOnly(True)

        self.formLayout.setWidget(8, QFormLayout.ItemRole.FieldRole, self.resFolder)

        self.label_7 = QLabel(self.newTab)
        self.label_7.setObjectName(u"label_7")

        self.formLayout.setWidget(10, QFormLayout.ItemRole.LabelRole, self.label_7)

        self.genFolder = QLineEdit(self.newTab)
        self.genFolder.setObjectName(u"genFolder")
        self.genFolder.setReadOnly(True)

        self.formLayout.setWidget(10, QFormLayout.ItemRole.FieldRole, self.genFolder)

        self.label_8 = QLabel(self.newTab)
        self.label_8.setObjectName(u"label_8")

        self.formLayout.setWidget(12, QFormLayout.ItemRole.LabelRole, self.label_8)

        self.projectFilepath = QLineEdit(self.newTab)
        self.projectFilepath.setObjectName(u"projectFilepath")
        self.projectFilepath.setReadOnly(True)

        self.formLayout.setWidget(12, QFormLayout.ItemRole.FieldRole, self.projectFilepath)

        self.horizontalLayout = QHBoxLayout()
        self.horizontalLayout.setObjectName(u"horizontalLayout")
        self.horizontalSpacer = QSpacerItem(40, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum)

        self.horizontalLayout.addItem(self.horizontalSpacer)

        self.errorCreateProject = QLabel(self.newTab)
        self.errorCreateProject.setObjectName(u"errorCreateProject")
        self.errorCreateProject.setStyleSheet(u"color: red;")
        self.errorCreateProject.setTextFormat(Qt.AutoText)

        self.horizontalLayout.addWidget(self.errorCreateProject)

        self.createProjectButton = QPushButton(self.newTab)
        self.createProjectButton.setObjectName(u"createProjectButton")

        self.horizontalLayout.addWidget(self.createProjectButton)


        self.formLayout.setLayout(14, QFormLayout.ItemRole.FieldRole, self.horizontalLayout)

        self.horizontalLayout_5 = QHBoxLayout()
        self.horizontalLayout_5.setObjectName(u"horizontalLayout_5")
        self.projectName = QLineEdit(self.newTab)
        self.projectName.setObjectName(u"projectName")

        self.horizontalLayout_5.addWidget(self.projectName)

        self.label_9 = QLabel(self.newTab)
        self.label_9.setObjectName(u"label_9")
        self.label_9.setMinimumSize(QSize(110, 0))
        self.label_9.setAlignment(Qt.AlignRight|Qt.AlignTrailing|Qt.AlignVCenter)

        self.horizontalLayout_5.addWidget(self.label_9)


        self.formLayout.setLayout(0, QFormLayout.ItemRole.FieldRole, self.horizontalLayout_5)

        self.tabWidget.addTab(self.newTab, "")
        self.openTab = QWidget()
        self.openTab.setObjectName(u"openTab")
        self.formLayout_2 = QFormLayout(self.openTab)
        self.formLayout_2.setObjectName(u"formLayout_2")
        self.label_4 = QLabel(self.openTab)
        self.label_4.setObjectName(u"label_4")

        self.formLayout_2.setWidget(0, QFormLayout.ItemRole.LabelRole, self.label_4)

        self.horizontalLayout_3 = QHBoxLayout()
        self.horizontalLayout_3.setObjectName(u"horizontalLayout_3")
        self.openProject = QLineEdit(self.openTab)
        self.openProject.setObjectName(u"openProject")
        self.openProject.setReadOnly(True)

        self.horizontalLayout_3.addWidget(self.openProject)

        self.openBrowseButton = QPushButton(self.openTab)
        self.openBrowseButton.setObjectName(u"openBrowseButton")

        self.horizontalLayout_3.addWidget(self.openBrowseButton)


        self.formLayout_2.setLayout(0, QFormLayout.ItemRole.FieldRole, self.horizontalLayout_3)

        self.horizontalLayout_4 = QHBoxLayout()
        self.horizontalLayout_4.setObjectName(u"horizontalLayout_4")
        self.horizontalSpacer_2 = QSpacerItem(40, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum)

        self.horizontalLayout_4.addItem(self.horizontalSpacer_2)

        self.openProjectButton = QPushButton(self.openTab)
        self.openProjectButton.setObjectName(u"openProjectButton")

        self.horizontalLayout_4.addWidget(self.openProjectButton)


        self.formLayout_2.setLayout(1, QFormLayout.ItemRole.FieldRole, self.horizontalLayout_4)

        self.tabWidget.addTab(self.openTab, "")
        self.recentTab = QWidget()
        self.recentTab.setObjectName(u"recentTab")
        self.formLayout_3 = QFormLayout(self.recentTab)
        self.formLayout_3.setObjectName(u"formLayout_3")
        self.recentCombo = QComboBox(self.recentTab)
        self.recentCombo.setObjectName(u"recentCombo")
        sizePolicy = QSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Fixed)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.recentCombo.sizePolicy().hasHeightForWidth())
        self.recentCombo.setSizePolicy(sizePolicy)
        self.recentCombo.setMinimumSize(QSize(300, 0))

        self.formLayout_3.setWidget(0, QFormLayout.ItemRole.LabelRole, self.recentCombo)

        self.horizontalLayout_7 = QHBoxLayout()
        self.horizontalLayout_7.setObjectName(u"horizontalLayout_7")
        self.horizontalSpacer_3 = QSpacerItem(40, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum)

        self.horizontalLayout_7.addItem(self.horizontalSpacer_3)

        self.openRecentProjectButton = QPushButton(self.recentTab)
        self.openRecentProjectButton.setObjectName(u"openRecentProjectButton")
        sizePolicy1 = QSizePolicy(QSizePolicy.Policy.Preferred, QSizePolicy.Policy.Fixed)
        sizePolicy1.setHorizontalStretch(0)
        sizePolicy1.setVerticalStretch(0)
        sizePolicy1.setHeightForWidth(self.openRecentProjectButton.sizePolicy().hasHeightForWidth())
        self.openRecentProjectButton.setSizePolicy(sizePolicy1)
        self.openRecentProjectButton.setMaximumSize(QSize(100, 16777215))

        self.horizontalLayout_7.addWidget(self.openRecentProjectButton)


        self.formLayout_3.setLayout(0, QFormLayout.ItemRole.FieldRole, self.horizontalLayout_7)

        self.tabWidget.addTab(self.recentTab, "")
        self.deleteTab = QWidget()
        self.deleteTab.setObjectName(u"deleteTab")
        self.formLayout_5 = QFormLayout(self.deleteTab)
        self.formLayout_5.setObjectName(u"formLayout_5")
        self.label_10 = QLabel(self.deleteTab)
        self.label_10.setObjectName(u"label_10")

        self.formLayout_5.setWidget(0, QFormLayout.ItemRole.LabelRole, self.label_10)

        self.horizontalLayout_8 = QHBoxLayout()
        self.horizontalLayout_8.setObjectName(u"horizontalLayout_8")
        self.deleteProject = QLineEdit(self.deleteTab)
        self.deleteProject.setObjectName(u"deleteProject")
        self.deleteProject.setReadOnly(True)

        self.horizontalLayout_8.addWidget(self.deleteProject)

        self.deleteBrowseButton = QPushButton(self.deleteTab)
        self.deleteBrowseButton.setObjectName(u"deleteBrowseButton")

        self.horizontalLayout_8.addWidget(self.deleteBrowseButton)


        self.formLayout_5.setLayout(0, QFormLayout.ItemRole.FieldRole, self.horizontalLayout_8)

        self.horizontalLayout_6 = QHBoxLayout()
        self.horizontalLayout_6.setObjectName(u"horizontalLayout_6")
        self.horizontalSpacer_4 = QSpacerItem(40, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum)

        self.horizontalLayout_6.addItem(self.horizontalSpacer_4)

        self.deleteProjectButton = QPushButton(self.deleteTab)
        self.deleteProjectButton.setObjectName(u"deleteProjectButton")

        self.horizontalLayout_6.addWidget(self.deleteProjectButton)


        self.formLayout_5.setLayout(1, QFormLayout.ItemRole.FieldRole, self.horizontalLayout_6)

        self.horizontalLayout_9 = QHBoxLayout()
        self.horizontalLayout_9.setObjectName(u"horizontalLayout_9")
        self.horizontalSpacer_5 = QSpacerItem(40, 20, QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Minimum)

        self.horizontalLayout_9.addItem(self.horizontalSpacer_5)

        self.cleanManifestButton = QPushButton(self.deleteTab)
        self.cleanManifestButton.setObjectName(u"cleanManifestButton")

        self.horizontalLayout_9.addWidget(self.cleanManifestButton)


        self.formLayout_5.setLayout(2, QFormLayout.ItemRole.FieldRole, self.horizontalLayout_9)

        self.tabWidget.addTab(self.deleteTab, "")

        self.verticalLayout.addWidget(self.tabWidget)


        self.retranslateUi(Form)

        self.tabWidget.setCurrentIndex(0)


        QMetaObject.connectSlotsByName(Form)
    # setupUi

    def retranslateUi(self, Form):
        Form.setWindowTitle(QCoreApplication.translate("Form", u"Form", None))
        self.label.setText(QCoreApplication.translate("Form", u"Project name", None))
        self.label_2.setText(QCoreApplication.translate("Form", u"Folder", None))
        self.newBrowseButton.setText(QCoreApplication.translate("Form", u"Browse", None))
        self.label_3.setText(QCoreApplication.translate("Form", u"Create project folder", None))
        self.createProjectFolderButton.setText("")
        self.label_5.setText(QCoreApplication.translate("Form", u"Source folder", None))
        self.label_6.setText(QCoreApplication.translate("Form", u"Resource folder", None))
        self.label_7.setText(QCoreApplication.translate("Form", u"Autogen folder", None))
        self.label_8.setText(QCoreApplication.translate("Form", u"Project filepath", None))
        self.errorCreateProject.setText("")
        self.createProjectButton.setText(QCoreApplication.translate("Form", u"Create Project", None))
        self.label_9.setText(QCoreApplication.translate("Form", u"*Only: A-Z, 0-9, space, _, or -.", None))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.newTab), QCoreApplication.translate("Form", u"New", None))
        self.label_4.setText(QCoreApplication.translate("Form", u"Project filepath", None))
        self.openBrowseButton.setText(QCoreApplication.translate("Form", u"Browse", None))
        self.openProjectButton.setText(QCoreApplication.translate("Form", u"Open Project", None))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.openTab), QCoreApplication.translate("Form", u"Open", None))
        self.openRecentProjectButton.setText(QCoreApplication.translate("Form", u"Open Project", None))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.recentTab), QCoreApplication.translate("Form", u"Recent", None))
        self.label_10.setText(QCoreApplication.translate("Form", u"Project filepath", None))
        self.deleteBrowseButton.setText(QCoreApplication.translate("Form", u"Browse", None))
        self.deleteProjectButton.setText(QCoreApplication.translate("Form", u"Delete Project", None))
        self.cleanManifestButton.setText(QCoreApplication.translate("Form", u"Clean Manifest", None))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.deleteTab), QCoreApplication.translate("Form", u"Delete", None))
    # retranslateUi

