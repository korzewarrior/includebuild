// Wait for DOM to be fully loaded
document.addEventListener('DOMContentLoaded', function() {
    // Mobile navigation toggle with enhanced animations
    const mobileNavToggle = document.querySelector('.mobile-nav-toggle');
    const navLinks = document.querySelector('.nav-links');
    const nav = document.querySelector('nav');

    if (mobileNavToggle) {
        mobileNavToggle.addEventListener('click', function() {
            navLinks.classList.toggle('active');
            this.classList.toggle('open');
            nav.classList.toggle('mobile-active');
            
            // Add subtle animation to each menu item
            if (navLinks.classList.contains('active')) {
                const links = navLinks.querySelectorAll('a');
                links.forEach((link, index) => {
                    link.style.opacity = '0';
                    link.style.transform = 'translateY(-10px)';
                    
                    setTimeout(() => {
                        link.style.transition = 'all 0.3s ease';
                        link.style.transitionDelay = `${index * 0.05}s`;
                        link.style.opacity = '1';
                        link.style.transform = 'translateY(0)';
                    }, 50);
                });
            }
        });
    }

    // Set active nav link based on current section
    const sections = document.querySelectorAll('section[id]');
    const navItems = document.querySelectorAll('.nav-links a[href^="#"]');
    
    function setActiveNavItem() {
        let scrollPosition = window.scrollY + 100;
        
        sections.forEach(section => {
            const sectionTop = section.offsetTop;
            const sectionHeight = section.offsetHeight;
            const sectionId = section.getAttribute('id');
            
            if (scrollPosition >= sectionTop && scrollPosition < sectionTop + sectionHeight) {
                navItems.forEach(navItem => {
                    navItem.classList.remove('active');
                    if (navItem.getAttribute('href') === `#${sectionId}`) {
                        navItem.classList.add('active');
                    }
                });
            }
        });
        
        // Check if we're at the top of the page and no section is active
        if (scrollPosition < sections[0].offsetTop) {
            navItems.forEach(navItem => navItem.classList.remove('active'));
        }
    }
    
    // Run on load
    setActiveNavItem();
    
    // Run on scroll
    window.addEventListener('scroll', setActiveNavItem);

    // Code tabs functionality with animated transitions
    const codeTabs = document.querySelectorAll('.code-tab');
    const codePanes = document.querySelectorAll('.code-pane');

    codeTabs.forEach(tab => {
        tab.addEventListener('click', () => {
            // Remove active class from all tabs and panes
            codeTabs.forEach(t => t.classList.remove('active'));
            codePanes.forEach(p => p.classList.remove('active'));
            
            // Add active class to clicked tab
            tab.classList.add('active');
            
            // Find and activate corresponding pane
            const targetPane = document.getElementById(tab.dataset.tab);
            if (targetPane) {
                targetPane.classList.add('active');
            }
        });
    });

    // Smooth scrolling for anchor links with enhanced easing
    document.querySelectorAll('a[href^="#"]').forEach(anchor => {
        anchor.addEventListener('click', function (e) {
            if (this.getAttribute('href') !== '#') {
                e.preventDefault();
                const target = document.querySelector(this.getAttribute('href'));
                if (target) {
                    const headerOffset = 90;
                    const elementPosition = target.getBoundingClientRect().top;
                    const offsetPosition = elementPosition + window.pageYOffset - headerOffset;

                    window.scrollTo({
                        top: offsetPosition,
                        behavior: 'smooth'
                    });
                }

                // Close mobile menu if open
                if (navLinks.classList.contains('active')) {
                    navLinks.classList.remove('active');
                    mobileNavToggle.classList.remove('open');
                }
            }
        });
    });

    // Enhanced sticky header with transparency effect
    const header = document.querySelector('nav');
    const hero = document.querySelector('.hero');
    
    if (header && hero) {
        window.addEventListener('scroll', () => {
            if (window.scrollY > 10) {
                header.classList.add('scrolled');
            } else {
                header.classList.remove('scrolled');
            }
            
            // Add shadow based on scroll depth
            if (window.scrollY > 50) {
                header.style.boxShadow = '0 4px 20px rgba(0, 0, 0, 0.1)';
            } else {
                header.style.boxShadow = '0 2px 10px rgba(0, 0, 0, 0.08)';
            }
        });
    }

    // Animated terminal with sequential typing effect
    const terminalLines = document.querySelectorAll('.terminal-line:not(:last-child)');
    const lastTerminalLine = document.querySelector('.terminal-line:last-child');
    
    if (terminalLines.length > 0) {
        // Hide all lines initially except the first one
        terminalLines.forEach((line, index) => {
            if (index > 0) {
                line.style.opacity = '0';
                line.style.transform = 'translateY(10px)';
            }
        });
        
        // Show last terminal line with cursor
        if (lastTerminalLine) {
            lastTerminalLine.style.opacity = '0';
            lastTerminalLine.style.transform = 'translateY(10px)';
        }
        
        // Show lines sequentially with typing effect
        let lineIndex = 1;
        const lineInterval = setInterval(() => {
            if (lineIndex < terminalLines.length) {
                terminalLines[lineIndex].style.opacity = '1';
                terminalLines[lineIndex].style.transform = 'translateY(0)';
                
                // Add typing sound effect
                const typingSound = new Audio('data:audio/mp3;base64,SUQzAwAAAAABBFRYWFgAAAASAAADbWFqb3JfYnJhbmQAZGFzaABUWFhYAAAAEQAAA21pbm9yX3ZlcnNpb24AMABUWFhYAAAAHAAAA2NvbXBhdGlibGVfYnJhbmRzAGlzbzZtcDQxAFRERU4AAAAVAAADMDE5AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA');
                typingSound.volume = 0.1;
                //typingSound.play();
                
                lineIndex++;
            } else {
                if (lastTerminalLine) {
                    lastTerminalLine.style.opacity = '1';
                    lastTerminalLine.style.transform = 'translateY(0)';
                }
                clearInterval(lineInterval);
            }
        }, 400);
    }

    // Add animations for sections when they come into view with variable speeds
    const animatedElements = document.querySelectorAll('.feature, .example-card, .docs-card, .community-link');
    
    const observerOptions = {
        root: null,
        rootMargin: '0px',
        threshold: 0.15
    };
    
    const appearOnScroll = new IntersectionObserver((entries, observer) => {
        entries.forEach((entry, index) => {
            if (entry.isIntersecting) {
                // Add slight delay for each element to create staggered effect
                setTimeout(() => {
                    entry.target.classList.add('animated');
                }, index * 100);
                observer.unobserve(entry.target);
            }
        });
    }, observerOptions);
    
    animatedElements.forEach(el => {
        el.classList.add('animate-on-scroll');
        appearOnScroll.observe(el);
    });

    // Parallax effect for hero section
    const heroSection = document.querySelector('.hero');
    const heroContent = document.querySelector('.hero-content');
    const terminal = document.querySelector('.terminal');
    
    if (heroSection && heroContent && terminal) {
        window.addEventListener('scroll', () => {
            const scrollValue = window.scrollY;
            if (scrollValue < 800) {
                heroContent.style.transform = `translateY(${scrollValue * 0.2}px)`;
                terminal.style.transform = `perspective(1000px) rotateY(-5deg) rotateX(2deg) translateY(${scrollValue * 0.1}px)`;
            }
        });
    }
    
    // Add hover effect to feature icons
    const featureIcons = document.querySelectorAll('.feature-icon');
    featureIcons.forEach(icon => {
        icon.addEventListener('mouseenter', () => {
            icon.style.transform = 'rotate(0deg) scale(1.1)';
        });
        icon.addEventListener('mouseleave', () => {
            icon.style.transform = 'rotate(-5deg) scale(1)';
        });
    });
    
    // Add subtle hover effect to code blocks
    const codeBlocks = document.querySelectorAll('.code-block');
    codeBlocks.forEach(block => {
        block.addEventListener('mouseenter', () => {
            block.style.transform = 'perspective(1000px) rotateX(2deg)';
            block.style.boxShadow = 'var(--shadow-md)';
        });
        block.addEventListener('mouseleave', () => {
            block.style.transform = 'perspective(1000px) rotateX(0deg)';
            block.style.boxShadow = 'var(--shadow-sm)';
        });
    });

    // Add CSS for additional animation and interaction effects
    const style = document.createElement('style');
    style.textContent = `
        .animate-on-scroll {
            opacity: 0;
            transform: translateY(40px);
            transition: opacity 0.8s ease, transform 0.8s cubic-bezier(0.175, 0.885, 0.32, 1.275);
        }
        
        .animated {
            opacity: 1;
            transform: translateY(0);
        }
        
        .nav-links.active {
            display: flex;
            flex-direction: column;
            position: absolute;
            top: 80px;
            left: 0;
            right: 0;
            background-color: white;
            box-shadow: 0 5px 20px rgba(0, 0, 0, 0.1);
            padding: 1.5rem 0;
            z-index: 100;
            animation: slideDown 0.3s ease forwards;
        }
        
        @keyframes slideDown {
            from { opacity: 0; transform: translateY(-10px); }
            to { opacity: 1; transform: translateY(0); }
        }
        
        .nav-links.active li {
            margin: 0.8rem 0;
            padding: 0.5rem 2rem;
            opacity: 0;
            animation: fadeIn 0.5s ease forwards;
        }
        
        .nav-links.active li:nth-child(1) { animation-delay: 0.1s; }
        .nav-links.active li:nth-child(2) { animation-delay: 0.2s; }
        .nav-links.active li:nth-child(3) { animation-delay: 0.3s; }
        .nav-links.active li:nth-child(4) { animation-delay: 0.4s; }
        
        .mobile-nav-toggle.open span:nth-child(1) {
            transform: rotate(45deg) translate(5px, 5px);
        }
        
        .mobile-nav-toggle.open span:nth-child(2) {
            opacity: 0;
        }
        
        .mobile-nav-toggle.open span:nth-child(3) {
            transform: rotate(-45deg) translate(7px, -6px);
        }
        
        nav.scrolled {
            background-color: rgba(255, 255, 255, 0.98);
            backdrop-filter: blur(10px);
            -webkit-backdrop-filter: blur(10px);
        }
        
        .terminal-line {
            transition: opacity 0.5s ease, transform 0.5s ease;
        }
        
        .feature:nth-child(even) .feature-icon {
            transform: rotate(5deg);
        }
        
        .feature:nth-child(even):hover .feature-icon {
            transform: rotate(0deg) scale(1.1);
        }
        
        .logo .include, .logo .build {
            position: relative;
        }
        
        .logo .include:after {
            content: '';
            position: absolute;
            width: 100%;
            height: 2px;
            background: var(--primary-gradient);
            bottom: 0;
            left: 0;
            transform: scaleX(0);
            transform-origin: right;
            transition: transform 0.3s ease;
        }
        
        .logo .build:after {
            content: '';
            position: absolute;
            width: 100%;
            height: 2px;
            background: var(--secondary-gradient);
            bottom: 0;
            left: 0;
            transform: scaleX(0);
            transform-origin: left;
            transition: transform 0.3s ease;
        }
        
        .logo a:hover .include:after,
        .logo a:hover .build:after {
            transform: scaleX(1);
        }
    `;
    document.head.appendChild(style);
    
    // Add subtle cursor trail effect that follows mouse movement
    const cursorTrail = document.createElement('div');
    cursorTrail.className = 'cursor-trail';
    document.body.appendChild(cursorTrail);
    
    // Add cursor trail CSS
    const cursorTrailStyle = document.createElement('style');
    cursorTrailStyle.textContent = `
        .cursor-trail {
            position: fixed;
            width: 8px;
            height: 8px;
            border-radius: 50%;
            background: var(--primary-gradient);
            pointer-events: none;
            opacity: 0;
            z-index: 9999;
            mix-blend-mode: screen;
            transition: transform 0.1s ease, opacity 0.3s ease;
            filter: blur(2px);
            box-shadow: 0 0 15px rgba(54, 98, 255, 0.5);
        }
    `;
    
    document.head.appendChild(cursorTrailStyle);
    
    document.addEventListener('mousemove', (e) => {
        // Animate cursor trail
        cursorTrail.style.opacity = '0.7';
        cursorTrail.style.transform = `translate(${e.clientX - 4}px, ${e.clientY - 4}px) scale(1.5)`;
        
        setTimeout(() => {
            cursorTrail.style.opacity = '0';
            cursorTrail.style.transform = `translate(${e.clientX - 4}px, ${e.clientY - 4}px) scale(0.5)`;
        }, 150);
    });
}); 