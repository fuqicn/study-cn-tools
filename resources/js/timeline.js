// 国防安全科普教育软件 - 时间轴JavaScript

// 显示特定时期的时间轴
function showPeriod(period) {
    // 更新按钮状态
    document.querySelectorAll('.period-btn').forEach(btn => {
        btn.classList.remove('active');
    });
    event.target.classList.add('active');

    // 过滤时间轴项目
    const items = document.querySelectorAll('.timeline-item');
    items.forEach((item, index) => {
        const itemPeriod = item.getAttribute('data-period');

        if (period === 'all' || itemPeriod === period) {
            item.style.display = 'block';
            // 重新触发动画
            item.style.animation = 'none';
            setTimeout(() => {
                item.style.animation = `slideUp 0.6s ease-out forwards`;
                item.style.animationDelay = `${index * 0.1}s`;
            }, 10);
        } else {
            item.style.display = 'none';
        }
    });

    // 更新时间轴线条
    updateTimelineLine();
}

// 更新时间轴线条
function updateTimelineLine() {
    const visibleItems = document.querySelectorAll('.timeline-item[style*="block"], .timeline-item:not([style*="none"])');
    const timeline = document.querySelector('.timeline');

    if (visibleItems.length > 0 && timeline) {
        // 重新计算时间轴线条高度
        const firstItem = visibleItems[0];
        const lastItem = visibleItems[visibleItems.length - 1];

        const timelineRect = timeline.getBoundingClientRect();
        const firstRect = firstItem.getBoundingClientRect();
        const lastRect = lastItem.getBoundingClientRect();

        const startY = firstRect.top - timelineRect.top + firstRect.height / 2;
        const endY = lastRect.top - timelineRect.top + lastRect.height / 2;
    }
}

// 滚动动画
function initScrollAnimation() {
    const observerOptions = {
        root: null,
        rootMargin: '0px',
        threshold: 0.1
    };

    const observer = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                entry.target.style.opacity = '1';
                entry.target.style.transform = 'translateY(0)';
            }
        });
    }, observerOptions);

    document.querySelectorAll('.timeline-item').forEach(item => {
        item.style.opacity = '0';
        item.style.transform = 'translateY(20px)';
        item.style.transition = 'opacity 0.6s ease, transform 0.6s ease';
        observer.observe(item);
    });
}

// 时间轴项目点击展开详情
function initTimelineItems() {
    document.querySelectorAll('.timeline-content').forEach(content => {
        content.addEventListener('click', function() {
            // 添加点击效果
            this.style.transform = 'scale(0.98)';
            setTimeout(() => {
                this.style.transform = 'scale(1.02)';
            }, 100);

            // 可以在这里添加展开详情的逻辑
            const title = this.querySelector('.timeline-title').textContent;
            const year = this.querySelector('.timeline-year').textContent;
            console.log(`点击了: ${year} - ${title}`);
        });
    });
}

// 键盘导航
function initKeyboardNavigation() {
    let currentIndex = -1;
    const items = document.querySelectorAll('.timeline-item');

    document.addEventListener('keydown', (e) => {
        if (e.key === 'ArrowDown') {
            e.preventDefault();
            currentIndex = Math.min(currentIndex + 1, items.length - 1);
            focusItem(items[currentIndex]);
        } else if (e.key === 'ArrowUp') {
            e.preventDefault();
            currentIndex = Math.max(currentIndex - 1, 0);
            focusItem(items[currentIndex]);
        }
    });
}

function focusItem(item) {
    if (item) {
        item.scrollIntoView({ behavior: 'smooth', block: 'center' });
        item.querySelector('.timeline-content').classList.add('glow');
        setTimeout(() => {
            item.querySelector('.timeline-content').classList.remove('glow');
        }, 1000);
    }
}

// 触摸滑动支持
function initTouchSupport() {
    let touchStartY = 0;
    let touchEndY = 0;

    const timeline = document.querySelector('.timeline');
    if (!timeline) return;

    timeline.addEventListener('touchstart', (e) => {
        touchStartY = e.changedTouches[0].screenY;
    }, { passive: true });

    timeline.addEventListener('touchend', (e) => {
        touchEndY = e.changedTouches[0].screenY;
        handleSwipe();
    }, { passive: true });

    function handleSwipe() {
        const swipeThreshold = 50;
        const diff = touchStartY - touchEndY;

        if (Math.abs(diff) > swipeThreshold) {
            // 可以添加滑动切换时期的逻辑
            const buttons = document.querySelectorAll('.period-btn');
            const activeIndex = Array.from(buttons).findIndex(btn => btn.classList.contains('active'));

            if (diff > 0 && activeIndex < buttons.length - 1) {
                // 向上滑动，下一个时期
                buttons[activeIndex + 1].click();
            } else if (diff < 0 && activeIndex > 0) {
                // 向下滑动，上一个时期
                buttons[activeIndex - 1].click();
            }
        }
    }
}

// 动态加载更多内容
function loadMoreContent(period) {
    // 这里可以从后端加载更多时间轴内容
    if (typeof qt !== 'undefined' && qt.mainwindow) {
        const timelineData = qt.mainwindow.getTimelineData();
        console.log('Timeline data:', timelineData);
    }
}

// 导出函数
window.showPeriod = showPeriod;
window.updateTimelineLine = updateTimelineLine;

// 初始化
document.addEventListener('DOMContentLoaded', function() {
    initScrollAnimation();
    initTimelineItems();
    initKeyboardNavigation();
    initTouchSupport();
});
