// 国防安全科普教育软件 - AI聊天JavaScript

let isWaitingResponse = false;
let currentResponseElement = null;

// 发送消息
function sendMessage() {
    const input = document.getElementById('chatInput');
    const message = input.value.trim();

    if (!message || isWaitingResponse) return;

    // 添加用户消息
    addUserMessage(message);

    // 清空输入框
    input.value = '';
    input.style.height = 'auto';

    // 显示打字指示器
    showTypingIndicator(true);

    // 禁用发送按钮
    setSendButtonState(false);

    // 发送到后端
    if (typeof qt !== 'undefined' && qt.mainwindow) {
        isWaitingResponse = true;
        qt.mainwindow.sendChatMessage(message);
    } else {
        // 模拟回复
        setTimeout(() => {
            showTypingIndicator(false);
            addAIMessage('这是一个模拟回复。在实际运行中，AI会根据您的问题提供专业的国防知识回答。');
            setSendButtonState(true);
            isWaitingResponse = false;
        }, 2000);
    }
}

// 发送快速问题
function sendQuickQuestion(question) {
    const input = document.getElementById('chatInput');
    input.value = question;
    sendMessage();
}

// 添加用户消息
function addUserMessage(message) {
    const container = document.getElementById('chatMessages');

    // 移除欢迎消息
    const welcome = container.querySelector('.welcome-message');
    if (welcome) welcome.remove();

    const messageDiv = document.createElement('div');
    messageDiv.className = 'message message-user';
    messageDiv.innerHTML = `
        <div class="message-bubble">${escapeHtml(message)}</div>
        <span class="message-avatar">👤</span>
    `;

    container.appendChild(messageDiv);
    scrollToBottom();
}

// 添加AI消息（开始）
function addAIMessageStart() {
    const container = document.getElementById('chatMessages');

    const messageDiv = document.createElement('div');
    messageDiv.className = 'message message-ai';
    messageDiv.innerHTML = `
        <span class="message-avatar">🤖</span>
        <div class="message-bubble" id="aiResponse"></div>
    `;

    container.appendChild(messageDiv);
    currentResponseElement = messageDiv.querySelector('#aiResponse');
    scrollToBottom();
}

// 添加AI消息（完整）
function addAIMessage(message) {
    const container = document.getElementById('chatMessages');

    const messageDiv = document.createElement('div');
    messageDiv.className = 'message message-ai';
    messageDiv.innerHTML = `
        <span class="message-avatar">🤖</span>
        <div class="message-bubble">${formatMessage(message)}</div>
    `;

    container.appendChild(messageDiv);
    scrollToBottom();
}

// 追加响应内容（流式）
window.appendResponse = function(text) {
    showTypingIndicator(false);

    if (!currentResponseElement) {
        addAIMessageStart();
    }

    if (currentResponseElement) {
        currentResponseElement.innerHTML = formatMessage(text);
        scrollToBottom();
    }

    // 如果收到空字符串，表示响应结束
    if (text === '') {
        currentResponseElement = null;
        isWaitingResponse = false;
        setSendButtonState(true);
    }
};

// 格式化消息（支持Markdown）
function formatMessage(text) {
    // 转义HTML
    text = escapeHtml(text);

    // 代码块
    text = text.replace(/```(\w+)?\n([\s\S]*?)```/g, '<pre><code>$2</code></pre>');

    // 行内代码
    text = text.replace(/`([^`]+)`/g, '<code>$1</code>');

    // 粗体
    text = text.replace(/\*\*([^*]+)\*\*/g, '<strong>$1</strong>');

    // 斜体
    text = text.replace(/\*([^*]+)\*/g, '<em>$1</em>');

    // 换行
    text = text.replace(/\n/g, '<br>');

    return text;
}

// 转义HTML
function escapeHtml(text) {
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
}

// 显示/隐藏打字指示器
function showTypingIndicator(show) {
    const indicator = document.getElementById('typingIndicator');
    if (indicator) {
        indicator.classList.toggle('active', show);
    }
}

// 设置发送按钮状态
function setSendButtonState(enabled) {
    const btn = document.getElementById('sendBtn');
    if (btn) {
        btn.disabled = !enabled;
    }
}

// 滚动到底部
function scrollToBottom() {
    const container = document.getElementById('chatMessages');
    if (container) {
        container.scrollTop = container.scrollHeight;
    }
}

// 清空聊天记录
function clearChat() {
    const container = document.getElementById('chatMessages');
    if (container) {
        container.innerHTML = `
            <div class="welcome-message">
                <h2>👋 欢迎来到国防知识AI助手</h2>
                <p>我可以帮您解答关于国防历史、军事科技、国防政策等方面的问题。</p>
            </div>
        `;
    }
}

// 导出函数
window.sendMessage = sendMessage;
window.sendQuickQuestion = sendQuickQuestion;
window.clearChat = clearChat;
