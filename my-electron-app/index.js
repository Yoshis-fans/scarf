const { app, BrowserWindow, ipcMain } = require('electron');
const { spawn } = require('child_process');
let mainWindow;
let cppProcess;  // C++ process declaration

function createWindow() {
    mainWindow = new BrowserWindow({
        width: 800,
        height: 600,
        webPreferences: {
            nodeIntegration: true,
            contextIsolation: false,
            enableRemoteModule: true
        }
    });
    mainWindow.loadFile('index.html');

    // Spawn the C++ process
    cppProcess = spawn('./cpp_program');  // Adjust path to your C++ executable

    // Listen for data from stdout
    cppProcess.stdout.on('data', (data) => {
        console.log('Raw data from C++:', data.toString());
        try {
            const message = JSON.parse(data.toString());

            if (mainWindow && mainWindow.webContents) {
                // Handle different message types
                if (message.type === 'file_request') {
                    mainWindow.webContents.send('cpp-file-request', message.data.file_name);
                } else if (message.type === 'devices') {
                    mainWindow.webContents.send('cpp-devices', message.data);
                } else if (message.type === 'errors') {
                    mainWindow.webContents.send('cpp-error', message.data);
                } else if (message.type === 'progress') {
                    mainWindow.webContents.send('cpp-progress', message.data);
                } else if (message.type === 'status') {
                    mainWindow.webContents.send('cpp-status', message.data);
                } else {
                    mainWindow.webContents.send('cpp-unknown', message.data);
                }
            }
          } catch (err) {
            console.error('Failed to parse message:', err);
            console.error('Raw output:', output); // Log raw output for debugging
        }
    });

    mainWindow.on('closed', () => {
        mainWindow = null;
    });
}

app.on('ready', createWindow);
app.on('window-all-closed', () => {
    if (process.platform !== 'darwin') app.quit();
});
app.on('activate', () => {
    if (mainWindow === null) createWindow();
});

// Handle the file sharing request from the renderer process
ipcMain.on('start-share', (event, data) => {
    if (!cppProcess) {
        console.error('C++ process is not running.');
        return;
    }

    console.log('Sharing file:', data.file, 'to device:', data.device);

    // Send the file path to the C++ process
    cppProcess.stdin.write(data.file + '\n'); 

    // Simulate receiving progress updates
    let progress = 0;
    const progressInterval = setInterval(() => {
        if (progress >= 100) {
            clearInterval(progressInterval);
        } else {
            progress += 10;
            event.sender.send('cpp-progress', progress); 
        }
    }, 500);
});
