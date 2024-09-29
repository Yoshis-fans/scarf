const { ipcRenderer } = require('electron');

// Listen for the device list from the C++ process
ipcRenderer.on('cpp-devices', (event, devices) => {
    const dropdownList = document.getElementById("dropdownList");
    dropdownList.innerHTML = '';  // Clear any existing items
    
    // Populate the dropdown with device names
    if (devices && devices.length > 0) {
        devices.forEach(device => {
            const listItem = document.createElement('li');
            listItem.textContent = device.name;  // Display device name
            listItem.onclick = () => selectDevice(device.name);  // Select device on click
            dropdownList.appendChild(listItem);
        });
    } else {
        const noDevices = document.createElement('li');
        noDevices.textContent = 'No devices available';
        dropdownList.appendChild(noDevices);
    }
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
