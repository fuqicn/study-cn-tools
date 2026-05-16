// 国防安全科普教育软件 - 主JavaScript文件

// 全局变量
var qt = {
    mainwindow: null
};

// 初始化QWebChannel
document.addEventListener('DOMContentLoaded', function() {
    if (typeof qt !== 'undefined' && typeof qt.webChannelTransport !== 'undefined') {
        new QWebChannel(qt.webChannelTransport, function(channel) {
            qt.mainwindow = channel.objects.mainwindow;
            console.log('QWebChannel initialized');
        });
    } else {
        // 开发调试模式
        console.log('Running in development mode (no QWebChannel)');
        setupMockQt();
    }
});

// 模拟Qt对象（用于调试）
function setupMockQt() {
    qt.mainwindow = {
        navigateTo: function(page) {
            console.log('Navigate to:', page);
            window.location.href = page + '.html';
        },
        checkOllama: function() {
            console.log('Checking Ollama...');
            setTimeout(() => {
                if (window.updateOllamaStatus) {
                    window.updateOllamaStatus(true, 'Ollama 服务已连接（模拟）');
                }
            }, 1000);
        },
        sendChatMessage: function(message) {
            console.log('Sending message:', message);
            setTimeout(() => {
                if (window.appendResponse) {
                    window.appendResponse('这是一个模拟回复。在实际运行中，这里会显示AI的回答内容。');
                }
            }, 1500);
        },
        fetchOllamaModels: function() {
            console.log('Fetching models...');
            setTimeout(() => {
                if (window.updateModels) {
                    window.updateModels(['qwen2.5', 'llama2', 'mistral']);
                }
            }, 500);
        },
        getSetting: function(key) {
            const defaults = {
                'theme': 'dark',
                'fontSize': '14',
                'ollamaModel': 'qwen2.5',
                'ollamaUrl': 'http://localhost:11434',
                'autoCheckOllama': 'true',
                'useCustomCss': 'false',
                'customCssPath': ''
            };
            return defaults[key] || '';
        },
        setSetting: function(key, value) {
            console.log('Setting', key, 'to', value);
        },
        saveSettings: function() {
            console.log('Settings saved');
        },
        getDefaultPrompt: function() {
            return `你是一位专业的国防安全科普助手。你的职责是：
1. 准确回答关于国防历史、军事科技、战略思想等方面的问题
2. 用通俗易懂的语言解释复杂的军事概念
3. 传播爱国主义精神和国防意识
4. 强调中国国防的防御性质和和平发展理念
5. 避免涉及敏感军事机密信息

请基于公开资料和权威信息回答问题，保持客观、准确、积极的立场。`;
        },
        loadCustomCss: function() {
            return '';
        }
    };
}

// 导航函数
function navigate(page) {
    if (qt.mainwindow) {
        qt.mainwindow.navigateTo(page);
    } else {
        console.log('Navigate to:', page);
    }
}

// 检查Ollama状态
function checkOllama() {
    if (qt.mainwindow) {
        qt.mainwindow.checkOllama();
    }
}

// 更新Ollama状态（被C++调用）
window.updateOllamaStatus = function(available, message) {
    const statusDot = document.getElementById('statusDot');
    const statusText = document.getElementById('statusText');

    if (statusDot && statusText) {
        if (available) {
            statusDot.classList.remove('offline');
            statusDot.classList.add('online');
        } else {
            statusDot.classList.remove('online');
            statusDot.classList.add('offline');
        }
        statusText.textContent = message;
    }

    // 如果可用，获取模型列表
    if (available && qt.mainwindow) {
        qt.mainwindow.fetchOllamaModels();
    }
};

// 更新模型列表（被C++调用）
window.updateModels = function(models) {
    const select = document.getElementById('modelSelect');
    if (select && models && models.length > 0) {
        // 保存当前选择
        const currentValue = select.value;

        // 清空并重新填充
        select.innerHTML = '';
        models.forEach(model => {
            const option = document.createElement('option');
            option.value = model;
            option.textContent = model;
            select.appendChild(option);
        });

        // 恢复选择
        if (currentValue && models.includes(currentValue)) {
            select.value = currentValue;
        }
    }
};

// 显示错误（被C++调用）
window.showError = function(error) {
    const errorDiv = document.getElementById('errorMessage');
    if (errorDiv) {
        errorDiv.textContent = error;
        errorDiv.classList.add('show');
        setTimeout(() => {
            errorDiv.classList.remove('show');
        }, 5000);
    }
};

// 添加CSS动画类
function addAnimation(element, animationClass) {
    element.classList.remove(animationClass);
    void element.offsetWidth; // 触发重排
    element.classList.add(animationClass);
}

// 平滑滚动到元素
function scrollToElement(element) {
    element.scrollIntoView({ behavior: 'smooth', block: 'center' });
}

// 防抖函数
function debounce(func, wait) {
    let timeout;
    return function executedFunction(...args) {
        const later = () => {
            clearTimeout(timeout);
            func(...args);
        };
        clearTimeout(timeout);
        timeout = setTimeout(later, wait);
    };
}

// 节流函数
function throttle(func, limit) {
    let inThrottle;
    return function(...args) {
        if (!inThrottle) {
            func.apply(this, args);
            inThrottle = true;
            setTimeout(() => inThrottle = false, limit);
        }
    };
}

// 格式化日期
function formatDate(date) {
    const d = new Date(date);
    return `${d.getFullYear()}年${d.getMonth() + 1}月${d.getDate()}日`;
}

// 复制到剪贴板
function copyToClipboard(text) {
    if (navigator.clipboard) {
        navigator.clipboard.writeText(text);
    } else {
        const textarea = document.createElement('textarea');
        textarea.value = text;
        document.body.appendChild(textarea);
        textarea.select();
        document.execCommand('copy');
        document.body.removeChild(textarea);
    }
}

// 本地存储封装
const storage = {
    set: function(key, value) {
        try {
            localStorage.setItem(key, JSON.stringify(value));
        } catch (e) {
            console.error('Storage error:', e);
        }
    },
    get: function(key, defaultValue) {
        try {
            const item = localStorage.getItem(key);
            return item ? JSON.parse(item) : defaultValue;
        } catch (e) {
            console.error('Storage error:', e);
            return defaultValue;
        }
    },
    remove: function(key) {
        try {
            localStorage.removeItem(key);
        } catch (e) {
            console.error('Storage error:', e);
        }
    }
};

// 主题切换
function setTheme(theme) {
    document.documentElement.setAttribute('data-theme', theme);
    storage.set('theme', theme);
}

// 初始化主题
function initTheme() {
    const savedTheme = storage.get('theme', 'dark');
    setTheme(savedTheme);
}

// 页面加载完成后初始化
document.addEventListener('DOMContentLoaded', initTheme);

// 导出全局函数
window.navigate = navigate;
window.checkOllama = checkOllama;
