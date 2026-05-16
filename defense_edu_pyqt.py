#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
国防安全科普教育软件 - Python/PyQt5版本
用于在没有C++编译环境时运行
"""

import sys
import json
import os
from pathlib import Path

try:
    from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout,
                                  QHBoxLayout, QPushButton, QLabel, QTextEdit,
                                  QLineEdit, QComboBox, QCheckBox, QSlider,
                                  QFileDialog, QMessageBox, QStackedWidget,
                                  QFrame, QScrollArea, QGridLayout)
    from PyQt5.QtCore import QUrl, Qt, QSettings, pyqtSlot, QObject, pyqtSignal
    from PyQt5.QtNetwork import QNetworkAccessManager, QNetworkRequest, QNetworkReply
except ImportError:
    print("需要安装PyQt5: pip install PyQt5")
    sys.exit(1)


class Backend(QObject):
    """后端接口，供JavaScript调用"""

    def __init__(self, parent=None):
        super().__init__(parent)
        self.main_window = parent

    @pyqtSlot(str, result=str)
    def getSetting(self, key):
        return self.main_window.get_setting(key)

    @pyqtSlot(str, str)
    def setSetting(self, key, value):
        self.main_window.set_setting(key, value)

    @pyqtSlot(str)
    def navigateTo(self, page):
        self.main_window.navigate_to(page)

    @pyqtSlot()
    def checkOllama(self):
        self.main_window.check_ollama()

    @pyqtSlot(str)
    def sendChatMessage(self, message):
        self.main_window.send_chat_message(message)

    @pyqtSlot(result=str)
    def getDefaultPrompt(self):
        return self.main_window.get_default_prompt()


class DefenseEduApp(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("国防安全科普教育软件")
        self.setMinimumSize(1200, 800)
        self.resize(1400, 900)

        # 设置
        self.settings = QSettings("DefenseEdu", "DefenseEducation")

        # 网络管理器
        self.network_manager = QNetworkAccessManager(self)

        # 创建UI
        self.setup_ui()

        # 加载设置
        self.load_settings()

    def setup_ui(self):
        # 中央部件
        central = QWidget()
        self.setCentralWidget(central)
        layout = QHBoxLayout(central)
        layout.setContentsMargins(0, 0, 0, 0)

        # 侧边栏
        sidebar = QFrame()
        sidebar.setFixedWidth(200)
        sidebar.setStyleSheet("""
            QFrame {
                background-color: #1a1a2e;
                border-right: 2px solid #c41e3a;
            }
            QPushButton {
                background-color: transparent;
                color: white;
                border: none;
                padding: 15px;
                text-align: left;
                font-size: 14px;
            }
            QPushButton:hover {
                background-color: rgba(196, 30, 58, 0.3);
            }
        """)
        sidebar_layout = QVBoxLayout(sidebar)
        sidebar_layout.setSpacing(5)

        # Logo
        logo_label = QLabel("★ 国防科普")
        logo_label.setStyleSheet("""
            color: #ffd700;
            font-size: 18px;
            font-weight: bold;
            padding: 20px;
        """)
        sidebar_layout.addWidget(logo_label)

        # 导航按钮
        nav_buttons = [
            ("🏠 首页", "home"),
            ("📅 时间轴", "timeline"),
            ("🤖 AI问答", "chat"),
            ("⚙️ 设置", "settings"),
        ]

        for text, page in nav_buttons:
            btn = QPushButton(text)
            btn.clicked.connect(lambda checked, p=page: self.show_page(p))
            sidebar_layout.addWidget(btn)

        sidebar_layout.addStretch()

        # 版本信息
        version_label = QLabel("v1.0.0")
        version_label.setStyleSheet("color: #666; padding: 10px;")
        version_label.setAlignment(Qt.AlignCenter)
        sidebar_layout.addWidget(version_label)

        layout.addWidget(sidebar)

        # 内容区域
        self.content_stack = QStackedWidget()
        layout.addWidget(self.content_stack)

        # 创建各个页面
        self.create_home_page()
        self.create_timeline_page()
        self.create_chat_page()
        self.create_settings_page()

    def create_home_page(self):
        page = QScrollArea()
        page.setWidgetResizable(True)
        page.setStyleSheet("background-color: #0d0d1a; border: none;")

        content = QWidget()
        content.setStyleSheet("""
            QWidget {
                background-color: #0d0d1a;
                color: white;
            }
            QLabel {
                color: white;
            }
        """)
        layout = QVBoxLayout(content)
        layout.setAlignment(Qt.AlignCenter)

        # 标题
        title = QLabel("国防安全科普教育")
        title.setStyleSheet("""
            font-size: 48px;
            font-weight: bold;
            color: #ffd700;
            margin: 40px;
        """)
        title.setAlignment(Qt.AlignCenter)
        layout.addWidget(title)

        subtitle = QLabel("了解国防历史，掌握国防知识，增强国防意识")
        subtitle.setStyleSheet("font-size: 18px; color: #aaa; margin-bottom: 40px;")
        subtitle.setAlignment(Qt.AlignCenter)
        layout.addWidget(subtitle)

        # 功能卡片
        cards_widget = QWidget()
        cards_layout = QGridLayout(cards_widget)
        cards_layout.setSpacing(20)

        cards = [
            ("📚", "国防历史", "从南昌起义到现代化强军，回顾人民军队的光辉历程"),
            ("🚀", "武器装备", "歼-20、航母、东风导弹...探索中国自主研发的先进武器装备"),
            ("🛡️", "国防政策", "了解新时代中国国防的防御性质和和平发展理念"),
            ("🎖️", "军队建设", "陆军、海军、空军、火箭军、战略支援部队"),
            ("🤖", "AI助手", "内置智能AI助手，基于Ollama本地大模型"),
            ("⚙️", "个性定制", "支持简易模式和专业模式，满足不同用户需求"),
        ]

        for i, (icon, title_text, desc) in enumerate(cards):
            card = QFrame()
            card.setStyleSheet("""
                QFrame {
                    background-color: #1a1a2e;
                    border: 1px solid rgba(196, 30, 58, 0.3);
                    border-radius: 15px;
                    padding: 20px;
                }
                QFrame:hover {
                    border-color: #c41e3a;
                }
            """)
            card_layout = QVBoxLayout(card)

            icon_label = QLabel(icon)
            icon_label.setStyleSheet("font-size: 36px;")
            card_layout.addWidget(icon_label)

            title_label = QLabel(title_text)
            title_label.setStyleSheet("font-size: 18px; font-weight: bold; color: #ffd700;")
            card_layout.addWidget(title_label)

            desc_label = QLabel(desc)
            desc_label.setStyleSheet("color: #aaa;")
            desc_label.setWordWrap(True)
            card_layout.addWidget(desc_label)

            cards_layout.addWidget(card, i // 3, i % 3)

        layout.addWidget(cards_widget)
        layout.addStretch()

        page.setWidget(content)
        self.content_stack.addWidget(page)

    def create_timeline_page(self):
        page = QScrollArea()
        page.setWidgetResizable(True)
        page.setStyleSheet("background-color: #0d0d1a; border: none;")

        content = QWidget()
        layout = QVBoxLayout(content)
        layout.setAlignment(Qt.AlignCenter)

        # 标题
        title = QLabel("国防发展历程")
        title.setStyleSheet("font-size: 36px; font-weight: bold; color: #ffd700; margin: 30px;")
        title.setAlignment(Qt.AlignCenter)
        layout.addWidget(title)

        # 时间轴数据
        timeline_data = [
            ("过去", "1927年", "南昌起义", "中国共产党独立领导武装斗争的开始"),
            ("过去", "1949年", "新中国成立", "中华人民共和国成立，国防建设进入新阶段"),
            ("过去", "1950-1953年", "抗美援朝", "保家卫国，维护朝鲜半岛和平"),
            ("现在", "2012年", "辽宁舰入列", "中国第一艘航空母舰正式交付海军"),
            ("现在", "2017年", "歼-20服役", "中国自主研制的第五代战斗机正式列装"),
            ("现在", "2022年", "福建舰下水", "中国首艘电磁弹射型航空母舰下水"),
            ("未来", "2025年", "智能化作战", "人工智能与军事深度融合"),
            ("未来", "2035年", "现代化军队", "基本实现国防和军队现代化"),
            ("未来", "2049年", "世界一流军队", "全面建成世界一流军队"),
        ]

        for period, year, title_text, desc in timeline_data:
            item = QFrame()
            color = {"过去": "#888", "现在": "#c41e3a", "未来": "#ffd700"}[period]
            item.setStyleSheet(f"""
                QFrame {{
                    background-color: #1a1a2e;
                    border-left: 4px solid {color};
                    border-radius: 10px;
                    padding: 15px;
                    margin: 10px 100px;
                    min-width: 600px;
                }}
            """)
            item_layout = QVBoxLayout(item)

            period_label = QLabel(f"[{period}]")
            period_label.setStyleSheet(f"color: {color}; font-weight: bold;")
            item_layout.addWidget(period_label)

            year_label = QLabel(year)
            year_label.setStyleSheet("font-size: 20px; font-weight: bold; color: #ffd700;")
            item_layout.addWidget(year_label)

            title_lbl = QLabel(title_text)
            title_lbl.setStyleSheet("font-size: 16px; color: white;")
            item_layout.addWidget(title_lbl)

            desc_lbl = QLabel(desc)
            desc_lbl.setStyleSheet("color: #aaa;")
            item_layout.addWidget(desc_lbl)

            layout.addWidget(item)

        layout.addStretch()
        page.setWidget(content)
        self.content_stack.addWidget(page)

    def create_chat_page(self):
        page = QWidget()
        page.setStyleSheet("background-color: #0d0d1a;")
        layout = QVBoxLayout(page)
        layout.setContentsMargins(20, 20, 20, 20)

        # 标题
        title = QLabel("🤖 AI 国防知识助手")
        title.setStyleSheet("font-size: 24px; font-weight: bold; color: #ffd700;")
        title.setAlignment(Qt.AlignCenter)
        layout.addWidget(title)

        # 状态栏
        status_frame = QFrame()
        status_frame.setStyleSheet("""
            QFrame {
                background-color: #1a1a2e;
                border: 1px solid rgba(196, 30, 58, 0.3);
                border-radius: 10px;
                padding: 10px;
            }
        """)
        status_layout = QHBoxLayout(status_frame)

        self.status_label = QLabel("● 未连接")
        self.status_label.setStyleSheet("color: #ff4444;")
        status_layout.addWidget(self.status_label)

        status_layout.addStretch()

        check_btn = QPushButton("检查连接")
        check_btn.setStyleSheet("""
            QPushButton {
                background-color: #c41e3a;
                color: white;
                border: none;
                padding: 8px 20px;
                border-radius: 5px;
            }
            QPushButton:hover {
                background-color: #a01830;
            }
        """)
        check_btn.clicked.connect(self.check_ollama)
        status_layout.addWidget(check_btn)

        layout.addWidget(status_frame)

        # 聊天显示区域
        self.chat_display = QTextEdit()
        self.chat_display.setReadOnly(True)
        self.chat_display.setStyleSheet("""
            QTextEdit {
                background-color: #1a1a2e;
                border: 1px solid rgba(196, 30, 58, 0.3);
                border-radius: 10px;
                color: white;
                padding: 10px;
            }
        """)
        layout.addWidget(self.chat_display)

        # 输入区域
        input_frame = QFrame()
        input_layout = QHBoxLayout(input_frame)
        input_layout.setContentsMargins(0, 0, 0, 0)

        self.chat_input = QTextEdit()
        self.chat_input.setPlaceholderText("请输入您的问题...")
        self.chat_input.setMaximumHeight(80)
        self.chat_input.setStyleSheet("""
            QTextEdit {
                background-color: #1a1a2e;
                border: 1px solid rgba(196, 30, 58, 0.3);
                border-radius: 10px;
                color: white;
                padding: 10px;
            }
        """)
        input_layout.addWidget(self.chat_input)

        send_btn = QPushButton("发送")
        send_btn.setFixedWidth(100)
        send_btn.setStyleSheet("""
            QPushButton {
                background-color: #c41e3a;
                color: white;
                border: none;
                border-radius: 10px;
                font-size: 14px;
            }
            QPushButton:hover {
                background-color: #a01830;
            }
        """)
        send_btn.clicked.connect(self.send_chat_message)
        input_layout.addWidget(send_btn)

        layout.addWidget(input_frame)

        # 快速问题
        quick_frame = QFrame()
        quick_layout = QHBoxLayout(quick_frame)
        quick_layout.setContentsMargins(0, 0, 0, 0)

        quick_questions = [
            "什么是国防？",
            "中国国防政策是什么？",
            "介绍一下歼-20",
        ]

        for q in quick_questions:
            btn = QPushButton(q)
            btn.setStyleSheet("""
                QPushButton {
                    background-color: rgba(196, 30, 58, 0.2);
                    border: 1px solid rgba(196, 30, 58, 0.3);
                    color: white;
                    border-radius: 15px;
                    padding: 5px 15px;
                }
                QPushButton:hover {
                    background-color: #c41e3a;
                }
            """)
            btn.clicked.connect(lambda checked, text=q: self.chat_input.setText(text))
            quick_layout.addWidget(btn)

        quick_layout.addStretch()
        layout.addWidget(quick_frame)

        self.content_stack.addWidget(page)

    def create_settings_page(self):
        page = QScrollArea()
        page.setWidgetResizable(True)
        page.setStyleSheet("background-color: #0d0d1a; border: none;")

        content = QWidget()
        layout = QVBoxLayout(content)
        layout.setAlignment(Qt.AlignCenter)

        # 标题
        title = QLabel("⚙️ 软件设置")
        title.setStyleSheet("font-size: 32px; font-weight: bold; color: #ffd700; margin: 30px;")
        title.setAlignment(Qt.AlignCenter)
        layout.addWidget(title)

        # 设置卡片
        settings_frame = QFrame()
        settings_frame.setStyleSheet("""
            QFrame {
                background-color: #1a1a2e;
                border: 1px solid rgba(196, 30, 58, 0.3);
                border-radius: 15px;
                padding: 20px;
                min-width: 500px;
            }
            QLabel {
                color: white;
                font-size: 14px;
            }
            QLineEdit, QComboBox {
                background-color: #0d0d1a;
                border: 1px solid rgba(196, 30, 58, 0.3);
                color: white;
                padding: 8px;
                border-radius: 5px;
            }
            QCheckBox {
                color: white;
            }
            QCheckBox::indicator {
                width: 20px;
                height: 20px;
            }
        """)
        settings_layout = QVBoxLayout(settings_frame)

        # Ollama URL
        url_label = QLabel("Ollama服务地址:")
        settings_layout.addWidget(url_label)
        self.url_input = QLineEdit("http://localhost:11434")
        settings_layout.addWidget(self.url_input)

        # 模型选择
        model_label = QLabel("模型名称:")
        settings_layout.addWidget(model_label)
        self.model_input = QLineEdit("qwen2.5")
        settings_layout.addWidget(self.model_input)

        # 自动检查
        self.auto_check = QCheckBox("启动时自动检查Ollama服务")
        self.auto_check.setChecked(True)
        settings_layout.addWidget(self.auto_check)

        settings_layout.addSpacing(20)

        # 保存按钮
        save_btn = QPushButton("💾 保存设置")
        save_btn.setStyleSheet("""
            QPushButton {
                background-color: #c41e3a;
                color: white;
                border: none;
                padding: 12px;
                border-radius: 8px;
                font-size: 16px;
            }
            QPushButton:hover {
                background-color: #a01830;
            }
        """)
        save_btn.clicked.connect(self.save_settings)
        settings_layout.addWidget(save_btn)

        layout.addWidget(settings_frame)
        layout.addStretch()

        page.setWidget(content)
        self.content_stack.addWidget(page)

    def show_page(self, page):
        pages = {
            "home": 0,
            "timeline": 1,
            "chat": 2,
            "settings": 3,
        }
        self.content_stack.setCurrentIndex(pages.get(page, 0))

    def navigate_to(self, page):
        self.show_page(page)

    def get_setting(self, key):
        return self.settings.value(key, "")

    def set_setting(self, key, value):
        self.settings.setValue(key, value)

    def load_settings(self):
        url = self.settings.value("ollamaUrl", "http://localhost:11434")
        model = self.settings.value("ollamaModel", "qwen2.5")
        auto_check = self.settings.value("autoCheckOllama", "true")

        self.url_input.setText(url)
        self.model_input.setText(model)
        self.auto_check.setChecked(auto_check == "true")

        if auto_check == "true":
            self.check_ollama()

    def save_settings(self):
        self.settings.setValue("ollamaUrl", self.url_input.text())
        self.settings.setValue("ollamaModel", self.model_input.text())
        self.settings.setValue("autoCheckOllama", "true" if self.auto_check.isChecked() else "false")

        QMessageBox.information(self, "保存成功", "设置已保存！")

    def check_ollama(self):
        self.status_label.setText("● 检查中...")
        self.status_label.setStyleSheet("color: #ffaa00;")

        url = self.url_input.text() + "/api/tags"
        request = QNetworkRequest(QUrl(url))

        reply = self.network_manager.get(request)
        reply.finished.connect(lambda: self.on_ollama_check_finished(reply))

    def on_ollama_check_finished(self, reply):
        if reply.error() == QNetworkReply.NoError:
            self.status_label.setText("● 已连接")
            self.status_label.setStyleSheet("color: #00ff00;")
        else:
            self.status_label.setText("● 未连接")
            self.status_label.setStyleSheet("color: #ff4444;")
        reply.deleteLater()

    def send_chat_message(self):
        message = self.chat_input.toPlainText().strip()
        if not message:
            return

        # 显示用户消息
        self.chat_display.append(f"<p style='color: #ffd700;'><b>您:</b> {message}</p>")
        self.chat_input.clear()

        # 模拟AI回复
        self.chat_display.append("<p style='color: #aaa;'><b>AI:</b> 正在思考...</p>")

        # 这里可以集成实际的Ollama API调用
        # 简化版本使用模拟回复
        import random
        responses = [
            "国防是国家为防备和抵抗侵略，制止武装颠覆，保卫国家的主权、统一、领土完整和安全所进行的军事活动。",
            "中国奉行防御性的国防政策，坚持走和平发展道路。",
            "歼-20是中国自主研制的第五代隐身战斗机，具有高隐身性、高态势感知、高机动性等特点。",
        ]
        response = random.choice(responses)

        # 延迟显示回复
        from PyQt5.QtCore import QTimer
        QTimer.singleShot(1000, lambda: self.show_ai_response(response))

    def show_ai_response(self, response):
        # 移除"正在思考"
        html = self.chat_display.toHtml()
        html = html.replace("正在思考...", response)
        self.chat_display.setHtml(html)

    def get_default_prompt(self):
        return """你是一位专业的国防安全科普助手。你的职责是：
1. 准确回答关于国防历史、军事科技、战略思想等方面的问题
2. 用通俗易懂的语言解释复杂的军事概念
3. 传播爱国主义精神和国防意识
4. 强调中国国防的防御性质和和平发展理念
5. 避免涉及敏感军事机密信息"""


def main():
    app = QApplication(sys.argv)
    app.setStyle('Fusion')

    # 设置应用样式
    app.setStyleSheet("""
        QMainWindow {
            background-color: #0d0d1a;
        }
        QScrollArea {
            border: none;
        }
    """)

    window = DefenseEduApp()
    window.show()

    sys.exit(app.exec_())


if __name__ == "__main__":
    main()
