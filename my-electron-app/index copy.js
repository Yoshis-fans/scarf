const { app, BrowserWindow, ipcMain } = require('electron');
const { spawn } = require('child_process');
let mainWindow;

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
    mainWindow.on('closed', () => {
        mainWindow = null;
    });
}

// Spawn the C++ process
const cppProcess = spawn('./cpp_program');

// Listen for data from stdout
cppProcess.stdout.on('data', (data) => {
    try {
        // Parse the incoming data as JSON
        const message = JSON.parse(data.toString());

        // Differentiate data based on the "type" field and send to renderer process
        if (message.type === 'devices') {
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
    } catch (err) {
        console.error('Failed to parse message:', err);
    }
});

// Standard Electron app lifecycle
app.on('ready', createWindow);
app.on('window-all-closed', () => {
    if (process.platform !== 'darwin') app.quit();
});
app.on('activate', () => {
    if (mainWindow === null) createWindow();
});
