// The module 'vscode' contains the VS Code extensibility API
// Import the module and reference it with the alias vscode in your code below
const vscode = require('vscode');

// This method is called when your extension is activated
// Your extension is activated the very first time the command is executed

/**
 * @param {vscode.ExtensionContext} context
 */
function activate(context) {

	// Use the console to output diagnostic information (console.log) and errors (console.error)
	// This line of code will only be executed once when your extension is activated
	console.log('Congratulations, your extension "tabulationPlugin" is now active!');

	const vscode = require('vscode');
	const fs = require('fs');
	const { exec } = require('child_process');

	let disposable = vscode.commands.registerCommand('tabulationPlugin.refactorCode', async function () {
		console.log('command activated');
		try {
			const editor = vscode.window.activeTextEditor;
			if (!editor) {
				vscode.window.showErrorMessage('No active text editor');
				return;
			}

			const document = editor.document;
			const fullText = document.getText();

			// Запись текста в файл "in.txt"
			fs.writeFileSync('in.txt', fullText);

			
			// Запуск программы pluginMain.exe
			exec('pluginMain.exe', (error, stdout, stderr) => {
				if (error) {
					vscode.window.showErrorMessage(`Error running pluginMain.exe (error): ${error.code}`);
					console.log(`Error running pluginMain.exe (error): ${error.code}`);
					return;
				}
				if (stderr) {
					vscode.window.showErrorMessage(`Error running pluginMain.exe (stderr): ${stderr}`);
					console.log(`Error running pluginMain.exe (stderr): ${stderr}`);
					return;
				}

				// Чтение текста из файла "out.txt"
				const newText = fs.readFileSync('out.txt', 'utf8');

				// Замена кода в редакторе на считанный текст
				editor.edit(editBuilder => {
					const documentStart = new vscode.Position(0, 0);
					const documentEnd = new vscode.Position(document.lineCount - 1, document.lineAt(document.lineCount - 1).text.length);
					const fullRange = new vscode.Range(documentStart, documentEnd);
					editBuilder.replace(fullRange, newText);
				});

				// Удаление файлов "in.txt" и "out.txt"
				fs.unlinkSync('in.txt');
				fs.unlinkSync('out.txt');
			});
			
		} catch (error) {
			vscode.window.showErrorMessage(`Error: ${error.message}`);
			console.log(`Error: ${error.message}`);
		}
	});

	context.subscriptions.push(disposable);


	
	
}

// This method is called when your extension is deactivated
function deactivate() {}

module.exports = {
	activate,
	deactivate
}
