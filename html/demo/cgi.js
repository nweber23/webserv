/**
 * CGI Demo - Client-side UI enhancements
 * This script provides a better user experience by:
 * - Displaying a visual flow of the request/response process
 * - Showing status updates (pending, success, error)
 * - Displaying script output with formatting
 * - Handling file uploads and script execution
 *
 * NOTE: The actual CGI execution, file handling, and environment variable
 * setup is handled by the C++ server. This script only enhances the UI.
 */

document.addEventListener('DOMContentLoaded', function() {
    const scriptFile = document.getElementById('scriptFile');
    const scriptPath = document.getElementById('scriptPath');
    const queryInput = document.getElementById('queryInput');
    const executeBtn = document.getElementById('executeBtn');
    const outputDisplay = document.getElementById('outputDisplay');
    const outputSection = document.getElementById('outputSection');
    const flowContainer = document.getElementById('flowContainer');
    const requestInfo = document.getElementById('requestInfo');
    const statusBadge = document.getElementById('statusBadge');

    executeBtn.addEventListener('click', executeCGI);

    /**
     * Main execution handler - routes to upload or script execution
     */
    function executeCGI() {
        let scriptToExecute = null;

        if (scriptFile.files.length > 0) {
            uploadAndExecute(scriptFile.files[0]);
            return;
        }

        scriptToExecute = scriptPath.value.trim();
        if (!scriptToExecute) {
            alert('Please select a file to upload or enter an existing script path');
            return;
        }

        executeScript(scriptToExecute);
    }

    /**
     * Upload file to /cgi-bin/ and then execute it
     * Server receives the POST, stores the file, and executes it
     */
    function uploadAndExecute(file) {
        const formData = new FormData();
        formData.append('file', file);

        outputSection.style.display = 'block';
        flowContainer.style.display = 'block';
        requestInfo.style.display = 'block';
        outputDisplay.classList.add('empty');
        outputDisplay.textContent = 'Uploading file...';
        statusBadge.textContent = 'pending';
        statusBadge.className = 'status-badge pending';
        executeBtn.disabled = true;
        executeBtn.classList.add('loading');

        document.getElementById('infoMethod').textContent = 'POST (Upload)';
        document.getElementById('infoEndpoint').textContent = '/cgi-bin/';
        document.getElementById('flowRequest').textContent = 'Uploading: ' + file.name;

        fetch('/cgi-bin/', {
            method: 'POST',
            body: formData
        })
            .then(response => {
                if (!response.ok) {
                    throw new Error('Upload failed: HTTP ' + response.status);
                }
                return response.text();
            })
            .then(result => {
                document.getElementById('flowExecution').textContent = 'Upload complete, executing...';
                const uploadedPath = '/cgi-bin/' + file.name;
                executeScript(uploadedPath);
            })
            .catch(error => {
                outputDisplay.classList.remove('empty');
                outputDisplay.classList.add('output-error');
                outputDisplay.textContent = 'Upload Error: ' + error.message;
                document.getElementById('flowResponse').textContent = 'Error: ' + error.message;
                statusBadge.textContent = 'error';
                statusBadge.className = 'status-badge error';
                executeBtn.disabled = false;
                executeBtn.classList.remove('loading');
            });
    }

    /**
     * Execute a script on the server and fetch its output
     * The server runs the CGI script and returns the response
     */
    function executeScript(script) {
        const query = queryInput.value;
        const fullPath = query ? script + '?' + query : script;

        outputSection.style.display = 'block';
        flowContainer.style.display = 'block';
        requestInfo.style.display = 'block';

        document.getElementById('infoMethod').textContent = 'GET';
        document.getElementById('infoEndpoint').textContent = script;
        document.getElementById('infoQuery').textContent = query || 'none';

        document.getElementById('flowRequest').textContent = fullPath;
        document.getElementById('flowExecution').textContent = 'Running ' + script.split('/').pop();

        outputDisplay.classList.add('empty');
        outputDisplay.textContent = 'Executing script...';
        statusBadge.textContent = 'pending';
        statusBadge.className = 'status-badge pending';
        executeBtn.disabled = true;
        executeBtn.classList.add('loading');

        fetch(fullPath)
            .then(response => {
                const size = response.headers.get('content-length') || 'unknown';
                document.getElementById('infoSize').textContent = size + ' bytes';

                if (!response.ok) {
                    throw new Error('HTTP ' + response.status);
                }
                return response.text();
            })
            .then(data => {
                outputDisplay.classList.remove('empty');
                outputDisplay.textContent = data;
                document.getElementById('flowResponse').textContent = 'Response received: ' + (data.length) + ' bytes';

                statusBadge.textContent = 'success';
                statusBadge.className = 'status-badge success';
            })
            .catch(error => {
                outputDisplay.classList.remove('empty');
                outputDisplay.classList.add('output-error');
                outputDisplay.textContent = 'Error: ' + error.message;
                document.getElementById('flowResponse').textContent = 'Error: ' + error.message;

                statusBadge.textContent = 'error';
                statusBadge.className = 'status-badge error';
            })
            .finally(() => {
                executeBtn.disabled = false;
                executeBtn.classList.remove('loading');
            });
    }

    // Allow Enter key to execute
    queryInput.addEventListener('keypress', function(e) {
        if (e.key === 'Enter') {
            executeCGI();
        }
    });
});
