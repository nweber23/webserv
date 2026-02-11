// WebSrv Demo JavaScript

document.addEventListener('DOMContentLoaded', function() {
    // Click counter demo
    const demoBtn = document.getElementById('demo-btn');
    const demoOutput = document.getElementById('demo-output');
    let clickCount = 0;

    demoBtn.addEventListener('click', function() {
        clickCount++;
        demoOutput.textContent = `Clicks: ${clickCount}`;
        
        // Add a little animation effect
        demoOutput.style.transform = 'scale(1.2)';
        setTimeout(() => {
            demoOutput.style.transform = 'scale(1)';
        }, 150);
    });

    // Live clock
    const timeDisplay = document.getElementById('time-display');
    
    function updateTime() {
        const now = new Date();
        const hours = String(now.getHours()).padStart(2, '0');
        const minutes = String(now.getMinutes()).padStart(2, '0');
        const seconds = String(now.getSeconds()).padStart(2, '0');
        timeDisplay.textContent = `${hours}:${minutes}:${seconds}`;
    }

    updateTime();
    setInterval(updateTime, 1000);

    // Smooth scrolling for navigation links
    document.querySelectorAll('a[href^="#"]').forEach(anchor => {
        anchor.addEventListener('click', function(e) {
            e.preventDefault();
            const target = document.querySelector(this.getAttribute('href'));
            if (target) {
                target.scrollIntoView({
                    behavior: 'smooth',
                    block: 'start'
                });
            }
        });
    });

    // Navbar background change on scroll
    const header = document.querySelector('header');
    
    window.addEventListener('scroll', function() {
        if (window.scrollY > 50) {
            header.style.background = 'rgba(44, 62, 80, 0.95)';
        } else {
            header.style.background = 'var(--secondary-color)';
        }
    });

    // Feature cards animation on scroll
    const featureCards = document.querySelectorAll('.feature-card');
    
    const observer = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                entry.target.style.opacity = '1';
                entry.target.style.transform = 'translateY(0)';
            }
        });
    }, { threshold: 0.1 });

    featureCards.forEach(card => {
        card.style.opacity = '0';
        card.style.transform = 'translateY(20px)';
        card.style.transition = 'opacity 0.5s ease, transform 0.5s ease';
        observer.observe(card);
    });

    console.log('WebSrv demo loaded successfully!');
});
