const { ipcRenderer } = require('electron');

let selectedDevice = null;
let selectedFile = null;

// Listen for the device list from the C++ process and populate the select dropdown
ipcRenderer.on('cpp-devices', (event, devices) => {
    const deviceSelect = document.getElementById("deviceSelect");
    deviceSelect.innerHTML = '<option value="" disabled selected>Select Device</option>';  // Clear existing items

    // Populate the select dropdown with device names
    if (Array.isArray(devices) && devices.length > 0) {
        devices.forEach(device => {
            const option = document.createElement('option');
            option.value = device.name;
            option.textContent = device.name;
            deviceSelect.appendChild(option);
        });
    } else {
        const option = document.createElement('option');
        option.value = '';
        option.textContent = 'No devices available';
        deviceSelect.appendChild(option);
    }
});

// Function to handle device selection
function selectDevice() {
    const deviceSelect = document.getElementById("deviceSelect");
    selectedDevice = deviceSelect.value;

    // Display the selected device name
    const selectedDeviceName = document.getElementById("selectedDeviceName");
    selectedDeviceName.textContent = `Selected Device: ${selectedDevice}`;

    checkIfReadyToShare();
}

// Function to handle file selection and display the file name
function showFileName() {
    const fileInput = document.getElementById('fileInput');
    selectedFile = fileInput.files[0];  // Get the selected file

    if (selectedFile) {
        const selectedFileName = document.getElementById("selectedFileName");
        selectedFileName.textContent = `Selected File: ${selectedFile.name}`;
    }

    checkIfReadyToShare();
}

// Function to check if both device and file are selected, and then display the Share button
function checkIfReadyToShare() {
    const shareButton = document.getElementById("shareButton");

    if (selectedDevice && selectedFile) {
        shareButton.style.display = 'inline-block';  // Show the Share button
    } else {
        shareButton.style.display = 'none';  // Hide the Share button if both are not selected
    }
}

// Function to handle file sharing
function shareFile() {
    const shareButton = document.getElementById("shareButton");
    const progressContainer = document.getElementById("progressContainer");

    shareButton.style.display = 'none';
    progressContainer.style.display = 'block';

    // Send the selected device and file path to the C++ process
    ipcRenderer.send('start-share', { device: selectedDevice, file: selectedFile.path });
    console.log('Sent file to main process:', { device: selectedDevice, file: selectedFile.path }); 
    alert('File sharing initiated for ' + selectedFile.name + ' to ' + selectedDevice);
}



// Listen for progress updates from the C++ process
ipcRenderer.on('cpp-progress', (event, progress) => {
    const progressBar = document.getElementById('progressBar');

    // Update the progress bar width and text
    progressBar.style.width = progress + '%';
    progressBar.textContent = progress + '%';

    // Check if the process is complete (progress reaches 100%)
    if (progress >= 100) {
        alert('File sharing complete!');
        resetProgress();
    }
});

// Function to reset the progress UI after the operation completes
const downloadedFiles = [];  // To store the last 5 downloaded files

function resetProgress() {
    const progressContainer = document.getElementById("progressContainer");
    const progressBar = document.getElementById("progressBar");

    // Hide the progress container
    progressContainer.style.display = 'none';

    // Reset the progress bar
    progressBar.style.width = '0%';
    progressBar.textContent = '';

    // Store the downloaded file
    if (selectedFile) {
        addDownloadedFile(selectedFile.name);
    }
}

// Function to add a file to the list of downloaded files
function addDownloadedFile(fileName) {
    const downloadedFilesList = document.getElementById('downloadedFilesList');

    // Add the file to the array
    downloadedFiles.unshift(fileName);  // Add new file to the beginning
    if (downloadedFiles.length > 5) {
        downloadedFiles.pop();  // Remove the oldest file if the list exceeds 5 items
    }

    // Clear the current list in the UI
    downloadedFilesList.innerHTML = '';

    // Populate the updated list in the UI
    downloadedFiles.forEach(file => {
        const listItem = document.createElement('li');
        listItem.textContent = file;
        downloadedFilesList.appendChild(listItem);
    });
}
