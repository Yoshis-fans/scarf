const { ipcRenderer } = require('electron');

// Listen for the device list from the C++ process
ipcRenderer.on('cpp-devices', (event, devices) => {
    // Update the UI to show the list of devices
    const devicesElement = document.getElementById('devices');
    devicesElement.innerHTML = '';
    devices.forEach(device => {
        const deviceElement = document.createElement('li');
        deviceElement.textContent = device;
        devicesElement.appendChild(deviceElement);
    });
});

// Listen for error messages
ipcRenderer.on('cpp-error', (event, errorMessage) => {
    // Update the UI to show the error message
    const errorElement = document.getElementById('error');
    errorElement.textContent = errorMessage;
    errorElement.style.display = 'block';
});

// Listen for progress updates
ipcRenderer.on('cpp-progress', (event, progress) => {
    // Update a progress bar or progress text
    const progressBar = document.getElementById('progress');
    progressBar.style.width = progress + '%';
    progressBar.textContent = progress + '%';
});

// Listen for status updates
ipcRenderer.on('cpp-status', (event, statusMessage) => {
    // Update the UI to show the current status
    const statusElement = document.getElementById('status');
    statusElement.textContent = statusMessage;
});

// Handle unknown data types
ipcRenderer.on('cpp-unknown', (event, data) => {
    console.warn('Received unknown message type from C++ process:', data);
});
