
"""
国防安全科普教育软件 - Python调试版本
用于在没有Qt环境时测试HTML/CSS/JS
"""

import http.server
import socketserver
import os
import webbrowser
from pathlib import Path

PORT = 8000

class MyHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header('Cache-Control', 'no-store, no-cache, must-revalidate')
        super().end_headers()

    def translate_path(self, path):
        # 处理qrc路径
        if path.startswith('/qrc:'):
            path = path[5:]  # 移除/qrc:前缀
        return super().translate_path(path)

def main():
    # 切换到项目目录
    os.chdir(Path(__file__).parent)

    # 创建服务器
    with socketserver.TCPServer(("", PORT), MyHTTPRequestHandler) as httpd:
        print(f"服务器运行在 http://localhost:{PORT}")
        print(f"请访问: http://localhost:{PORT}/resources/html/main.html")
        print("按 Ctrl+C 停止服务器")

        # 自动打开浏览器
        webbrowser.open(f"http://localhost:{PORT}/resources/html/main.html")

        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n服务器已停止")

if __name__ == "__main__":
    main()
