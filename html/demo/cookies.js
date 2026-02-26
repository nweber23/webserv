/**
 * Cookie Demo - Client-side UI enhancements
 * This script provides a better user experience by:
 * - Updating the color display in real-time as the user picks a color
 * - Calculating text brightness to ensure readability
 * - Reading the browser's cookie storage to show the current saved value
 *
 * NOTE: The actual cookie management (Set-Cookie header, parsing) is
 * handled by the C++ server. This script only enhances the UI.
 */

document.addEventListener('DOMContentLoaded', function() {
    var colorPicker = document.getElementById('colorPicker');
    var colorDisplay = document.getElementById('colorDisplay');
    var colorValue = document.getElementById('colorValue');
    var cookieCurrent = document.querySelector('.cookie-current');

    /**
     * Update the color preview box with selected color and
     * calculate text color (dark/light) for readability
     */
    function updateColorDisplay(color) {
        if (color) {
            colorDisplay.style.backgroundColor = color;
            colorValue.textContent = color;
            var rgb = hexToRgb(color);
            var brightness = (rgb.r * 299 + rgb.g * 587 + rgb.b * 114) / 1000;
            colorDisplay.style.color = brightness > 128 ? '#1a1a1a' : '#f6f4f0';
            colorDisplay.textContent = color;
        } else {
            colorDisplay.style.backgroundColor = 'transparent';
            colorValue.textContent = 'Select a color';
            colorDisplay.textContent = '';
        }
    }

    /**
     * Convert hex color to RGB for brightness calculation
     */
    function hexToRgb(hex) {
        var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
        return result ? {
            r: parseInt(result[1], 16),
            g: parseInt(result[2], 16),
            b: parseInt(result[3], 16)
        } : { r: 0, g: 0, b: 0 };
    }

    /**
     * Read the theme_color cookie from the browser
     * (Server sets this via Set-Cookie header after form submission)
     */
    function getCookieValue(name) {
        var cookies = document.cookie.split(';');
        for (var i = 0; i < cookies.length; i++) {
            var cookie = cookies[i].trim();
            if (cookie.startsWith(name + '=')) {
                return cookie.substring(name.length + 1);
            }
        }
        return null;
    }

    /**
     * Load and display the current cookie value on page load
     */
    function updateCookieDisplay() {
        var savedColor = getCookieValue('theme_color');
        if (savedColor) {
            colorPicker.value = savedColor;
            updateColorDisplay(savedColor);
            cookieCurrent.textContent = savedColor;
        } else {
            cookieCurrent.textContent = 'Not set';
        }
    }

    updateCookieDisplay();

    // Real-time preview as user picks a color
    colorPicker.addEventListener('input', function(e) {
        updateColorDisplay(e.target.value);
    });
});
