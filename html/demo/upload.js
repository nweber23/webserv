/**
 * Upload Demo - Client-side UI enhancements
 * This script provides a better user experience by:
 * - Dynamically fetching and displaying the file list from the server
 * - Showing upload status messages and feedback
 * - Providing download and delete functionality
 * - Listing files without page reloads
 *
 * NOTE: The actual file storage, deletion, and directory serving is
 * handled by the C++ server. This script only enhances the UI with
 * dynamic file listing and status messages.
 */

document.addEventListener('DOMContentLoaded', function() {
    const fileInput = document.getElementById('fileInput');
    const uploadBtn = document.getElementById('uploadBtn');
    const filesList = document.getElementById('filesList');
    const uploadMessage = document.getElementById('uploadMessage');

    // Load files on page load
    loadFiles();

    uploadBtn.addEventListener('click', uploadFile);
    fileInput.addEventListener('keypress', function(e) {
        if (e.key === 'Enter') {
            uploadFile();
        }
    });

    /**
     * Display a temporary status message to the user
     */
    function showMessage(message, type) {
        uploadMessage.textContent = message;
        uploadMessage.className = 'upload-message ' + type;
        uploadMessage.style.display = 'block';
        setTimeout(() => {
            uploadMessage.style.display = 'none';
        }, 4000);
    }

    /**
     * Fetch the directory listing from /uploads
     * Server returns HTML, we parse it to get file list
     */
    function loadFiles() {
        fetch('/uploads')
            .then(response => response.text())
            .then(html => {
                const parser = new DOMParser();
                const doc = parser.parseFromString(html, 'text/html');
                const links = doc.querySelectorAll('a');
                const files = [];

                links.forEach(link => {
                    const href = link.getAttribute('href');
                    if (href && href !== '../' && !href.startsWith('?')) {
                        const fileName = href.replace(/\/$/, '');
                        files.push(fileName);
                    }
                });

                renderFiles(files);
            })
            .catch(error => {
                filesList.innerHTML = '<div class="files-empty">Error loading files</div>';
            });
    }

    /**
     * Render the file list with download and delete buttons
     */
    function renderFiles(files) {
        if (files.length === 0) {
            filesList.innerHTML = '<div class="files-empty">No files uploaded yet</div>';
            return;
        }

        filesList.innerHTML = files.map(file => `
            <div class="file-item">
                <div class="file-info">
                    <span class="file-name">${escapeHtml(file)}</span>
                </div>
                <div class="file-actions">
                    <a href="/uploads/${encodeURIComponent(file)}" class="btn-small" download>Download</a>
                    <button class="btn-small btn-delete" onclick="deleteFile('${escapeHtml(file)}')">Delete</button>
                </div>
            </div>
        `).join('');
    }

    /**
     * Upload file to /uploads endpoint
     * Server receives POST request and stores the file
     */
    function uploadFile() {
        if (fileInput.files.length === 0) {
            showMessage('Please select a file', 'error');
            return;
        }

        const file = fileInput.files[0];
        const formData = new FormData();
        formData.append('file', file);

        uploadBtn.disabled = true;
        uploadBtn.classList.add('loading');
        showMessage('Uploading ' + file.name + '...', 'pending');

        fetch('/uploads', {
            method: 'POST',
            body: formData
        })
            .then(response => {
                if (!response.ok) {
                    throw new Error('HTTP ' + response.status);
                }
                showMessage('File uploaded successfully', 'success');
                fileInput.value = '';
                loadFiles();
            })
            .catch(error => {
                showMessage('Upload failed: ' + error.message, 'error');
            })
            .finally(() => {
                uploadBtn.disabled = false;
                uploadBtn.classList.remove('loading');
            });
    }

    /**
     * Escape HTML to prevent XSS when rendering filenames
     */
    function escapeHtml(text) {
        const div = document.createElement('div');
        div.textContent = text;
        return div.innerHTML;
    }

    /**
     * Delete a file from the server
     * Server receives DELETE request and removes the file
     */
    window.deleteFile = function(fileName) {
        if (!confirm('Delete ' + fileName + '?')) {
            return;
        }

        fetch('/uploads/' + encodeURIComponent(fileName), {
            method: 'DELETE'
        })
            .then(response => {
                if (!response.ok) {
                    throw new Error('HTTP ' + response.status);
                }
                showMessage('File deleted successfully', 'success');
                loadFiles();
            })
            .catch(error => {
                showMessage('Delete failed: ' + error.message, 'error');
            });
    };
});
